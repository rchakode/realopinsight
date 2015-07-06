/*
 * OpManagerHelper.hpp
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

#ifndef OPMANAGERHELPERR_HPP_
#define OPMANAGERHELPERR_HPP_
#include "Base.hpp"
#include "JsonHelper.hpp"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QSslConfiguration>


namespace {
  const QString OPMANAGER_API_CONTEXT = "api/json";
}

class OpManagerHelper : public QNetworkAccessManager {
    Q_OBJECT
  public:
    enum {
      ListAllDevices = 0,
      ListDeviceByName = 1,
      ListDeviceByType = 2,
      ListDeviceByCategory = 4,
      ListDeviceAssociatedMonitors = 5
    };
    static const RequestListT ReqPatterns;

  public:
    OpManagerHelper(const QString& baseUrl="http://localhost/");
    virtual ~OpManagerHelper();
    int loadChecks(const SourceT& srcInfo, int filterType, const QString& filter, ChecksT& checks);
    QString lastError(void) const {return m_lastError;}


  public Q_SLOTS:
    void processError(const QNetworkReply::NetworkError& code) { m_evlHandler.exit(code);}

  Q_SIGNALS:
    void propagateError(QNetworkReply::NetworkError);

  private :
    static RequestListT requestsPatterns();
    QString m_apiUri;
    QString m_apiKey;
    QNetworkRequest m_reqHandler;
    QEventLoop m_evlHandler;
    QSslConfiguration m_sslConfig;
    QString m_lastError;
    QString m_replyData;

    QNetworkReply* postRequest(int reqId, const QStringList& params);
    int fetchAndAppendDeviceMonitors(const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks);
    void setBaseUrl(const QString& url);
    void setApiKey(const QString& key) {m_apiKey = key;}
    void setSslPeerVerification(bool verifyPeer);
    bool checkJsonData(const QString& data);
    void setSslReplyErrorHandlingOptions(QNetworkReply* reply);
    static void processDevicesJsonData(const QScriptValue& data, ChecksT& checks);
    static void processMonitorsJsonData(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks);
    static void parseNtServiceMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& check);
    static void parseScriptMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks);
    static void parsePerformanceMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks);
    static void parseUrlMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks);
    static void parseEvenLogMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks);
    static void parseFileMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks);
    static void parseServerMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks);
    static void parseProcessMonitors(const QScriptValue& json, const std::string& deviceName, const std::string& deviceGroups, ChecksT& checks);
    static std::string currentLastChangeDate(void) {return QString::number( QDateTime::currentDateTime().toTime_t() ).toStdString();}
    static std::string statusAlarmMessage(const QString& itemName, const QString& itemType, int status);
    static int statusFromIconPath(const QString& iconPath);

};

#endif /* OPMANAGERHELPERR_HPP_ */
