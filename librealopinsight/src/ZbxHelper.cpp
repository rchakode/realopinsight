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

  QNetworkReply* reply = QNetworkAccessManager::post(*m_reqHandler, request.toAscii());
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
