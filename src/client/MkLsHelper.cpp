#include "MkLsHelper.hpp"
#include "utilsClient.hpp"

MkLsHelper::MkLsHelper(const QString& host, const QString& port)
  : msocket(new QTcpSocket()), mhost(host), mport(port)
{
  setRequestPatterns();
}

MkLsHelper::~MkLsHelper()
{
  msocket->disconnect();
  msocket->waitForDisconnected();
  delete msocket;
}


bool MkLsHelper::connect()
{
  qDebug() << mhost << mport;
  msocket->connectToHost(mhost, mport.toInt(), QAbstractSocket::ReadWrite);
  if (!msocket->waitForConnected(DefaultTimeout)) {
      displayError(msocket->error());
      return false;
    }
  return true;
}


bool MkLsHelper::disconnect()
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
      qDebug() << "Unable to write to socket";
      return false;
    }
  return true;
}

bool MkLsHelper::recvData(const ReqTypeT& reqType)
{
  mldchecks.clear();
  if (!msocket->waitForReadyRead()) {
      qDebug() << "Nothing to read";
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
          if (fields.size() != 5)
            return false;
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
          qDebug() << "Bad request type " << reqType;
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

void MkLsHelper::displayError(QAbstractSocket::SocketError socketError)
{
  switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
      break;
    case QAbstractSocket::HostNotFoundError:
      utils::alert(tr("The host was not found. Please check the "
                      "host name and port settings."));
      break;
    case QAbstractSocket::ConnectionRefusedError:
      utils::alert(tr("The connection was refused by the peer. "
                      "Make sure the fortune server is running, "
                      "and check that the host name and port "
                      "settings are correct."));
      break;
    default:
      utils::alert(tr("The following error occurred: %1.").arg(msocket->errorString()));
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
