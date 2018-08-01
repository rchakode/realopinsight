/*
* OpManagerHelper.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 04-07-2015                                                 #
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

#include "OpManagerHelper.hpp"
#include "utilsCore.hpp"
#include "JsonHelper.hpp"
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QDebug>
#include <QSslConfiguration>


const RequestListT OpManagerHelper::ReqPatterns = OpManagerHelper::requestsPatterns();


OpManagerHelper::OpManagerHelper(const QString& baseUrl)
  : QNetworkAccessManager()
{
  setBaseUrl(baseUrl);
  m_reqHandler.setUrl(QUrl(m_apiUri));
  m_reqHandler.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
}

OpManagerHelper::~OpManagerHelper()
{
}

void
OpManagerHelper::setBaseUrl(const QString& url)
{
  if (url.endsWith("/")) {
    m_apiUri = QString("%1%2").arg(url, OPMANAGER_API_CONTEXT);
  } else {
    m_apiUri = QString("%1/%2").arg(url, OPMANAGER_API_CONTEXT);
  }
  m_reqHandler.setUrl(QUrl(m_apiUri));
}

QNetworkReply*
OpManagerHelper::postRequest(int reqId, const QStringList& params)
{
  QString requestContext = ReqPatterns[reqId];
  Q_FOREACH(const QString& param, params) { requestContext = requestContext.arg(param); }
  QString reqUrl = QString("%1%2").arg(m_apiUri, requestContext);
  m_reqHandler.setUrl(QUrl(reqUrl));

  QNetworkReply* reply = QNetworkAccessManager::get(m_reqHandler);
  setSslReplyErrorHandlingOptions(reply);
  connect(reply, SIGNAL(finished()), &m_evlHandler, SLOT(quit()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));
  m_evlHandler.exec();
  return reply;
}

RequestListT
OpManagerHelper::requestsPatterns()
{
  RequestListT patterns;
  patterns[ListAllDevices]               = "/device/listDevices?"
                                           "apiKey=%1";
  patterns[ListDeviceByName]             = "/device/listDevices?"
                                           "apiKey=%1"
                                           "&deviceName=%2";
  patterns[ListDeviceByType]             = "/device/listDevices?"
                                           "apiKey=%1"
                                           "&type=%2";
  patterns[ListDeviceByCategory]         = "/device/listDevices?"
                                           "apiKey=%1"
                                           "&category=%2";
  patterns[ListDeviceAssociatedMonitors] = "/device/getAssociatedMonitors?"
                                           "apiKey=%1"
                                           "&name=%2";
  return patterns;
}

void
OpManagerHelper::setSslPeerVerification(bool verifyPeer)
{
  if (verifyPeer) {
    m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
  } else {
    m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
  }
}


int
OpManagerHelper::loadChecks(const SourceT& srcInfo, int filterType, const QString& filter, ChecksT& checks)
{
  checks.clear();

  setBaseUrl(srcInfo.mon_url);
  setApiKey(srcInfo.auth);

  QStringList params(m_apiKey);
  if (filterType != ListAllDevices) params.push_back(filter);
  QNetworkReply* reply = postRequest(filterType, params);

  reply->deleteLater();
  QString data = reply->readAll();
  if (reply->error() != QNetworkReply::NoError) {
    m_lastError = reply->errorString();
    return ngrt4n::RcGenericFailure;
  }

  if (checkJsonData(data)) {
    return ngrt4n::RcGenericFailure;
  }

  processDevicesJsonData(JsonHelper(data).data(), checks);
  Q_FOREACH(const CheckT& check, checks) { fetchAndAppendDeviceMonitors(check.host, check.host_groups, checks); }

  return ngrt4n::RcSuccess;
}


int
OpManagerHelper::fetchAndAppendDeviceMonitors(const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks)
{
  QStringList params = (QStringList() << m_apiKey << deviceName.c_str());
  QNetworkReply* reply = postRequest(ListDeviceAssociatedMonitors, params);

  reply->deleteLater();
  QString data = reply->readAll();

  if (checkJsonData(data))
    return ngrt4n::RcGenericFailure;

  processMonitorsJsonData(JsonHelper(data).data(), deviceName, deviceGroups, checks);

  return ngrt4n::RcSuccess;
}



void
OpManagerHelper::setSslReplyErrorHandlingOptions(QNetworkReply* reply)
{
  reply->setSslConfiguration(m_sslConfig);
  if (m_sslConfig.peerVerifyMode() == QSslSocket::VerifyNone)
    reply->ignoreSslErrors();
}

bool OpManagerHelper::checkJsonData(const QString& data)
{
  JsonHelper jsonHelper(data);
  if (! jsonHelper.isGood()) {
    m_lastError = jsonHelper.lastError();
    return jsonHelper.isGood();
  }

  bool success = jsonHelper.getProperty("error").toString().isEmpty();
  if (! success) {
    m_lastError = jsonHelper.getProperty("error").property("message").toString();
  } else {
    m_lastError = jsonHelper.getProperty("message").toString();
    success = m_lastError.isEmpty();
  }
  return ! success;
}

void OpManagerHelper::processDevicesJsonData(const QScriptValue& json, ChecksT& checks)
{
  QScriptValueIterator deviceIt(json);
  while (deviceIt.hasNext()) {
    deviceIt.next(); if (deviceIt.flags() & QScriptValue::SkipInEnumeration) continue;
    QScriptValue deviceJsonObject = deviceIt.value();
    QString deviceName = deviceJsonObject.property("deviceName").toString();
    CheckT check;
    check.host = deviceName.toStdString();
    check.id = check.host + "/ping";
    check.status = deviceJsonObject.property("numericStatus").toInt32();
    check.alarm_msg = statusAlarmMessage(deviceName, "device", check.status);
    check.last_state_change = currentLastChangeDate(); //FIXME: last update time ?
    check.host_groups = QString("%1,%2").arg(deviceJsonObject.property("category").toString(),
                                             deviceJsonObject.property("type").toString()).toStdString();
    checks.insert(check.id, check);
  }
}


void OpManagerHelper::processMonitorsJsonData(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks)
{
  typedef void (*ParseMonitorJson)(const QScriptValue&, const std::string&, const std::string&, ChecksT&);
  QMap<QString, ParseMonitorJson> parsers;
  parsers["ntServiceMonitors"] = &OpManagerHelper::parseNtServiceMonitors;
  parsers["scriptMonitors"] = &OpManagerHelper::parseScriptMonitors;
  parsers["performanceMonitors"] = &OpManagerHelper::parsePerformanceMonitors;
  parsers["urlMonitors"] = &OpManagerHelper::parseUrlMonitors;
  parsers["eventlogMonitors"] = &OpManagerHelper::parseEvenLogMonitors;
  parsers["folderMonitors"] = &OpManagerHelper::parseFileMonitors;
  parsers["fileMonitors"] = &OpManagerHelper::parseFileMonitors;
  parsers["serverMonitors"] = &OpManagerHelper::parseServerMonitors;
  parsers["processMonitors"] = &OpManagerHelper::parseProcessMonitors;

  Q_FOREACH(const QString& parserName, parsers.keys())
    parsers[parserName](json.property(parserName).property("monitors"), deviceName, deviceGroups, checks);
}


void
OpManagerHelper::parseNtServiceMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
    CheckT check;
    QScriptValue entryValue = entryIt.value();
    QString entryName = entryValue.property("serviceName").toString();
    check.id = QString("%1/%2").arg(deviceName.c_str(), entryName).toStdString();
    check.host = deviceName;
    check.host_groups = deviceGroups;
    check.status = statusFromIconPath( entryValue.property("status").toString() );
    check.last_state_change = currentLastChangeDate();
    check.alarm_msg = statusAlarmMessage(entryName, "ntservice", check.status);
    checks.insert(check.id, check);
  }
}



void
OpManagerHelper::parseScriptMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
    CheckT check;
    QScriptValue entryValue = entryIt.value();
    QString entryName = entryValue.property("displayName").toString();
    check.id = QString("%1/%2").arg(deviceName.c_str(), entryName).toStdString();
    check.host = deviceName;
    check.host_groups = deviceGroups;
    check.status = statusFromIconPath( entryValue.property("statusIcon").toString() );
    check.last_state_change = currentLastChangeDate();
    check.alarm_msg = statusAlarmMessage(entryName, "script", check.status);
    checks.insert(check.id, check);
  }
}


void
OpManagerHelper::parsePerformanceMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks)
{
  /** do nothing right now: cannot find status information in  the returned json **/
}


void
OpManagerHelper::parseUrlMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
    CheckT check;
    QScriptValue entryValue = entryIt.value();
    QString entryName = entryValue.property("urlName").toString();
    QString url = entryValue.property("url").toString();
    QString availPercentage = entryValue.property("availPercentage").toString();
    QString stateText = entryValue.property("state").toString();
    check.id = QString("%1/%2").arg(deviceName.c_str(), entryName).toStdString();
    check.host = deviceName;
    check.host_groups = deviceGroups;
    check.status = entryValue.property("status").toInt32();
    check.last_state_change = currentLastChangeDate();
    check.alarm_msg = QObject::tr("Check URL: %1 - availability: %2 - state: %3").arg(url, availPercentage, stateText).toStdString();
    checks.insert(check.id, check);
  }
}


void
OpManagerHelper::parseEvenLogMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks)
{
  /** do nothing right now: cannot find status information in  the returned json **/
}


void
OpManagerHelper::parseFileMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
    CheckT check;
    QScriptValue entryValue = entryIt.value();
    QString entryName = entryValue.property("monitorname").toString();
    QString path = entryValue.property("folderpath").toString() + entryValue.property("filepath").toString();
    QString exist = entryValue.property("FolderExist").toString() + entryValue.property("FileExist").toString();
    QString stateText = entryValue.property("state").toString();
    check.id = QString("%1/%2").arg(deviceName.c_str(), entryName).toStdString();
    check.host = deviceName;
    check.host_groups = deviceGroups;
    check.status = entryValue.property("status").toInt32();
    check.last_state_change = currentLastChangeDate();
    check.alarm_msg = QObject::tr("Check Path: %1 - exist: %2 - state: %3").arg(path, exist, stateText).toStdString();
    checks.insert(check.id, check);
  }
}

void
OpManagerHelper::parseServerMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
    CheckT check;
    QScriptValue entryValue = entryIt.value();
    QString entryName = entryValue.property("name").toString();
    check.id = QString("%1/%2").arg(deviceName.c_str(), entryName).toStdString();
    check.host = deviceName;
    check.host_groups = deviceGroups;
    check.status = statusFromIconPath( entryValue.property("status").toString() );
    check.last_state_change = currentLastChangeDate();
    check.alarm_msg = statusAlarmMessage(entryName, "server", check.status);
    checks.insert(check.id, check);
  }
}


void
OpManagerHelper::parseProcessMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
    CheckT check;
    QScriptValue entryValue = entryIt.value();
    QString entryName = entryValue.property("name").toString();
    check.id = QString("%1/%2").arg(deviceName.c_str(), entryName).toStdString();
    check.host = deviceName;
    check.host_groups = deviceGroups;
    check.status = entryValue.property("status").toInt32();
    check.last_state_change = currentLastChangeDate();
    check.alarm_msg = statusAlarmMessage(entryName, "process", check.status);
    checks.insert(check.id, check);
  }
}

std::string OpManagerHelper::statusAlarmMessage(const QString& itemName, const QString& itemType, int status)
{
  QString result = QObject::tr("Unknown status: %1").arg(QString::number(status));
  switch (status) {
    case 5:
      result = QObject::tr("%1[%2] Status is clear").arg(itemName, itemType);
      break;
    case 4:
      result = QObject::tr("%1 [%2]: Status is offline/inaccessible").arg(itemName, itemType);
      break;
    case 3:
      result = QObject::tr("%1 [%2]: Status requires attention").arg(itemName, itemType);
      break;
    case 2:
      result = QObject::tr("%1 [%2]: Status is in trouble").arg(itemName, itemType);
      break;
    case 1:
      result = QObject::tr("%1 [%2]: Status is critical").arg(itemName, itemType);
      break;
    default:
      break;
  }
  return result.toStdString();
}


int OpManagerHelper::statusFromIconPath(const QString& iconPath)
{
  int result = 1;
  QString bs = ngrt4n::basename(iconPath);

  if (bs.indexOf("status1") != -1)
    result = 1;
  else if (bs.indexOf("status2") != -1)
    result = 2;
  else if (bs.indexOf("status3") != -1)
    result = 3;
  else if (bs.indexOf("status4") != -1)
    result = 4;
  else if (bs.indexOf("status5") != -1)
    result = 5;
  return result;
}
