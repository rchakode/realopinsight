
#include "Socket.hpp"
#include <zmq.h>
#include <string.h>
#include <iostream>
#include <memory>

// Emulate zeromq 3.x over zeromq 2.x
#ifndef ZMQ_DONTWAIT
#   define ZMQ_DONTWAIT     ZMQ_NOBLOCK
#endif
#if ZMQ_VERSION_MAJOR == 2
#   define ZMQ_POLL_MSEC    1000        //  zmq_poll is usec
#elif ZMQ_VERSION_MAJOR == 3
#   define ZMQ_POLL_MSEC    1           //  zmq_poll is msec
#endif

const int MAX_MSG_SIZE=8192;
const int TIMEOUT = 2500; //15000
const int NUM_RETRIES = 3;

Socket::Socket(const int & _type)
  : mtype(_type),
    mconnected2Server(false),
    mserverSerial(-1) { }

Socket::~Socket()
{
  disconnect();
}

bool Socket::disconnect()
{
  if (zmq_close(msocket) != 0) return false;
  return (zmq_term(mcontext) == 0);
}

bool Socket::init()
{
  mcontext = zmq_init(1);
  if (!mcontext) return false;
  return (msocket = zmq_socket(mcontext, mtype)) != NULL;
}

bool Socket::connect(const std::string & _uri)
{
  mserverUri = _uri;
  if (!init()) return false;
  return zmq_connect(msocket, mserverUri.c_str()) == 0;
}

bool Socket::bind(const std::string & _uri)
{
  mserverUri = _uri;
  if (!init()) return false;
  return zmq_bind(msocket, mserverUri.c_str()) == 0;
}

void Socket::send(const std::string & _msg) {
  int sent = -1;
#if ZMQ_VERSION_MAJOR == 2
  zmq_msg_t msg;
  sent = zmq_msg_init_size (&msg, _msg.size());
  memcpy (zmq_msg_data (&msg), _msg.c_str(), _msg.size());
  zmq_send(msocket, &msg, 0);
  zmq_msg_close(&msg);
#elif ZMQ_VERSION_MAJOR == 3
  sent = zmq_send(msocket, _msg.c_str(), _msg.size(), 0);
#endif
  if(sent <= 0) {
      //TODO: deal with error
    }
}

std::string Socket::recv() const{
  int ret = -1;
  zmq_msg_t msg;
  ret = zmq_msg_init_size(&msg, MAX_MSG_SIZE);
  if (ret != 0) return "";
  memset(zmq_msg_data(&msg), 0, MAX_MSG_SIZE);
#if ZMQ_VERSION_MAJOR == 2
  ret = zmq_recv(msocket, &msg, 0);
#elif ZMQ_VERSION_MAJOR == 3
  received = zmq_recv(msocket, zmq_msg_data(&msg), MAX_MSG_SIZE, 0);
#endif
  if (ret < 0) return "";
  char *retBuffer = (char*)zmq_msg_data(&msg);
  return std::string(retBuffer, zmq_msg_size(&msg));
}

void Socket::makeHandShake() {
  int retriesLeft = NUM_RETRIES;
  auto socket = std::unique_ptr<Socket>(new Socket(ZMQ_REQ));
  std::string msg("PING");
  while (retriesLeft) {
      if(!socket->connect(mserverUri)) break;
      socket->send(msg);
      bool expectReply = true;
      while (expectReply) {
          zmq_pollitem_t items[] = { {socket->getSocket(), 0, ZMQ_POLLIN, 0 } };
          zmq_poll(&items[0], 1, TIMEOUT * ZMQ_POLL_MSEC);  //TODO in zeromq 3.x the unit is different

          if (items[0].revents & ZMQ_POLLIN) {
              std::string reply = socket->recv();
              socket->disconnect();
              socket.reset(nullptr);
              size_t pos = reply.find(":");
              std::string respType = reply.substr(0, pos);
              if(respType == "ALIVE") {
                  mconnected2Server = true;
                  if(pos == std::string::npos){
                      mserverSerial = 100;
                    } else {
                      mserverSerial = convert2ServerSerial(reply.substr(pos+1, std::string::npos));
                    }
                  std::cerr << "INFO: Connection etablished; server serial: " << mserverSerial <<"\n";
                  return;
                } else {
                  std::cerr << "ERROR: Weird response from the server\n";
                }
            } else {
              if (--retriesLeft == 0) {
                  std::cerr << "ERROR: Server seems to be offline, abandoning\n";
                  expectReply = false;
                } else {
                  std::cerr << "WARNING: No response from server, retrying…\n";
                  socket.reset(nullptr);
                  socket.reset(new Socket(ZMQ_REQ));
                  socket->send(msg);
                }
            }
        }
    }
}

int Socket::convert2ServerSerial(const std::string & versionStr){
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
