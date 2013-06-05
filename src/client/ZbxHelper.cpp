/*
 * ZbxHelper.cpp
 # ------------------------------------------------------------------------ #
 # Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
 # Last Update : 4 août 2012                                                #
 #                                                                          #
 # This file is part of NGRT4N (http://ngrt4n.com).                         #
 #                                                                          #
 # NGRT4N is free software: you can redistribute it and/or modify           #
 # it under the terms of the GNU General Public License as published by     #
 # the Free Software Foundation, either version 3 of the License, or        #
 # (at your option) any later version.                                      #
 #                                                                          #
 # NGRT4N is distributed in the hope that it will be useful,                #
 # but WITHOUT ANY WARRANTY; without even the implied warranty of           #
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
 # GNU General Public License for more details.                             #
 #                                                                          #
 # You should have received a copy of the GNU General Public License        #
 # along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
 #--------------------------------------------------------------------------#
 */

#include "ZbxHelper.hpp"
#include <QtScript/QScriptEngine>
#include <QDebug>
#include <QMessageBox>


const RequestListT ZbxHelper::ReqPatterns = ZbxHelper::requestsPatterns();


ZbxHelper::ZbxHelper(const QString & baseUrl)
  : QNetworkAccessManager(),
    m_apiUri(baseUrl%ZBX_API_CONTEXT),
    m_reqHandler(new QNetworkRequest()),
    m_trid(-1),
    m_evlHandler(new QEventLoop(this)),
    m_isLogged(false)
{
  m_reqHandler->setRawHeader("Content-Type", "application/json");
  m_reqHandler->setUrl(QUrl(m_apiUri));

}

ZbxHelper::~ZbxHelper()
{
  delete m_reqHandler;
  delete m_evlHandler;
  delete sslConf;
}

QNetworkReply* ZbxHelper::postRequest(const qint32 & reqId, const QStringList & params)
{
  QString request;
  if (reqId == Login) {
    request = ReqPatterns[reqId];
  } else {
    request = ReqPatterns[reqId].arg(m_auth);
  }
  foreach(const QString &param, params) { request = request.arg(param); }

  QNetworkReply* reply = QNetworkAccessManager::post(*m_reqHandler, request.toAscii());
  reply->setSslConfiguration(*sslConf);
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

void ZbxHelper::setSslConf(bool verifyPeer)
{
  if (verifyPeer) {
    sslConf->setPeerVerifyMode(QSslSocket::VerifyPeer);
  } else {
    sslConf->setPeerVerifyMode(QSslSocket::QueryPeer);
  }
}
