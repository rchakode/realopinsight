/*
 * ZnsHelper.cpp
 # ------------------------------------------------------------------------ #
 # Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
 # Last Update : 6 Decembre 2012                                            #
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

#include "ZnsHelper.hpp"
#include <QtScript/QScriptEngine>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkCookieJar>


const RequestListT ZnsHelper::ReQPatterns = ZnsHelper::getRequestsPatterns();
const RequestListT ZnsHelper::ContentTypes = ZnsHelper::getContentTypes();
const RequestListT ZnsHelper::Routers = ZnsHelper::getRouters();

ZnsHelper::ZnsHelper(const QString& baseUrl)
  : QNetworkAccessManager(),
    mapiBaseUrl(baseUrl),
    mrequestHandler(new QNetworkRequest())
{
  mrequestHandler->setUrl(QUrl(mapiBaseUrl+ZNS_API_CONTEXT));
}

ZnsHelper::~ZnsHelper()
{
  delete mrequestHandler;
}

void ZnsHelper::setBaseUrl(const QString& url)
{
  mapiBaseUrl = url;
  mrequestHandler->setUrl(QUrl(mapiBaseUrl+ZNS_LOGIN_API_CONTEXT));
}

void ZnsHelper::postRequest(const qint32& reqType, const QByteArray& data)
{
  mrequestHandler->setRawHeader("Content-Type", ContentTypes[reqType].toAscii());
  QNetworkReply* reply = QNetworkAccessManager::post(*mrequestHandler, data);
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));
}

void ZnsHelper::setRouter(const int& reqType) {
  QString url = mapiBaseUrl+ZNS_API_CONTEXT + "/" + Routers[reqType];
  setRequestUrl(url);
}

RequestListT ZnsHelper::getContentTypes()
{
  RequestListT list;
  list[LOGIN] = "application/x-www-form-urlencoded";
  list[COMPONENT] = "application/json; charset=utf-8";
  list[DEVICE] = "application/json; charset=utf-8";
  return list;
}


RequestListT ZnsHelper::getRequestsPatterns()
{
  RequestListT list;
  list[DEVICE] = "{\"action\": \"DeviceRouter\", \
      \"method\": \"getDevices\", \
      \"data\": [{\
      \"uid\": \"/zport/dmd/Devices\", \
      \"sort\": \"name\", \
      \"params\": {\"name\": \"%1\"},  \
      \"keys\":[\"name\",\"uid\"] }], \
      \"type\": \"rpc\", \
      \"tid\": %2}";
  list[COMPONENT] = "{\"action\": \"DeviceRouter\", \
      \"method\": \"getComponents\", \
      \"data\": [{\
      \"uid\": \"%1\", \
      \"limit\": 1000, \
      \"keys\":[\"name\",\"status\",\"severity\",\"pingStatus\",\"device\",\"failSeverity\",\"lastChanged\"]}], \
      \"type\": \"rpc\", \
      \"tid\": %2}";
  return list;
}

RequestListT ZnsHelper::getRouters()
{
  RequestListT list;
  list[COMPONENT] = "device_router";
  list[DEVICE] = "device_router";
  return list;
}

