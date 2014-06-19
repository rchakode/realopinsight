/*
 * ZmqSocket.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#	                                                                         #
# This file is part of RealOpInsight (http://RealOpInsight.com) authored   #
# by Rodrigue Chakode <rodrigue.chakode@gmail.com>                         #
#                                                                          #
# RealOpInsight is free software: you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with RealOpInsight.  If not, see <http://www.gnu.org/licenses/>.   #
#--------------------------------------------------------------------------#
 */

#include "ZmqSocket.hpp"
#include <zmq.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <ctime>

#ifndef ZMQ_DONTWAIT
#   define ZMQ_DONTWAIT     ZMQ_NOBLOCK
#endif
#if ZMQ_VERSION_MAJOR == 2
#   define ZMQ_POLL_MSEC    1000
#elif ZMQ_VERSION_MAJOR == 3
#   define ZMQ_POLL_MSEC    1
#endif
const int ZERO_LINGER = 0;
const int TIMEOUT_MSEC = 2500;
const int MAX_MSG_SIZE = 8192;
const int NUM_RETRIES = 3;
const int TIMEOUT =  TIMEOUT_MSEC * ZMQ_POLL_MSEC;

ZmqSocket::ZmqSocket(const int & _type)
  : m_type(_type),
    m_connected2Server(false),
    m_serverSerial(-1) { }

ZmqSocket::ZmqSocket(const std::string& uri, const int & _type)
  : m_serverUri(uri),
    m_type(_type),
    m_connected2Server(false),
    m_serverSerial(-1) { }

ZmqSocket::~ZmqSocket()
{
  disconnecteFromService();
}

bool ZmqSocket::init()
{
  m_context = zmq_init(1);
  if (!m_context) return false;
  return (m_socket = zmq_socket(m_context, m_type)) != NULL;
}

void ZmqSocket::disconnecteFromService()
{
  zmq_setsockopt(m_socket, ZMQ_LINGER, &ZERO_LINGER, sizeof(ZERO_LINGER));
  zmq_close(m_socket);
  zmq_term(m_context);
}

void ZmqSocket::reset()
{
  disconnecteFromService();
  connect();
}

bool ZmqSocket::connect()
{
  if (!init()) return false;
  return zmq_connect(m_socket, m_serverUri.c_str()) == 0;
}

bool ZmqSocket::connect(const std::string & _uri)
{
  m_serverUri = _uri;
  return connect();
}

bool ZmqSocket::bind(const std::string & _uri)
{
  m_serverUri = _uri;
  if (!init()) return false;
  return zmq_bind(m_socket, m_serverUri.c_str()) == 0;
}

void ZmqSocket::send(const std::string & _msg) {
  int sent = -1;
#if ZMQ_VERSION_MAJOR == 2
  zmq_msg_t msg;
  sent = zmq_msg_init_size (&msg, _msg.size());
  memcpy (zmq_msg_data (&msg), _msg.c_str(), _msg.size());
  zmq_send(m_socket, &msg, 0);
  zmq_msg_close(&msg);
#elif ZMQ_VERSION_MAJOR == 3
  sent = zmq_send(m_socket, _msg.c_str(), _msg.size(), 0);
#endif
  if(sent <= 0) { /* TODO: deal with error */}
}

std::string ZmqSocket::recv() const{
  int ret = -1;
  zmq_msg_t msg;
#if ZMQ_VERSION_MAJOR == 2
  ret = zmq_msg_init(&msg);
  if (ret != 0) return "";
  ret = zmq_recv(m_socket, &msg, 0);
  if (ret == 0) ret = zmq_msg_size(&msg);
#elif ZMQ_VERSION_MAJOR == 3
  ret = zmq_msg_init_size(&msg, MAX_MSG_SIZE);
  memset(zmq_msg_data(&msg), '\0', MAX_MSG_SIZE);
  ret = zmq_recv(m_socket, zmq_msg_data(&msg), MAX_MSG_SIZE, 0);
#endif
  if (ret <= 0) return "";
  char *retBuffer = (char*)zmq_msg_data(&msg);
  return std::string(retBuffer, ret);
}

void ZmqSocket::makeHandShake() {
  int retriesLeft = NUM_RETRIES;
  std::string reply("");
  auto socket = std::unique_ptr<ZmqSocket>(new ZmqSocket(m_serverUri, ZMQ_REQ));
  std::string msg("PING");
  m_connected2Server = false;

  while (retriesLeft) {
    if(!socket->connect()) break;
    socket->send(msg);
    time_t curTime = time(NULL); std::string timeStr = std::string(ctime(&curTime));
    zmq_pollitem_t items[] = { {socket->getSocket(), 0, ZMQ_POLLIN, 0 } };
    zmq_poll(&items[0], 1, TIMEOUT);
    if (items[0].revents & ZMQ_POLLIN) {
      reply.clear();
      reply = socket->recv();
      socket->disconnecteFromService();
      size_t pos = reply.find(":");
      std::string respType = reply.substr(0, pos);
      if(respType == "ALIVE") {
        m_connected2Server = true;
        if(pos == std::string::npos){
          m_serverSerial = 100;
        } else {
          m_serverSerial = convert2ServerSerial(reply.substr(pos+1, std::string::npos));
        }
        std::ostringstream oss;
        oss << "Connection etablished; server serial: " << m_serverSerial;
        m_errorMsg = oss.str();
        return;
      } else {
        //FIXME: sometimes this could be due to authentication failed
        m_errorMsg = "Weird response from the server ("+reply+")";
        std::cerr << m_errorMsg << "\n";
        break;
      }
    } else {
      m_errorMsg = "No response from server, retrying...";
      std::cerr << m_errorMsg << "\n";
      socket->reset();
    }
    if (--retriesLeft == 0) {
      m_errorMsg = "Unable to connect to the service from this address ("+m_serverUri+")";
      std::cerr << m_errorMsg << "\n";
    }
  }
}

int ZmqSocket::convert2ServerSerial(const std::string & versionStr){
  std::string str = "";
  for(size_t i = 0; i < versionStr.size(); i++) {
    if(versionStr[i] >= '0' &&
       versionStr[i]<= '9') {
      str += versionStr[i];
    } else if(versionStr[i] != '.') {
      break;
    }
  }
  return atoi(str.c_str());
}
