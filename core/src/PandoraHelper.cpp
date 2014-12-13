/*
* PandoraHelper.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 22-11-2014                                                 #
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

#include "PandoraHelper.hpp"
#include "utilsCore.hpp"
#include "JsonHelper.hpp"
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QDebug>
#include <QMessageBox>
#include <QSslConfiguration>


const RequestListT PandoraHelper::ReqPatterns = PandoraHelper::requestsPatterns();


PandoraHelper::PandoraHelper(const QString & baseUrl)
  : QNetworkAccessManager(),
    m_reqHandler(new QNetworkRequest()),
    m_pandoraVersion("UNKNOWN"),
    m_evlHandler(new QEventLoop(this)),
    m_isLogged(false)
{
  setBaseUrl(baseUrl);
  m_reqHandler->setUrl(QUrl(m_apiUri));
  m_reqHandler->setRawHeader("Content-Type", "application/x-www-form-urlencoded");
}

PandoraHelper::~PandoraHelper()
{
  delete m_reqHandler;
  delete m_evlHandler;
}

void
PandoraHelper::setBaseUrl(const QString& url)
{
  m_apiUri = QString("%1/%2").arg(url, PANDORA_API_CONTEXT);
  m_reqHandler->setUrl(QUrl(m_apiUri));
}

QNetworkReply*
PandoraHelper::postRequest(const qint32& reqId, const QStringList& params)
{
  QString request = ReqPatterns[reqId];
  Q_FOREACH(const QString &param, params) {
    request = request.arg(param);
  }

  request = request.arg(m_pandoraApiPass, m_pandoraUsername, m_pandoraPassword);

  QNetworkReply* reply = QNetworkAccessManager::post(*m_reqHandler, ngrt4n::toByteArray(request));
  setSslReplyErrorHandlingOptions(reply);
  connect(reply, SIGNAL(finished()), m_evlHandler, SLOT(quit()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));
  m_evlHandler->exec();
  return reply;
}

RequestListT
PandoraHelper::requestsPatterns()
{
  RequestListT patterns;
  patterns[LoginTest] =
      "op=get"
      "&op2=test"
      "&apipass=%1"
      "&user=%2"
      "&pass=%3";
  patterns[GetTreeAgents] =
      "op=get"
      "&op2=tree_agents"
      "&return_type=csv"
      "&other=?|,|type_row,group_name,agent_name,module_name,module_description,module_data,module_state,module_utimestamp"
      "&other_mode=url_encode_separator_|"
      "&apipass=%1"
      "&user=%2"
      "&pass=%3";
  return patterns;
}

void
PandoraHelper::setSslPeerVerification(bool verifyPeer)
{
  if (verifyPeer) {
    m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
  } else {
    m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
  }
}

int
PandoraHelper::processReply(QNetworkReply* reply, QString& data)
{
  reply->deleteLater();

  // check for error in network communication
  if (reply->error() != QNetworkReply::NoError) {
    m_lastError = tr("Pandora: %1 (%2)").arg(reply->errorString(), reply->url().toString()) ;
    return -1;
  }

  // now read and process respnse data
  data = reply->readAll();
  if (data == "auth error") {
    m_lastError = tr("Pandora: authentication failed");
    return -1;
  }

  if (data.startsWith("OK"))
    return parseLoginTestResult(data);

  return 0;
}

int
PandoraHelper::openSession(const SourceT& srcInfo)
{
  setBaseUrl(srcInfo.mon_url);
  if (checkCredentialsInfo(srcInfo.auth) != 0)
    return -1;

  setSslPeerVerification(srcInfo.verify_ssl_peer != 0);
  QNetworkReply* response = postRequest(LoginTest, QStringList());

  if (! response)
    return -1;

  QString data;
  return processReply(response, data);
}

int
PandoraHelper::loadChecks(const SourceT& srcInfo, ChecksT& checks, const QString& agentName)
{
  checks.clear();

  if (checkCredentialsInfo(srcInfo.auth) != 0)
    return -1;

  setBaseUrl(srcInfo.mon_url);

  QNetworkReply* response = postRequest(GetTreeAgents, QStringList());

  if (! response)
    return -1;

  m_agentFilter = agentName;

  return processModuleReply(response, checks);
}

int
PandoraHelper::processModuleReply(QNetworkReply* reply, ChecksT& checks)
{
  QString data;
  if (processReply(reply, data) != 0)
    return -1;
  QTextStream streamReader(&data, QIODevice::ReadWrite);

  checks.clear();
  CheckT check;
  QString line;
  QString currentAgentName = "";
  QString currentGroupName = "";
  while (line = streamReader.readLine(), ! line.isNull()) {
    /** line :: type_row,group_name,agent_name,module_name,module_description,module_data,module_state,module_utimestamp */

    QStringList fields = line.split("?");
    if (fields.size() != 8) {
      qDebug()<< tr("Invalid module entry: %1").arg(line);
      continue;
    }

    if (! fields.isEmpty()) {
      if (fields[0] == "group") {
        currentGroupName = fields[1];
      } else if (fields[0] == "agent")  {
        currentAgentName = fields[2];
      } else if (fields[0] == "module") {
        QString moduleName        = fields[3].replace("&#x20;", " ");
        QString moduleDescription = fields[4].replace("&#x20;", " ");
        QString moduleValue       = fields[5].replace("&#x20;", " ");
        QString moduleState       = fields[6];
        QString moduleUTimestamp  = fields[7];
        check.host = currentAgentName.toStdString();
        check.id = QString("%1/%2").arg(currentAgentName, moduleName).toStdString(); /* agent_name/module_name */
        check.host_groups = currentGroupName.toStdString();
        check.status = moduleState.toInt();
        check.last_state_change = moduleUTimestamp.toStdString();

        if (! moduleDescription.isEmpty()) {
          check.alarm_msg = tr("%1 value is %2").arg(moduleDescription, moduleValue).toStdString();
        } else {
          check.alarm_msg = tr("%1 value is %2").arg(moduleName, moduleValue).toStdString();
        }

        if (m_agentFilter.isEmpty() || currentGroupName == m_agentFilter || currentAgentName == m_agentFilter) {
          checks.insert(std::pair<std::string, CheckT>(check.id, check));
        }
      }
    }
  }

  if (! data.isEmpty() && checks.empty()) {
    m_lastError = tr("Pandora Error: %1").arg(data);
    return -1;
  }

  return 0;
}

void
PandoraHelper::setSslReplyErrorHandlingOptions(QNetworkReply* reply)
{
  reply->setSslConfiguration(m_sslConfig);
  if (m_sslConfig.peerVerifyMode() == QSslSocket::VerifyNone)
    reply->ignoreSslErrors();
}


std::string
PandoraHelper::parseHostGroups(const QScriptValue& json)
{
  std::string result("");
  QScriptValueIterator entryIter(json);
  while (entryIter.hasNext()) {
    entryIter.next();
    if (entryIter.flags() & QScriptValue::SkipInEnumeration)
      continue;
    std::string name = entryIter.value().property("name").toString().toStdString();

    if (result.empty())
      result = name;
    else
      result.append(ngrt4n::CHILD_SEP).append(name);

  }

  return result;
}


std::string
PandoraHelper::parseHost(const QScriptValue& json)
{
  std::string result("");
  QScriptValueIterator entryIter(json);
  while (entryIter.hasNext()) {
    entryIter.next();
    if (entryIter.flags() & QScriptValue::SkipInEnumeration)
      continue;
    result = entryIter.value().property("host").toString().toStdString();
    break;
  }

  return result;
}

int PandoraHelper::parseLoginTestResult(const QString& data)
{
  QStringList fields = data.split(",");
  if (fields.size() != 3 || fields[0].toLower() != "ok"){
    m_lastError =  tr("Bad login test result: %1").arg(data);
    return -1;
  }
  m_isLogged = true;
  m_pandoraVersion = fields[1];
  return 0;
}



int PandoraHelper::checkCredentialsInfo(const QString& authString)
{
  QStringList params = authString.split(":");
  if (params.size() != 3) {
    m_lastError = tr("Bad auth string, should be in the form of login:password:apipass");
    return -1;
  }

  m_pandoraUsername = params[0];
  m_pandoraPassword = params[1];
  m_pandoraApiPass  = params[2];

  return 0;
}
