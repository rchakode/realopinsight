/*
* OpManagerHelper.cpp
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

#include "OpManagerHelper.hpp"
#include "utilsCore.hpp"
#include "JsonHelper.hpp"
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QDebug>
#include <QMessageBox>
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
  m_apiUri = QString("%1/%2").arg(url, OPMANAGER_API_CONTEXT);
  m_reqHandler.setUrl(QUrl(m_apiUri));
}

QNetworkReply*
OpManagerHelper::postRequest(int reqId, const QStringList& params)
{
  QString request = ReqPatterns[reqId];
  Q_FOREACH(const QString& param, params) { request = request.arg(param); }
  request = request.arg(m_apiKey);

  QNetworkReply* reply = QNetworkAccessManager::post(m_reqHandler, ngrt4n::toByteArray(request));
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
  patterns[GetDeviceByName]             = "/device/listDevices?"
                                          "apiKey=%1"
                                          "&deviceName=%2";
  patterns[GetDeviceByType]             = "/device/listDevices?"
                                          "apiKey=%1"
                                          "&type=%2";
  patterns[GetDeviceByCategory]         = "/device/listDevices?"
                                          "apiKey=%1"
                                          "&Category=%2";
  patterns[GetDeviceAssociatedMonitors] = "/device/getAssociatedMonitors?"
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
OpManagerHelper::loadChecks(const SourceT& srcInfo, ChecksT& checks, int filterType, const QString& filter)
{
  checks.clear();

  setBaseUrl(srcInfo.mon_url);
  setApiKey(srcInfo.auth);

  QStringList params = (QStringList() << m_apiKey << filter);
  QNetworkReply* reply = postRequest(filterType, params);

  reply->deleteLater();
  QString data = reply->readAll();

  processDevicesData(data, checks);

  Q_FOREACH(const CheckT& check, checks) { fetchAndAppendDeviceMonitors(check.host.c_str(), checks); }

  return 0;
}


int
OpManagerHelper::fetchAndAppendDeviceMonitors(const QString& deviceName, ChecksT& checks)
{
  QStringList params = (QStringList() << m_apiKey << deviceName);
  QNetworkReply* reply = postRequest(GetDeviceAssociatedMonitors, params);

  reply->deleteLater();
  QString data = reply->readAll();

  processMonitorsData(data, deviceName, checks);

  return 0;
}



void
OpManagerHelper::setSslReplyErrorHandlingOptions(QNetworkReply* reply)
{
  reply->setSslConfiguration(m_sslConfig);
  if (m_sslConfig.peerVerifyMode() == QSslSocket::VerifyNone)
    reply->ignoreSslErrors();
}



void OpManagerHelper::processDevicesData(const QString& data, ChecksT& checks)
{
  JsonHelper json(data);

  QScriptValueIterator deviceIt(json.data());
  while (deviceIt.hasNext()) {
    deviceIt.next(); if (deviceIt.flags() & QScriptValue::SkipInEnumeration) continue;
    QScriptValue deviceJsonObject = deviceIt.value();
    CheckT check;
    check.host = deviceJsonObject.property("deviceName").toString().toStdString();
    check.status = deviceJsonObject.property("numericStatus").toInt32();
    //FIXME: last update time ?
    check.last_state_change = checkLastChangeDate();
    check.id = check.host + "/ping";
    //FIXME: set other properties ?
    checks.insert(check.id, check);
  }
}


void OpManagerHelper::processMonitorsData(const QString& data, const QString& deviceName, ChecksT& checks)
{
  JsonHelper json(data);
  parseNtServiceMonitors(json.getProperty("ntServiceMonitors"), deviceName, checks);
  parseScriptMonitors(json.getProperty("scriptMonitors"), deviceName, checks);
  parsePerformanceMonitors(json.getProperty("performanceMonitors"), deviceName, checks);
  parseUrlMonitors(json.getProperty("urlMonitors"), deviceName, checks);
  parseEvenLogMonitors(json.getProperty("eventlogMonitors"), deviceName, checks);
  parseFolderMonitors(json.getProperty("folderMonitors"), deviceName, checks);
  parseFileMonitors(json.getProperty("fileMonitors").property("ntServiceMonitors"), deviceName, checks);
  parseServerMonitors(json.getProperty("serverMonitors"), deviceName, checks);
  parseProcessMonitors(json.getProperty("processMonitors"), deviceName, checks);
}


void
OpManagerHelper::parseNtServiceMonitors(const QScriptValue& json, const QString& deviceName, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
  }
}



void
OpManagerHelper::parseScriptMonitors(const QScriptValue& json, const QString& deviceName, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
  }
}


void
OpManagerHelper::parsePerformanceMonitors(const QScriptValue& json, const QString& deviceName, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
  }
}


void
OpManagerHelper::parseUrlMonitors(const QScriptValue& json, const QString& deviceName, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
  }
}


void
OpManagerHelper::parseEvenLogMonitors(const QScriptValue& json, const QString& deviceName, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
  }
}


void
OpManagerHelper::parseFolderMonitors(const QScriptValue& json, const QString& deviceName, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
  }
}


void
OpManagerHelper::parseFileMonitors(const QScriptValue& json, const QString& deviceName, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
  }
}

void
OpManagerHelper::parseServerMonitors(const QScriptValue& json, const QString& deviceName, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
  }
}


void
OpManagerHelper::parseProcessMonitors(const QScriptValue& json, const QString& deviceName, ChecksT& checks)
{
  QScriptValueIterator entryIt(json);
  while (entryIt.hasNext()) {
    entryIt.next(); if (entryIt.flags() & QScriptValue::SkipInEnumeration) continue;
    CheckT check;
    QScriptValue entryValue = entryIt.value();
    check.host = deviceName.toStdString();
    check.id = QString("%1/%2").arg(deviceName, entryValue.property("name").toString()).toStdString();
    check.status = entryValue.property("status").toInt32();
    check.last_state_change = checkLastChangeDate();
    checks.insert(check.id, check);
  }
}



