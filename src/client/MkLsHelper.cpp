#include "MkLsHelper.hpp"
#include "utilsClient.hpp"

#include "QsLog.h"
#include "QsLogDest.h"
#include <QDir>
#include <iostream>

MkLsHelper::MkLsHelper(const QString& host, const int& port)
  : msocket(new QTcpSocket()), mhost(host), mport(port)
{
  setRequestPatterns();
  SETUP_LOGGING(); //FIXME: logging
}

MkLsHelper::~MkLsHelper()
{
  msocket->disconnect();
  msocket->waitForDisconnected();
  delete msocket;
}


bool MkLsHelper::connect()
{
  msocket->connectToHost(mhost, mport, QAbstractSocket::ReadWrite);
  QObject::connect(msocket, SIGNAL(error(QAbstractSocket::SocketError)),
                   this, SLOT(displayError(QAbstractSocket::SocketError)));

  if (!msocket->waitForConnected(DefaultTimeout)) {
      displayError();
      return false;
    }
  return true;
}


bool MkLsHelper::disconnectSocket()
{
  msocket->disconnectFromHost();
  if (!msocket->waitForDisconnected(DefaultTimeout)) {
      return false;
    }
  return true;
}

bool MkLsHelper::requestData(const QString& host, const ReqTypeT& reqType)
{
  qint32 nb = msocket->write(mrequestMap[reqType].arg(host).toAscii());
  if (nb <= 0) {
      displayError();
      return false;
    }
  return true;
}

bool MkLsHelper::recvData(const ReqTypeT& reqType)
{
  mldchecks.clear();
  if (!msocket->waitForReadyRead()) {
      displayError();
      return false;
    }
  QString chkid = "";
  MonitorBroker::CheckT check;
  QString entry;
  QTextStream buffer(msocket);
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
          check.last_state_change = fields[3].toStdString();
          check.check_command = fields[4].toStdString();
          check.alarm_msg = fields[5].toStdString();
        } else {
          QLOG_ERROR() << "Bad request type " << reqType;
          return false;
        }
      mldchecks.insert(chkid, check);
    }
  return true;
}

bool MkLsHelper::findCheck(const QString& id, CheckListCstIterT& check)
{
  check = mldchecks.find(id.toLower());
  if (check != mldchecks.end()) {
      return true;
    }
  return false;
}

void MkLsHelper::displayError()
{
  displayError(msocket->error());
}

void MkLsHelper::displayError(QAbstractSocket::SocketError error)
{
  QString msg;
  switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
      msg = tr("The connection has been closed by the remote host.");
      QLOG_ERROR()<< msg;
      utils::alert(msg);
      break;
    case QAbstractSocket::HostNotFoundError:
      msg = tr("The host was not found. Please check the "
               "host name and port settings.");
      QLOG_ERROR()<< msg;
      utils::alert(msg);
      break;
    case QAbstractSocket::ConnectionRefusedError:
      msg = tr("The connection was refused by the peer. "
               "Make sure the fortune server is running, "
               "and check that the host name and port "
               "settings are correct.");
      QLOG_ERROR()<< msg;
      utils::alert(msg);
      break;
    default:
      msg = tr("The following error occurred: %1.").arg(msocket->errorString());
      QLOG_ERROR()<< msg;
      utils::alert(msg);
    }
}

void MkLsHelper::setRequestPatterns()
{
  mrequestMap[Host] = "GET hosts\n"
      "Columns: name state last_state_change check_command plugin_output\n"
      "Filter: name = %1\n\n";
  mrequestMap[Service] = "GET services\n"
      "Columns: host_name service_description state last_state_change check_command plugin_output\n"
      "Filter: host_name = %1\n\n";
}
