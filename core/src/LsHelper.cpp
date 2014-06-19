/*
 * LsHelper.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-03-2014                                                  #
#                                                                          #
# This file is part of RealOpInsight (http://RealOpInsight.com) authored   #
# by Rodrigue Chakode <rodrigue.chakode@gmail.com>                         #
#                                                                          #
# RealOpInsight is free software: you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with RealOpInsight.  If not, see <http://www.gnu.org/licenses/>.   #
#--------------------------------------------------------------------------#
 */

#include "LsHelper.hpp"
#include "utilsCore.hpp"
#include <QDir>
#include <iostream>
#include "utilsCore.hpp"

LsHelper::LsHelper(const QString& host, const int& port)
  : m_host(host), m_port(port)
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
  QAbstractSocket::connectToHost(m_host, m_port, QAbstractSocket::ReadWrite);
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
      (nb = QAbstractSocket::write(ngrt4n::toByteArray(mrequestMap[reqType].arg(host)))) <= 0 ||
      !QAbstractSocket::waitForBytesWritten(DefaultTimeout)) {
    handleNetworkFailure();
    return false;
  }
  return true;
}

bool LsHelper::recvData(const ReqTypeT& reqType)
{
  if (!QAbstractSocket::waitForReadyRead(DefaultTimeout)) {
    handleNetworkFailure();
    return false;
  }
  QString chkid = "";
  CheckT check;
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
      check.last_state_change = ngrt4n::humanTimeText(fields[2].toStdString());
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
      check.last_state_change = ngrt4n::humanTimeText(fields[3].toStdString());
      check.check_command = fields[4].toStdString();
      check.alarm_msg = fields[5].toStdString();
    } else {
      QAbstractSocket::setErrorString(tr("Bad request type: %1").arg(reqType));
      return false;
    }
    m_checks.insert(chkid, check);
  }
  return true;
}

bool LsHelper::fecthHostChecks(const QString& host)
{
  m_checks.clear();
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
  check = m_checks.find(id.toLower());
  if (check != m_checks.end()) {
    return true;
  }
  return false;
}

void LsHelper::handleNetworkFailure(QAbstractSocket::SocketError error)
{
  switch (error) {
  case QAbstractSocket::RemoteHostClosedError:
    QAbstractSocket::setErrorString(tr("Connection closed by the remote host"));
    break;
  case QAbstractSocket::HostNotFoundError:
    QAbstractSocket::setErrorString(tr("Host not found (%1).").arg(m_host));
    break;
  case QAbstractSocket::ConnectionRefusedError:
    QAbstractSocket::setErrorString(tr("Connection refused. "
                                       "Make sure that Livestatus API is listening on tcp://%1:%2").arg(m_host).arg(m_port));
    break;
  default:
    QAbstractSocket::setErrorString(tr("The following error occurred (%1)")
                                    .arg(QAbstractSocket::errorString()));
  }
}

