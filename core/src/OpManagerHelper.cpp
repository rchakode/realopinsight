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

  if (checkCredentialsInfo(srcInfo.auth) != 0)
    return -1;

  setBaseUrl(srcInfo.mon_url);
  setApiKey(srcInfo.auth);

  QStringList params = (QStringList() << m_apiKey << filter);
  QNetworkReply* response = postRequest(filterType, params);
  response->deleteLater();
  data = response->readAll();

  processDeviceData(data, deviceList);
  Q_FOREACH(const QString& device, deviceList) { fetchAndAppendDeviceMonitors(device, checks); }

  return 0;
}


int
OpManagerHelper::fetchAndAppendDeviceMonitors(const QString& deviceName, ChecksT& checks)
{
  QStringList params = (QStringList() << m_apiKey << deviceName);
  QNetworkReply* response = postRequest(GetDeviceAssociatedMonitors, params);
  response->deleteLater();
  data = response->readAll();
  processMonitorData(data, checks);
  return 0;
}



void
OpManagerHelper::setSslReplyErrorHandlingOptions(QNetworkReply* reply)
{
  reply->setSslConfiguration(m_sslConfig);
  if (m_sslConfig.peerVerifyMode() == QSslSocket::VerifyNone)
    reply->ignoreSslErrors();
}


std::string
OpManagerHelper::parseHostGroups(const QScriptValue& json)
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
OpManagerHelper::parseHost(const QScriptValue& json)
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

