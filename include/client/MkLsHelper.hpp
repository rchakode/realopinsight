#ifndef MKLSHELPER_HPP
#define MKLSHELPER_HPP

#include "Base.hpp"
#include <QTcpSocket>

class MkLsHelper : public QTcpSocket
{
  Q_OBJECT
public:
  enum ReqTypeT{
    Host = 0,
    Service = 1
  };

  explicit MkLsHelper(const QString& host, const QString& port);
  ~MkLsHelper();

  bool connect(void);
  bool disconnectSocket(void);
  bool requestData(const QString& host, const ReqTypeT& reqType);
  bool recvData(const ReqTypeT& reqType);
  void loadHostData(const QString& host);
  bool findCheck(const QString& id, CheckListCstIterT& check);
signals:
  
public slots:

private:
  const static int DefaultTimeout = 50000; /* 5 seconds */
  QTcpSocket* msocket;
  QString mhost;
  QString mport;
  RequestListT mrequestMap;
  CheckListT mldchecks;

  void displayError(QAbstractSocket::SocketError socketError);
  void setRequestPatterns();
};

#endif // MKLSHELPER_HPP
