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
  bool isConnected() const {return m_connected2Server; }
  void* getSocket() const {return m_socket;}
  int getServerSerial() const {return m_serverSerial;}
  std::string getErrorMsg() const {return m_errorMsg;}


private:
  std::string m_serverUri;
  std::string m_errorMsg;
  void *m_socket;
  void *m_context;
  int m_type;
  bool m_connected2Server;
  int m_serverSerial;
  int convert2ServerSerial(const std::string &versionStr);
};

#endif // ZMQSOCKET_HPP
