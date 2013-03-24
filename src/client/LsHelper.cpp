#include "LsHelper.hpp"
#include "utilsClient.hpp"
#include <QDir>
#include <iostream>
#include "client/utilsClient.hpp"

LsHelper::LsHelper(const QString& host, const int& port)
  : mhost(host), mport(port)
{
  setSocketOption(QAbstractSocket::KeepAliveOption, 1);
  setRequestPatterns();
}

LsHelper::~LsHelper()
{
  QAbstractSocket::disconnect();
}

void LsHelper::setRequestPatterns()
{
  mrequestMap[Host] = "GET hosts\n"
      "Columns: name state last_state_change check_command plugin_output\n"
      "Filter: name = %1\n\n";
  mrequestMap[Service] = "GET services\n"
      "Columns: host_name service_description state last_state_change check_command plugin_output\n"
      "Filter: host_name = %1\n\n";
}

bool LsHelper::connectToService()
{
  disconnectFromHost();
  qDebug() << tr("Connecting to %1:%2...").arg(mhost).arg(mport);
  QAbstractSocket::connectToHost(mhost, mport, QAbstractSocket::ReadWrite);
  if (!QAbstractSocket::waitForConnected(DefaultTimeout)) {
      handleNetworkFailure();
      return false;
    }
  return true;
}


void LsHelper::disconnectFromService()
{
  QAbstractSocket::disconnectFromHost();
}

bool LsHelper::requestData(const QString& host, const ReqTypeT& reqType)
{
  qint32 nb;
  if (!isConnected()) {
      connectToService();
    }
  if (!isConnected() ||
      (nb = QAbstractSocket::write(mrequestMap[reqType].arg(host).toAscii())) <= 0 ||
      !QAbstractSocket::waitForBytesWritten(DefaultTimeout)) {
      handleNetworkFailure();
      return false;
    }
  qDebug() << tr("%1 bytes written").arg(nb);
  return true;
}

bool LsHelper::recvData(const ReqTypeT& reqType)
{
  if (!QAbstractSocket::waitForReadyRead(DefaultTimeout)) {
      handleNetworkFailure();
      return false;
    }
  QString chkid = "";
  MonitorBroker::CheckT check;
  QString entry;
  QTextStream buffer(this);
  while (!((entry = buffer.readLine()).isNull())) {
      if (entry.isEmpty()) continue;
      QStringList fields = entry.split(";");
      chkid.clear();
      if (reqType == Host) {
          if (fields.size() != 5) {
              return false;
            }
          chkid = fields[0].toLower();
          check.id = check.host = fields[0].toStdString();
          check.status = fields[1].toInt();
          check.last_state_change = fields[2].toStdString();
          check.check_command = fields[3].toStdString();
          check.alarm_msg = fields[4].toStdString();
        } else if (reqType == Service) {
          if (fields.size() != 6)
            return false;
          QString hostname = fields[0];
          chkid = ID_PATTERN.arg(hostname, fields[1]).toLower();
          check.host = hostname.toStdString();
          check.id = chkid.toStdString();
          check.status = fields[2].toInt();
          check.last_state_change = utils::getCtime(fields[3].toUInt());
          check.check_command = fields[4].toStdString();
          check.alarm_msg = fields[5].toStdString();
        } else {
          QAbstractSocket::setErrorString(tr("Bad request type: %1").arg(reqType));
          return false;
        }
      mldchecks.insert(chkid, check);
    }
  return true;
}

bool LsHelper::loadHostData(const QString& host)
{
  mldchecks.clear();
  bool succeed;
  succeed = connectToService() &&
      requestData(host, Host) &&
      recvData(Host);
  disconnectFromService();

  succeed = succeed && connectToService() &&
      requestData(host, Service)&&
      recvData(Service);
  disconnectFromService();
  return true;
}

bool LsHelper::findCheck(const QString& id, CheckListCstIterT& check)
{
  check = mldchecks.find(id.toLower());
  if (check != mldchecks.end()) {
      return true;
    }
  return false;
}

void LsHelper::handleNetworkFailure(QAbstractSocket::SocketError error)
{
  switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
      QAbstractSocket::setErrorString(tr("The connection has been closed by the remote host"));
      break;
    case QAbstractSocket::HostNotFoundError:
      QAbstractSocket::setErrorString(tr("The host not found (%1).").arg(mhost));
      break;
    case QAbstractSocket::ConnectionRefusedError:
      QAbstractSocket::setErrorString(tr("Connection refused. "
                                         "Make sure that Livestatus API is listening on tcp://%1:%2").arg(mhost).arg(mport));
      break;
    default:
      QAbstractSocket::setErrorString(tr("The following error occurred: %1")
                                      .arg(QAbstractSocket::errorString()));
    }
}

