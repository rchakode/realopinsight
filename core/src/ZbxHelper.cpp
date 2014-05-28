/*
* ZbxHelper.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
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

#include "ZbxHelper.hpp"
#include "utilsCore.hpp"
#include "JsHelper.hpp"
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QDebug>
#include <QMessageBox>
#include <QSslConfiguration>


const RequestListT ZbxHelper::ReqPatterns = ZbxHelper::requestsPatterns();


ZbxHelper::ZbxHelper(const QString & baseUrl)
  : QNetworkAccessManager(),
    m_apiUri(baseUrl%ZBX_API_CONTEXT),
    m_reqHandler(new QNetworkRequest()),
    m_trid(-1),
    m_evlHandler(new QEventLoop(this)),
    m_isLogged(false),
    m_sslConfig(new QSslConfiguration())
{
  m_reqHandler->setRawHeader("Content-Type", "application/json");
  m_reqHandler->setUrl(QUrl(m_apiUri));

}

ZbxHelper::~ZbxHelper()
{
  delete m_reqHandler;
  delete m_evlHandler;
  delete m_sslConfig;
}

QNetworkReply* ZbxHelper::postRequest(const qint32 & reqId, const QStringList & params)
{
  QString request;
  if (reqId == Login) {
    request = ReqPatterns[reqId];
  } else {
    request = ReqPatterns[reqId].arg(m_auth);
  }
  Q_FOREACH(const QString &param, params) { request = request.arg(param); }
  QNetworkReply* reply = QNetworkAccessManager::post(*m_reqHandler, ngrt4n::toByteArray(request));
  reply->setSslConfiguration(*m_sslConfig);
  connect(reply, SIGNAL(finished()), m_evlHandler, SLOT(quit()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));
  m_evlHandler->exec();
  return reply;
}

RequestListT ZbxHelper::requestsPatterns()
{
  RequestListT patterns;
  patterns[Login] = "{\"jsonrpc\": \"2.0\", \
      \"auth\": null, \
      \"method\": \"user.login\", \
      \"params\": {\"user\": \"%1\",\"password\": \"%2\"}, \
      \"id\": %9}";
  patterns[ApiVersion] = "{\"jsonrpc\": \"2.0\", \
      \"method\": \"apiinfo.version\", \
      \"params\": [], \
      \"auth\": \"%1\", \
      \"id\": %9}";
  patterns[Trigger] = "{\"jsonrpc\": \"2.0\", \
      \"auth\": \"%1\", \
      \"method\": \"trigger.get\", \
      \"params\": { \
      \"filter\": { \"host\":[\"%2\"]}, \
      \"selectHosts\": [\"host\"], \
      \"selectItems\": [\"key_\",\"name\",\"lastclock\"], \
      \"output\": [\"description\",\"value\",\"error\",\"comments\",\"priority\"], \
      \"limit\": -1}, \
      \"id\": %9}";
  patterns[TriggerV18] = "{\"jsonrpc\": \"2.0\", \
      \"auth\": \"%1\", \
      \"method\": \"trigger.get\", \
      \"params\": { \
      \"filter\": { \"host\":[\"%2\"]}, \
      \"select_hosts\": [\"host\"], \
      \"output\":  \"extend\", \
      \"limit\": -1}, \
      \"id\": %9}";

  return patterns;
}

void ZbxHelper::setSslConfig(bool verifyPeer)
{
  if (verifyPeer) {
    m_sslConfig->setPeerVerifyMode(QSslSocket::VerifyPeer);
  } else {
    m_sslConfig->setPeerVerifyMode(QSslSocket::QueryPeer);
  }
}

void ZbxHelper::setTrid(const QString& apiv)
{
  qint32 vnum = apiv.mid(0, 3).remove(".").toInt();
  if (vnum < 14) {
    m_trid = TriggerV18;
  } else {
    m_trid = Trigger;
  }
}


int ZbxHelper::processReply(QNetworkReply* reply, ChecksT& checks)
{
  // necessary to properly free resource after processing
  reply->deleteLater();

  // Check for error in network communication
  if (reply->error() != QNetworkReply::NoError) {
    m_lastError = tr("%1 (%2)").arg(reply->errorString(), reply->url().toString()) ;
    return -1;
  }

  // Check for error in RPC result
  QString data = reply->readAll();
  JsonHelper jsHelper(data);
  QString errmsg = jsHelper.getProperty("error").property("data").toString();
  if (errmsg.isEmpty()) errmsg = jsHelper.getProperty("error").property("message").toString();
  if (! errmsg.isEmpty()) {
    m_lastError = errmsg;
    return -1;
  }

  // treat successful result
  int returnValue = 0;
  qint32 tid = jsHelper.getProperty("id").toInt32();
  switch(tid) {
  case ZbxHelper::Login: {
    QString auth = jsHelper.getProperty("result").toString();
    if (!auth.isEmpty()) {
      m_auth = auth;
      m_isLogged = true;
    }
    break;
  }
  case ZbxHelper::ApiVersion: {
    setTrid(jsHelper.getProperty("result").toString());
    break;
  }
  case ZbxHelper::Trigger:
  case ZbxHelper::TriggerV18: {
    QScriptValueIterator trigger(jsHelper.getProperty("result"));
    while (trigger.hasNext()) {

      trigger.next();
      if (trigger.flags()&QScriptValue::SkipInEnumeration) continue;

      QScriptValue triggerData = trigger.value();
      QString triggerName = triggerData.property("description").toString();

      CheckT check;
      check.check_command = triggerName.toStdString();
      check.status = triggerData.property("value").toInt32();
      if (check.status == ngrt4n::ZabbixClear) {
        check.alarm_msg = "OK ("+triggerName.toStdString()+")";
      } else {
        check.alarm_msg = triggerData.property("error").toString().toStdString();
        check.status = triggerData.property("priority").toInteger();
      }
      QString targetHost = "";
      QScriptValueIterator host(triggerData.property("hosts"));
      if (host.hasNext())
      {
        host.next(); if (host.flags()&QScriptValue::SkipInEnumeration) continue;
        QScriptValue hostData = host.value();
        targetHost = hostData.property("host").toString();
        check.host = targetHost.toStdString();
      }
      if (tid == ZbxHelper::TriggerV18) {
        check.last_state_change = triggerData.property("lastchange").toString().toStdString();
      } else {
        QScriptValueIterator item(triggerData.property("items"));
        if (item.hasNext()) {
          item.next(); if (item.flags()&QScriptValue::SkipInEnumeration) continue;
          QScriptValue itemData = item.value();
          check.last_state_change = itemData.property("lastclock").toString().toStdString();
        }
      }
      QString key = ID_PATTERN.arg(targetHost, triggerName);
      check.id = key.toStdString();
      checks.insert(std::pair<std::string, CheckT>(check.id, check));
    }
    break;
  }
  default:
    m_lastError = tr("Weird response received from the server");
    returnValue = -1;
    break;
  }

  return returnValue;
}


int ZbxHelper::loadChecks(const SourceT& srcInfo, const QString& host, ChecksT& checks)
{
  checks.clear();
  setBaseUrl(srcInfo.mon_url);

  QStringList params;
  QNetworkReply* response = NULL;
  // Try to log in if not yet the case
  if (! m_isLogged) {
    params = ngrt4n::getAuthInfo(srcInfo.auth);
    if (params.size() != 2) {
      m_lastError = tr("Bad auth string, should be in the form of login:password");
      return -1;
    }
    params.push_back(QString::number(Login));
    setSslConfig(srcInfo.verify_ssl_peer);
    response = postRequest(Login, params);
    if (! response
        || processReply(response, checks) !=0) {
      return -1;
    }
  }

  // Return if login failed
  if (! m_isLogged) {
    return -1;
  }

  // Get the API version
  params.clear();
  params.push_back(QString::number(ZbxHelper::ApiVersion));
  setSslConfig(srcInfo.verify_ssl_peer);
  response = postRequest(ZbxHelper::ApiVersion, params);
  if (! response
      || processReply(response, checks) !=0) {
    return -1;
  }

  // Finally retriev triggers related to the given host
  // FIXME: if host empty get triggers from all hosts
  params.clear();
  params.push_back(host);
  params.push_back(QString::number(m_trid));
  response = postRequest(m_trid, params);
  if (! response
      || processReply(response, checks) !=0) {
    return -1;
  }

  return 0;
}
