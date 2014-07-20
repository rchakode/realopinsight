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
#include "RawSocket.hpp"

LsHelper::LsHelper(const QString& host, const int& port)
  : m_host(host),
    m_port(port)
{
}

LsHelper::~LsHelper()
{
}

int LsHelper::setupSocket(void)
{
  if (m_socketHandler.setupSocket(m_host, m_port)) {
    m_lastError = m_socketHandler.lastError();
    return -1;
  }
  return 0;
}

QByteArray LsHelper::prepareRequestData(const QString& host, ReqTypeT requestType)
{
  QString data = "";
  switch(requestType) {
  case LsHelper::Host:
    data = "GET hosts\n"
        "Columns: name state last_state_change check_command plugin_output\n"
        "Filter: name = %1\n\n";
    break;
  case LsHelper::Service:
    data = "GET services\n"
        "Columns: host_name service_description state last_state_change check_command plugin_output\n"
        "Filter: host_name = %1\n\n";
    break;
  default:
    break;
  }

  return ngrt4n::toByteArray(data.arg(host));
}

int LsHelper::loadChecks(const QString& host, ChecksT& checks)
{
  checks.clear();

  // get host data
  if (m_socketHandler.makeRequest(prepareRequestData(host, LsHelper::Host)) != 0) {
    m_lastError = m_socketHandler.lastError();
  }
  parseResult(m_socketHandler.lastResult(), checks);

  // get service data
  if (m_socketHandler.makeRequest(prepareRequestData(host, LsHelper::Service)) != 0) {
    m_lastError = m_socketHandler.lastError();
  }
  parseResult(m_socketHandler.lastResult(), checks);
  return 0;
}

int LsHelper::makeRpcCall(const QString& host, ReqTypeT requestType)
{
  QByteArray data = prepareRequestData(host, requestType);

  WSADATA WSAData;
  SOCKET sock;
  SOCKADDR_IN sin;
  char buffer[1024*1024];
  WSAStartup(MAKEWORD(2,0), &WSAData);
  /* Tout est configuré pour se connecter sur IRC, haarlem, Undernet. */
  sock = socket(AF_INET, SOCK_STREAM, 0);
  sin.sin_addr.s_addr = inet_addr(m_host.toStdString().c_str());
  sin.sin_family = AF_INET;
  sin.sin_port = htons(m_port);
  connect(sock, (SOCKADDR *)&sin, sizeof(sin));

  send(sock, data, data.size(), 0);
  recv(sock, buffer, sizeof(buffer), 0);
  closesocket(sock);
  WSACleanup();
  return 0;
}


void LsHelper::parseResult(const QString& result, ChecksT& checks)
{
  QString chkid = "";
  CheckT check;
  QString entry;

  QTextStream stream(result.toLatin1(), QIODevice::ReadOnly);

  while (!((entry = stream.readLine()).isNull())) {
    if (entry.isEmpty()) continue;
    QStringList fields = entry.split(";");

    switch( fields.size() ) {
    case 5: // host response
      chkid = fields[0].toLower();
      check.id = check.host = fields[0].toStdString();
      check.status = fields[1].toInt();
      check.last_state_change = ngrt4n::humanTimeText(fields[2].toStdString());
      check.check_command = fields[3].toStdString();
      check.alarm_msg = fields[4].toStdString();
      break;

    case 6: // service response
      chkid = ID_PATTERN.arg(fields[0], fields[1]).toLower(); // fields[0] => hostname
      check.host = fields[0].toStdString();
      check.id = chkid.toStdString();
      check.status = fields[2].toInt();
      check.last_state_change = ngrt4n::humanTimeText(fields[3].toStdString());
      check.check_command = fields[4].toStdString();
      check.alarm_msg = fields[5].toStdString();
      break;

    default:
      continue;
      break;
    }
    checks.insert(std::pair<std::string, CheckT>(chkid.toStdString(), check));
  }
}
