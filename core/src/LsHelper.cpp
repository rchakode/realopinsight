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

LsHelper::LsHelper(const QString& host, int port)
  : m_socketHandler(new RawSocket(host, port))
{
}

LsHelper::~LsHelper()
{
  delete m_socketHandler;
}

int LsHelper::setupSocket(void)
{
  if (m_socketHandler->setupSocket()) {
    m_lastError = m_socketHandler->lastError();
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
        "Columns: name state last_state_change check_command plugin_output\n";
    break;
  case LsHelper::Service:
    data = "GET services\n"
        "Columns: host_name service_description state last_state_change check_command plugin_output\n";
    break;
  default:
    break;
  }

  if (! host.isEmpty()) {
    QString filterPattern;
    switch(requestType) {
    case LsHelper::Host:
      filterPattern = "Filter: name = %1\n"
          "Filter: host_groups ~ %1\n"
          "Or: 2\n";
      break;
    case LsHelper::Service:
      filterPattern = "Filter: host_name = %1\n"
          "Filter: host_groups ~ %1\n"
          "Or: 2\n";
      break;
    default:
      break;
    }
    data.append(filterPattern.arg(host));
  }
  return ngrt4n::toByteArray(data.append("\n"));
}

int LsHelper::loadChecks(const QString& host, ChecksT& checks)
{
  checks.clear();

  if (makeRequest(prepareRequestData(host, LsHelper::Host), checks) != 0)
    return -1;

  return makeRequest(prepareRequestData(host, LsHelper::Service), checks);
}


int LsHelper::makeRequest(const QByteArray& data, ChecksT& checks)
{
  if (m_socketHandler->makeRequest(data) != 0) {
    m_lastError = m_socketHandler->lastError();
    return -1;
  }
  parseResult(m_socketHandler->lastResult(), checks);
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
      check.last_state_change = fields[2].toStdString();
      check.check_command = fields[3].toStdString();
      check.alarm_msg = fields[4].toStdString();
      break;

    case 6: // service response
      chkid = ID_PATTERN.arg(fields[0], fields[1]).toLower(); // fields[0] => hostname
      check.host = fields[0].toStdString();
      check.id = chkid.toStdString();
      check.status = fields[2].toInt();
      check.last_state_change = fields[3].toStdString();
      check.check_command = fields[4].toStdString();
      check.alarm_msg = fields[5].toStdString();
      break;

    default:
        qDebug()<< "unexpected entry: "<< entry;
      continue;
      break;
    }
    checks.insert(std::pair<std::string, CheckT>(chkid.toStdString(), check));
  }
}
