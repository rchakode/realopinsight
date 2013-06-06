#ifndef ZMQSOCKET_HPP
#define ZMQSOCKET_HPP

#include <string>
#include <sstream>
#include <QDebug>

class ZmqSocket
{
public:
  ZmqSocket(const int & _type);
  ZmqSocket(const std::string& uri, const int & _type);
  ~ZmqSocket();
  bool init();
  bool connect();
  bool connect(const std::string & _uri);
  bool bind(const std::string & _uri);
  void disconnecteFromService();
  void reset();
  void send(const std::string & _msg);
  std::string recv() const;
  void makeHandShake();
  bool isConnected() const {return mconnected2Server; }
  void* getSocket() const {return msocket;}
  int getServerSerial() const {return mserverSerial;}
  std::string getErrorMsg() const {return merrorMsg;}


private:
  std::string mserverUri;
  std::string merrorMsg;
  void *msocket;
  void *mcontext;
  int mtype;
  bool mconnected2Server;
  int mserverSerial;
  int convert2ServerSerial(const std::string &versionStr);
};

#endif // ZMQSOCKET_HPP
