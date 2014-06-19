
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
constexpr int TIMEOUT =  TIMEOUT_MSEC * ZMQ_POLL_MSEC;

ZmqSocket::ZmqSocket(const int & _type)
  : mtype(_type),
    mconnected2Server(false),
    mserverSerial(-1) { }

ZmqSocket::ZmqSocket(const std::string& uri, const int & _type)
  : mserverUri(uri),
    mtype(_type),
    mconnected2Server(false),
    mserverSerial(-1) { }

ZmqSocket::~ZmqSocket()
{
  finalize();
}

bool ZmqSocket::init()
{
  mcontext = zmq_init(1);
  if (!mcontext) return false;
  return (msocket = zmq_socket(mcontext, mtype)) != NULL;
}

void ZmqSocket::finalize()
{
  zmq_setsockopt(msocket, ZMQ_LINGER, &ZERO_LINGER, sizeof(ZERO_LINGER));
  zmq_close(msocket);
  zmq_term(mcontext);
}

void ZmqSocket::reset()
{
  finalize();
  init();
}

bool ZmqSocket::connect()
{
  if (!init()) return false;
  return zmq_connect(msocket, mserverUri.c_str()) == 0;
}

bool ZmqSocket::connect(const std::string & _uri)
{
  mserverUri = _uri;
  return connect();
}

bool ZmqSocket::bind(const std::string & _uri)
{
  mserverUri = _uri;
  if (!init()) return false;
  return zmq_bind(msocket, mserverUri.c_str()) == 0;
}

void ZmqSocket::send(const std::string & _msg) {
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
  if(sent <= 0) { /* TODO: deal with error */}
}

std::string ZmqSocket::recv() const{
  int ret = -1;
  zmq_msg_t msg;
#if ZMQ_VERSION_MAJOR == 2
  ret = zmq_msg_init(&msg);
  if (ret != 0) return "";
  ret = zmq_recv(msocket, &msg, 0);
  if (ret == 0) ret = zmq_msg_size(&msg);
#elif ZMQ_VERSION_MAJOR == 3
  ret = zmq_msg_init_size(&msg, MAX_MSG_SIZE);
  memset(zmq_msg_data(&msg), '\0', MAX_MSG_SIZE);
  ret = zmq_recv(msocket, zmq_msg_data(&msg), MAX_MSG_SIZE, 0);
#endif
  if (ret <= 0) return "";
  char *retBuffer = (char*)zmq_msg_data(&msg);
  return std::string(retBuffer, ret);
}

void ZmqSocket::makeHandShake() {
  int retriesLeft = NUM_RETRIES;
  std::string reply("");
  auto socket = std::unique_ptr<ZmqSocket>(new ZmqSocket(mserverUri, ZMQ_REQ));
  std::string msg("PING");
  mconnected2Server = false;

  while (retriesLeft) {
      if(!socket->connect()) break;
      socket->send(msg);
      time_t curTime = time(NULL); std::string timeStr = std::string(ctime(&curTime));
      zmq_pollitem_t items[] = { {socket->getSocket(), 0, ZMQ_POLLIN, 0 } };
      zmq_poll(&items[0], 1, TIMEOUT);
      if (items[0].revents & ZMQ_POLLIN) {
          reply.clear();
          reply = socket->recv();
          socket->finalize();
          size_t pos = reply.find(":");
          std::string respType = reply.substr(0, pos);
          if(respType == "ALIVE") {
              mconnected2Server = true;
              if(pos == std::string::npos){
                  mserverSerial = 100;
                } else {
                  mserverSerial = convert2ServerSerial(reply.substr(pos+1, std::string::npos));
                }
              std::ostringstream oss;
              oss << "Connection etablished; server serial: " << mserverSerial;
              merrorMsg = oss.str();
              std::cerr << timeStr << "INFO: " << merrorMsg <<"\n";
              return;
            } else {
              //FIXME: sometimes this could be due to authentication failed
              merrorMsg = "Weird response from the server ("+reply+")";
              std::cerr << timeStr << "ERROR: " << merrorMsg <<"\n";
              break;
            }
        } else {
          merrorMsg = "No response from server, retrying...";
          std::cerr << timeStr << "WARNING: " << merrorMsg <<"\n";
          socket->reset();
        }
      if (--retriesLeft == 0) {
          merrorMsg = "Unable to connect to the service from this address ("+mserverUri+")";
          std::cerr << timeStr << "ERROR: " << merrorMsg <<"\n";
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
