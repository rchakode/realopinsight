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
  reply->setSslConfiguration(m_sslConfig);
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
    m_lastError = tr("%1 (%2)").arg(reply->errorString(), reply->url().toString()) ;
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
  setSslConfig(srcInfo.verify_ssl_peer);
  QNetworkReply* response = NULL;
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
  response = postRequest(ZnsHelper::Login, params.encodedQuery());
#else
  response = postRequest(ZnsHelper::Login, params.query(QUrl::FullyEncoded).toUtf8());
#endif

  if (! response
      || processLoginReply(response) != 0) {
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
  QScriptValueIterator components(m_replyJsonData.getProperty("data"));
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
    QString chkid = ID_PATTERN.arg(dname, cname);
    check.id = chkid.toStdString();
    check.host = dname.toStdString();
    check.last_state_change = ngrt4n::convertToTimet(device.property("lastChanged").toString(),
                                                     "yyyy/MM/dd hh:mm:ss");
    QString severity =citem.property("severity").toString();
    if (!severity.compare("clear", Qt::CaseInsensitive)) {
      check.status = ngrt4n::ZenossClear;
      check.alarm_msg = tr("The %1 component is Up").arg(cname).toStdString();
    } else {
      check.status = citem.property("failSeverity").toInt32();
      check.alarm_msg = citem.property("status").toString().toStdString();
    }
    checks.insert(std::pair<std::string, CheckT>(check.id, check));
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
  QScriptValue devInfo(m_replyJsonData.getProperty("result").property("data"));
  QString dname = devInfo.property("name").toString();
  check.id = check.host = dname.toStdString();
  check.status = devInfo.property("status").toBool();
  check.last_state_change = ngrt4n::convertToTimet(devInfo.property("lastChanged").toString(),
                                                   "yyyy/MM/dd hh:mm:ss");
  if (check.status) {
    check.status = ngrt4n::ZenossClear;
    check.alarm_msg = tr("The host '%1' is Up").arg(dname).toStdString();
  } else {
    check.status = ngrt4n::ZenossCritical;
    check.alarm_msg = tr("The host '%1' is Down").arg(dname).toStdString();
  }
  checks.insert(std::pair<std::string, CheckT>(check.id, check));

  return 0;
}



int
ZnsHelper::processDeviceReply(QNetworkReply* reply, ChecksT& checks)
{
  if (parseReply(reply) != 0){
    return -1;
  }

  if (! checkRPCResultStatus()) {
    return -1;
  }

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
    response = postRequest(Device, ngrt4n::toByteArray(ReqPatterns[DeviceInfo].arg(deviceUid, QString::number(DeviceInfo))));
    processDeviceInfoReply(response, checks);
  }
  return 0;
}


int
ZnsHelper::loadChecks(const SourceT& srcInfo, const QString& host, ChecksT& checks)
{
  setBaseUrl(srcInfo.mon_url);

  // Log in if not yet the case
  if (! m_isLogged
      && openSession(srcInfo) != 0) {
    return -1;
  }

  // check if login succeeded
  if (! m_isLogged) {
    return -1;
  }

  checks.clear();
  QNetworkReply* response = NULL;

  // Finally retriev triggers related to the given host
  // FIXME: if host empty get triggers from all hosts
  response = postRequest(Device, ngrt4n::toByteArray(ReqPatterns[Device].arg(host, QString::number(Device))));

  if (! response
      || processComponentReply(response, checks) !=0) {
    return -1;
  }

  return 0;
}


