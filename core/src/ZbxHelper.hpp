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
    GetTriggersByIds = 6,
    GetITServices = 5
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
  bool backendReturnedSuccessfulResult(void);
  int openSession(void);
  int loadChecks(const SourceT& srcInfo, ChecksT& checks, const QString& filterValue,
                 ngrt4n::RequestFilterT filterType = ngrt4n::HostFilter);
  int loadITServices(const SourceT& srcInfo, CoreDataT& cdata);


public Q_SLOTS:
  void processError(const QNetworkReply::NetworkError& code) { m_evlHandler->exit(code);}

Q_SIGNALS:
  void propagateError(QNetworkReply::NetworkError);

private :
  typedef QMap<QString, QSet<QString> > ZabbixParentChildsDependenciesMapT;
  typedef QMap<QString, QString> ZabbixChildParentDependenciesMapT;
  typedef QMap<QString, QString> ZabbixServiceTriggerDependenciesMapT;
  QString m_apiUri;
  QNetworkRequest* m_reqHandler;
  int m_trid;
  static RequestListT requestsPatterns();
  QEventLoop* m_evlHandler;
  bool m_isLogged;
  SourceT m_sourceInfo;
  QString m_auth;
  QSslConfiguration m_sslConfig;
  QString m_lastError;
  JsonHelper m_replyJsonData;

  bool checkLogin(void);
  int processLoginReply(void);
  int fecthApiVersion(void);
  int processGetApiVersionReply(void);
  int processTriggerData(ChecksT& checks);
  int processZabbixITServiceData(CoreDataT& cdata,
                               ZabbixParentChildsDependenciesMapT& parentChildsDependencies,
                               ZabbixChildParentDependenciesMapT& childParentDependencies,
                               ZabbixServiceTriggerDependenciesMapT& serviceTriggerDependencies);
  QString extractTopParentServices(const NodeListT& bpnodes, const ZabbixChildParentDependenciesMapT& childParentDependencies);
  int setBusinessServiceDependencies(NodeListT& bpnodes, const ZabbixParentChildsDependenciesMapT& parentChildsDependencies);
  int setITServiceDataPoint(NodeListT& cnodes, const ZabbixServiceTriggerDependenciesMapT& serviceTriggerDependencies);
  void setSslReplyErrorHandlingOptions(QNetworkReply* reply);
  std::string processHostGroupsJsonValue(const QScriptValue& hostGroupJsonValue);
  std::string processHostJsonValue(const QScriptValue& hostJsonValue);
  void processAppendDependenciesJsonValue(const QScriptValue& depsJsonValue,
                                          ZabbixParentChildsDependenciesMapT& parentChildsDependencies,
                                          ZabbixChildParentDependenciesMapT& childParentDependencies);
  ngrt4n::AggregatedSeverityT aggregationRuleFromZabbixCalcRule(int zabbixCalcRule);
  QString getTriggersIdsJsonList(const QSet<QString>& triggerIds);
  void setServicesParent(NodeListT& nodes, const ZabbixChildParentDependenciesMapT& childParentDependencies);

};

#endif /* ZABBIXHELPER_HPP_ */
