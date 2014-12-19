/*
 * ZbxHelper.hpp
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

#ifndef ZABBIXHELPER_HPP_
#define ZABBIXHELPER_HPP_
#include "Base.hpp"
#include "JsonHelper.hpp"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QSslConfiguration>


namespace {
  const QString ZBX_API_CONTEXT = "/api_jsonrpc.php";
  }

class ZbxHelper : public QNetworkAccessManager {
  Q_OBJECT
public:
  enum {
    GetLogin=1,
    GetApiVersion=2,
    GetTriggersByHostGroup=3,
    GetTriggersByHostGroupV18=4,
    GetItServiceTriggersByIds = 6,
    GetItServices = 5
  };
  static const RequestListT ReqPatterns;

public:
  ZbxHelper(const QString& baseUrl="http://localhost/zabbix");
  virtual ~ZbxHelper();
  int postRequest(qint32 reqId, const QStringList& params);
  void setBaseUrl(const QString& url) {m_apiUri = url%ZBX_API_CONTEXT; m_reqHandler->setUrl(QUrl(m_apiUri));}
  void setTrid(const QString& apiv);
  QString lastError(void) const {return m_lastError;}
  void setSslPeerVerification(bool verifyPeer);
  int parseReply(QNetworkReply* reply);
  bool backendReturnedSuccessResult(void);
  int openSession(const SourceT& srcInfo);
  int loadChecks(const SourceT& srcInfo, ChecksT& checks,
             const QString& filterValue, ngrt4n::RequestFilterT filterType = ngrt4n::HostFilter);
  int loadITServices(const SourceT& srcInfo, CoreDataT& cdata);


public Q_SLOTS:
  void processError(const QNetworkReply::NetworkError& code) { m_evlHandler->exit(code);}

Q_SIGNALS:
  void propagateError(QNetworkReply::NetworkError);

private :
  typedef QSet<int> DataPointTriggerIds;
  QString m_apiUri;
  QNetworkRequest* m_reqHandler;
  int m_trid;
  static RequestListT requestsPatterns();
  QEventLoop* m_evlHandler;
  bool m_isLogged;
  QString m_auth;
  QSslConfiguration m_sslConfig;
  QString m_lastError;
  JsonHelper m_replyJsonData;

  bool checkLogin(const SourceT& srcInfo);
  int processLoginReply(void);
  int fecthApiVersion(const SourceT& srcInfo);
  int processGetApiVersionReply(void);
  int processTriggerReply(ChecksT& checks);
  int processItServiceReply(CoreDataT& cdata, DataPointTriggerIds& triggerIds);
  int setDataPoints(NodeListT& cnodes, const QString& sourceId, const DataPointTriggerIds& dataPointTriggerIds);
  void setSslReplyErrorHandlingOptions(QNetworkReply* reply);
  std::string parseHostGroups(const QScriptValue& json);
  std::string parseHost(const QScriptValue& json);

};

#endif /* ZABBIXHELPER_HPP_ */
