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
  m_requestMap[Host] = "GET hosts\n"
      "Columns: name state last_state_change check_command plugin_output\n"
      "Filter: name = %1\n\n";
  m_requestMap[Service] = "GET services\n"
      "Columns: host_name service_description state last_state_change check_command plugin_output\n"
      "Filter: host_name = %1\n\n";
}

int LsHelper::connectToService()
{
  disconnectFromHost();
  QAbstractSocket::connectToHost(m_host, m_port, QAbstractSocket::ReadWrite);
  if (!QAbstractSocket::waitForConnected(DefaultTimeout)) {
    handleNetworkFailure();
    return -1;
  }
  return 0;
}


void LsHelper::disconnectFromService()
{
  QAbstractSocket::disconnectFromHost();
}

int LsHelper::requestData(const QString& host, const ReqTypeT& reqType)
{
  qint32 byteExchanged;
  if (! isConnected() && ! connectToService())
    return -1;

  if (! isConnected())
    return -1;

  byteExchanged = QAbstractSocket::write(ngrt4n::toByteArray(m_requestMap[reqType].arg(host)));
  if (byteExchanged <= 0 || ! QAbstractSocket::waitForBytesWritten(DefaultTimeout)) {
    handleNetworkFailure();
    return -1;
  }

  return 0;
}

int LsHelper::recvData(const ReqTypeT& reqType, ChecksT& checks)
{
  if (!QAbstractSocket::waitForReadyRead(DefaultTimeout)) {
    handleNetworkFailure();
    return -1;
  }
  QString chkid = "";
  CheckT check;
  QString entry;
  QTextStream buffer(this);

  while (!((entry = buffer.readLine()).isNull())) {

    if (entry.isEmpty()) continue;

    QStringList fields = entry.split(";");

    switch(reqType) {
    case Host:

      if (fields.size() != 5)
        continue;

      chkid = fields[0].toLower();
      check.id = check.host = fields[0].toStdString();
      check.status = fields[1].toInt();
      check.last_state_change = ngrt4n::humanTimeText(fields[2].toStdString());
      check.check_command = fields[3].toStdString();
      check.alarm_msg = fields[4].toStdString();
      break;

    case Service:

      if (fields.size() != 6)
        continue;

      // fields[0] => hostname
      chkid = ID_PATTERN.arg(fields[0], fields[1]).toLower();
      check.host = fields[0].toStdString();
      check.id = chkid.toStdString();
      check.status = fields[2].toInt();
      check.last_state_change = ngrt4n::humanTimeText(fields[3].toStdString());
      check.check_command = fields[4].toStdString();
      check.alarm_msg = fields[5].toStdString();
      break;

    default:
      m_lastError = tr("Bad request type: %1").arg(reqType);
      continue;
      break;
    }

    checks.insert(std::pair<std::string, CheckT>(chkid.toStdString(), check));
  }
  return 0;
}


int LsHelper::loadChecks(const QString& host, ChecksT& checks)
{
  checks.clear();
  if (connectToService() != 0)
    return -1;

  if (! isConnected()) {
    return -1;
  }

  if (requestData(host, Host) != 0)
    return -1;

  if (recvData(Host, checks) != 0)
    return -1;

  disconnectFromService();

  return 0;
}

