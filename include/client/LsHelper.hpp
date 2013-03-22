#ifndef MKLSHELPER_HPP
#define MKLSHELPER_HPP

#include "Base.hpp"
#include <QTcpSocket>

class LsHelper : public QTcpSocket
{
  Q_OBJECT
public:
  enum ReqTypeT{
    Host = 0,
    Service = 1
  };
  LsHelper(const QString& host, const int& port);
  ~LsHelper();

  bool connectToService(void);
  void disconnectFromService(void);
  bool requestData(const QString& host, const ReqTypeT& reqType);
  bool recvData(const ReqTypeT& reqType);
  bool loadHostData(const QString& host);
  bool findCheck(const QString& id, CheckListCstIterT& check);
  void clearData(void) {mldchecks.clear();}
  void setHost(const QString& host) {mhost = host;}
  void setPort(const int& port) {mport = port;}
  bool isConnected() const {return state() == QAbstractSocket::ConnectedState;}

private:
  const static int DefaultTimeout = 50000; /* 5 seconds */
  QString mhost;
  qint32 mport;
  RequestListT mrequestMap;
  CheckListT mldchecks;
  QString merrorMsg;
  void setRequestPatterns();
  void handleNetworkFailure() {handleNetworkFailure(QAbstractSocket::error());}
  void handleNetworkFailure(QAbstractSocket::SocketError error);
};

#endif // MKLSHELPER_HPP
