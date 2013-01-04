#ifndef ZMQSOCKET_HPP
#define ZMQSOCKET_HPP

#include <string>

class Socket
{
public:
  Socket(const int & _type);
  void init();
  void connect(const std::string & _uri);
  void bind(const std::string & _uri);
  void disconnect();
  void send(const std::string & _msg);
  std::string recv() const;
  void makeHandShake();
  inline bool isConnected2Server() const {return mconnected2Server;}
  inline void* getSocket() const {return msocket;}
  inline int getServerSerial() const {return mserverSerial;}


private:
  std::string mserverUri;
  void *msocket;
  void *mcontext;
  int mtype;
  bool mconnected2Server;
  int mserverSerial;
  int convert2ServerSerial(const std::string &versionStr);
};

#endif // ZMQSOCKET_HPP
