/*
* ZnsHelper.cpp
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

#include "ZnsHelper.hpp"
#include "utilsCore.hpp"
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValueIterator>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkCookieJar>
#include <QSslConfiguration>
#include <QNetworkCookie>


const RequestListT ZnsHelper::ReqPatterns = ZnsHelper::requestsPatterns();
const RequestListT ZnsHelper::ContentTypes = ZnsHelper::contentTypes();
const RequestListT ZnsHelper::Routers = ZnsHelper::routers();

ZnsHelper::ZnsHelper(const QString& baseUrl)
  : QNetworkAccessManager(),
    m_apiBaseUrl(baseUrl),
    m_reqHandler(new QNetworkRequest()),
    m_evlHandler(new QEventLoop(this)),
    m_isLogged(false)
{
  m_reqHandler->setUrl(QUrl(m_apiBaseUrl+ZNS_API_CONTEXT));
}

ZnsHelper::~ZnsHelper()
{
  delete m_reqHandler;
  delete m_evlHandler;
}

void ZnsHelper::setBaseUrl(const QString& url)
{
  m_apiBaseUrl = url;
  m_reqHandler->setUrl(QUrl(m_apiBaseUrl+ZNS_LOGIN_API_CONTEXT));
}

QNetworkReply* ZnsHelper::postRequest(const qint32& reqType, const QByteArray& data)
{
  m_reqHandler->setRawHeader("Content-Type", ngrt4n::toByteArray(ContentTypes[reqType]));
  QNetworkReply* reply = QNetworkAccessManager::post(*m_reqHandler, data);
  setSslReplyErrorHandlingOptions(reply);
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
  list[Device] = "{\"action\": \"DeviceRouter\", "
      " \"method\": \"getDevices\", "
      " \"data\": [{ "
      " \"uid\": \"/zport/dmd/Devices\", "
      " \"sort\": \"name\", "
      " \"params\": {\"%1\": \"%2\"}, "
      " \"keys\":[\"name\",\"uid\",\"groups\"] "
      "}], "
      " \"type\": \"rpc\", "
      " \"tid\": %3}";
  list[Component] = "{\"action\": \"DeviceRouter\", "
      " \"method\": \"getComponents\", "
      " \"data\": [{ "
      " \"uid\": \"%1\", "
      " \"limit\": 1000, "
      " \"keys\":[\"name\",\"status\",\"severity\",\"pingStatus\",\"device\",\"failSeverity\",\"lastChanged\",\"groups\"]"
      " }], "
      " \"type\": \"rpc\", "
      " \"tid\": %2}";
  list[DeviceInfo] = "{\"action\": \"DeviceRouter\", "
      " \"method\": \"getInfo\", "
      " \"data\": [{ "
      " \"uid\": \"%1\", "
      " \"keys\":[\"name\",\"status\",\"severity\",\"lastChanged\",\"groups\"] }], "
      " \"type\": \"rpc\", "
      " \"tid\": %2}";
  return list;
}

RequestListT ZnsHelper::routers()
{
  RequestListT list;
  list[Component] = "device_router";
  list[Device] = "device_router";
  return list;
}

void ZnsHelper::setSslPeerVerification(bool verifyPeer)
{
  if (verifyPeer) {
    m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
  } else {
    m_sslConfig.setPeerVerifyMode(QSslSocket::QueryPeer);
  }
}

int
ZnsHelper::parseReply(QNetworkReply* reply)
{
  reply->deleteLater();

  // check for error in network communication
  if (reply->error() != QNetworkReply::NoError) {
    m_lastError = QString("%1 (%2)").arg(reply->errorString(), reply->url().toString()) ;
    return -1;
  }

  // now read data
  m_replyData = reply->readAll();
  m_replyJsonData.setData(m_replyData);

  return 0;
}

bool
ZnsHelper::checkRPCResultStatus(void)
{
  QScriptValue result = m_replyJsonData.getProperty("result");
  bool reqSucceed = result.property("success").toBool();
  if (! reqSucceed) {
    m_lastError = tr("Authentication failed: %1").arg(result.property("msg").toString());
    return false;
  }
  return true;
}

int
ZnsHelper::openSession(const SourceT& srcInfo)
{
  setBaseUrl(srcInfo.mon_url);
  QStringList authInfo = ngrt4n::getAuthInfo(srcInfo.auth);
  if (authInfo.size() != 2) {
    m_lastError = tr("Bad auth string, should be in the form of login:password");
    return -1;
  }

  setBaseUrl(srcInfo.mon_url);

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
  QUrl params;
#else
  QUrlQuery params;
#endif

  params.addQueryItem("__ac_name", authInfo[0]);
  params.addQueryItem("__ac_password", authInfo[1]);
  params.addQueryItem("submitted", "true");
  params.addQueryItem("came_from", getApiContextEndpoint());
  setSslPeerVerification(srcInfo.verify_ssl_peer != 0);
  QNetworkReply* response = NULL;
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
  response = postRequest(ZnsHelper::Login, params.encodedQuery());
#else
  response = postRequest(ZnsHelper::Login, params.query(QUrl::FullyEncoded).toUtf8());
#endif

  if (! response || processLoginReply(response) != 0) {
    return -1;
  }

  return 0;
}

int
ZnsHelper::processLoginReply(QNetworkReply* reply)
{
  if (parseReply(reply) != 0){
    return -1;
  }

  int returnValue = -1;
  QVariant cookiesContainer = reply->header(QNetworkRequest::SetCookieHeader);
  QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(cookiesContainer);
  if (m_replyData.endsWith("submitted=true")) {
    cookieJar()->setCookiesFromUrl(cookies, m_apiBaseUrl);
    m_isLogged =  true;
    returnValue = 0;
  } else {
    if (checkRPCResultStatus()) {
      returnValue = 0;
    }
  }
  return returnValue;
}


int
ZnsHelper::processComponentReply(QNetworkReply* reply, ChecksT& checks)
{
  if (parseReply(reply) != 0){
    return -1;
  }

  if (! checkRPCResultStatus()) {
    return -1;
  }

  // check weird reponse
  qint32 tid = m_replyJsonData.getProperty("tid").toInt32();
  if (tid != ZnsHelper::Component ) {
    m_lastError = tr("Weird transaction type set for component info (%1)").arg(tid);
    return -1;
  }

  // now treat successful result
  CheckT check;
  QScriptValueIterator components(m_replyJsonData.getProperty("result").property("data"));
  while (components.hasNext()) {
    components.next(); if (components.flags()&QScriptValue::SkipInEnumeration) continue;
    QScriptValue citem = components.value();
    QString cname = citem.property("name").toString();
    QScriptValue device = citem.property("device");
    QString dname = device.property("name").toString();
    if (dname.isEmpty()) {
      QString duid = device.property("uid").toString();
      ZnsHelper::getDeviceName(duid);
    }

    check.id = ID_PATTERN.arg(dname, cname).toStdString();
    check.host = dname.toStdString();
    check.host_groups = parseHostGroups(device.property("groups"));
    check.last_state_change = ngrt4n::convertToTimet(device.property("lastChanged").toString(),
                                                     "yyyy/MM/dd hh:mm:ss");
    QString severity =citem.property("severity").toString();
    if (! severity.compare("clear", Qt::CaseInsensitive)) {
      check.status = ngrt4n::ZenossClear;
      check.alarm_msg = tr("%1 component is Up").arg(cname).toStdString();
    } else {
      check.status = citem.property("failSeverity").toInt32();
      check.alarm_msg = citem.property("status").toString().toStdString();
    }
    checks.insert(check.id, check);
  }

  return 0;
}

int
ZnsHelper::processDeviceInfoReply(QNetworkReply* reply, ChecksT& checks)
{
  if (parseReply(reply) != 0){
    return -1;
  }

  if (! checkRPCResultStatus()) {
    return -1;
  }

  // check weird reponse
  qint32 tid = m_replyJsonData.getProperty("tid").toInt32();
  if (tid != DeviceInfo ) {
    m_lastError = tr("Weird transaction type set for device info (%1)").arg(tid);
    return -1;
  }

  // now treat successful result
  CheckT check;
  QScriptValue deviceInfo(m_replyJsonData.getProperty("result").property("data"));
  QString dname = deviceInfo.property("name").toString();
  check.host = dname.toStdString();
  check.id = check.host; //FIXME: ??ID_PATTERN.arg(check.host.c_str(), "ping").toStdString();
  check.host_groups = parseHostGroups(deviceInfo.property("groups"));
  check.status = deviceInfo.property("status").toBool();
  check.last_state_change = ngrt4n::convertToTimet(deviceInfo.property("lastChanged").toString(),
                                                   "yyyy/MM/dd hh:mm:ss");
  if (check.status) {
    check.status = ngrt4n::ZenossClear;
    check.alarm_msg = tr("The host '%1' is Up").arg(dname).toStdString();
  } else {
    check.status = ngrt4n::ZenossCritical;
    check.alarm_msg = tr("The host '%1' is Down").arg(dname).toStdString();
  }
  checks.insert(check.id, check);

  return 0;
}



int
ZnsHelper::processDeviceReply(QNetworkReply* reply, ChecksT& checks)
{
  if (parseReply(reply) != 0)
    return -1;

  if (! checkRPCResultStatus())
    return -1;

  // check weird reponse
  qint32 tid = m_replyJsonData.getProperty("tid").toInt32();
  if (tid != ZnsHelper::Device) {
    m_lastError = tr("Weird transaction type set for device (%1)").arg(tid);
    return -1;
  }

  QScriptValueIterator devices(m_replyJsonData.getProperty("result").property("devices"));
  while (devices.hasNext()) {
    devices.next();
    if (devices.flags()&QScriptValue::SkipInEnumeration) continue;

    QScriptValue curDevice = devices.value();
    QString deviceUid = curDevice.property("uid").toString();
    QNetworkReply* response = NULL;
    response = postRequest(Component, ngrt4n::toByteArray(ReqPatterns[Component].arg(deviceUid, QString::number(Component))));
    processComponentReply(response, checks);

    //retrieve ping info
    response = postRequest(DeviceInfo, ngrt4n::toByteArray(ReqPatterns[DeviceInfo].arg(deviceUid, QString::number(DeviceInfo))));
    processDeviceInfoReply(response, checks);
  }
  return 0;
}


int
ZnsHelper::loadChecks(const SourceT& srcInfo,
                      ChecksT& checks,
                      const QString& filterValue,
                      ngrt4n::RequestFilterT filterType)
{
  setBaseUrl(srcInfo.mon_url);

  // Log in if not yet the case
  if (! m_isLogged && openSession(srcInfo) != 0)
    return -1;

  if (! m_isLogged)
    return -1;

  checks.clear();
  QNetworkReply* response = NULL;

  setRouterEndpoint(Device);
  if (filterType == ngrt4n::GroupFilter) {
    response = postRequest(Device, ngrt4n::toByteArray(ReqPatterns[Device].arg("groups", filterValue, QString::number(Device))));
  } else {
    response = postRequest(Device, ngrt4n::toByteArray(ReqPatterns[Device].arg("name", filterValue, QString::number(Device))));
  }

  if (! response || processDeviceReply(response, checks) !=0) {
    return -1;
  }
  return 0;
}

void
ZnsHelper::setSslReplyErrorHandlingOptions(QNetworkReply* reply)
{
  reply->setSslConfiguration(m_sslConfig);
  if (m_sslConfig.peerVerifyMode() == QSslSocket::VerifyNone)
    reply->ignoreSslErrors();
}


std::string
ZnsHelper::parseHostGroups(const QScriptValue& json)
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
