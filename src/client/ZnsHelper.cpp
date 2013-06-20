/*
* ZnsHelper.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 6 Decembre 2012                                            #
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

#include "ZnsHelper.hpp"
#include <QtScript/QScriptEngine>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkCookieJar>


const RequestListT ZnsHelper::ReqPatterns = ZnsHelper::requestsPatterns();
const RequestListT ZnsHelper::ContentTypes = ZnsHelper::contentTypes();
const RequestListT ZnsHelper::Routers = ZnsHelper::routers();

ZnsHelper::ZnsHelper(const QString& baseUrl)
  : QNetworkAccessManager(),
    m_apiBaseUrl(baseUrl),
    m_reqHandler(new QNetworkRequest()),
    m_evlHandler(new QEventLoop(this)),
    m_isLogged(false),
    m_sslConfig(new QSslConfiguration())
{
  m_reqHandler->setUrl(QUrl(m_apiBaseUrl+ZNS_API_CONTEXT));
}

ZnsHelper::~ZnsHelper()
{
  delete m_reqHandler;
  delete m_evlHandler;
  delete m_sslConfig;
}

void ZnsHelper::setBaseUrl(const QString& url)
{
  m_apiBaseUrl = url;
  m_reqHandler->setUrl(QUrl(m_apiBaseUrl+ZNS_LOGIN_API_CONTEXT));
}

QNetworkReply* ZnsHelper::postRequest(const qint32& reqType, const QByteArray& data)
{
  m_reqHandler->setRawHeader("Content-Type", ContentTypes[reqType].toAscii());
  QNetworkReply* reply = QNetworkAccessManager::post(*m_reqHandler, data);
  reply->setSslConfiguration(*m_sslConfig);
  connect(reply, SIGNAL(finished()), m_evlHandler, SLOT(quit()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));
  m_evlHandler->exec();
  return reply;
}

void ZnsHelper::setRouterEndpoint(const int& reqType) {
  QString url = m_apiBaseUrl+ZNS_API_CONTEXT + "/" + Routers[reqType];
  setRequestEndpoint(url);
}

RequestListT ZnsHelper::contentTypes()
{
  RequestListT list;
  list[Login] = "application/x-www-form-urlencoded";
  list[Component] = "application/json; charset=utf-8";
  list[Device] = "application/json; charset=utf-8";
  return list;
}


RequestListT ZnsHelper::requestsPatterns()
{
  RequestListT list;
  list[Device] = "{\"action\": \"DeviceRouter\", \
      \"method\": \"getDevices\", \
      \"data\": [{\
      \"uid\": \"/zport/dmd/Devices\", \
      \"sort\": \"name\", \
      \"params\": {\"name\": \"%1\"},  \
      \"keys\":[\"name\",\"uid\"] }], \
      \"type\": \"rpc\", \
      \"tid\": %2}";
  list[Component] = "{\"action\": \"DeviceRouter\", \
      \"method\": \"getComponents\", \
      \"data\": [{\
      \"uid\": \"%1\", \
      \"limit\": 1000, \
      \"keys\":[\"name\",\"status\",\"severity\",\"pingStatus\",\"device\",\"failSeverity\",\"lastChanged\"]}], \
      \"type\": \"rpc\", \
      \"tid\": %2}";
  list[DeviceInfo] = "{\"action\": \"DeviceRouter\", \
      \"method\": \"getInfo\", \
      \"data\": [{\
      \"uid\": \"%1\", \
      \"keys\":[\"name\",\"status\",\"severity\",\"lastChanged\"] }], \
      \"type\": \"rpc\", \
      \"tid\": %2}";
  return list;
}

RequestListT ZnsHelper::routers()
{
  RequestListT list;
  list[Component] = "device_router";
  list[Device] = "device_router";
  return list;
}

void ZnsHelper::setSslConfig(bool verifyPeer)
{
  if (verifyPeer) {
    m_sslConfig->setPeerVerifyMode(QSslSocket::VerifyPeer);
  } else {
    m_sslConfig->setPeerVerifyMode(QSslSocket::QueryPeer);
  }
}

