/*
 * PandoraHelper.hpp
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

#ifndef ZABBIXHELPER_HPP_
#define ZABBIXHELPER_HPP_
#include "Base.hpp"
#include "JsonHelper.hpp"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QSslConfiguration>


namespace {
  const QString PANDORA_API_CONTEXT = "/include/api.php";
  }

class PandoraHelper : public QNetworkAccessManager {
  Q_OBJECT
public:
  enum {
    Login=1,
    ApiVersion=2,
    Module=3
  };
  static const RequestListT ReqPatterns;

public:
  PandoraHelper(const QString& baseUrl="http://localhost/zabbix");
  virtual ~PandoraHelper();
  QNetworkReply*
  postRequest(const qint32& reqId, const QStringList& params);
  void
  setBaseUrl(const QString& url) {m_apiUri = url%PANDORA_API_CONTEXT; m_reqHandler->setUrl(QUrl(m_apiUri));}
  void
  setTrid(const QString& apiv);
  int
  getTrid(void) const {return m_trid;}
  void
  setIsLogged(bool state) {m_isLogged = state;}
  bool
  getIsLogged(void) const {return m_isLogged;}
  void
  setAuth(const QString& auth) {m_auth = auth;}
  QString
  getAuth(void) const {return m_auth;}
  QString
  lastError(void) const {return m_lastError;}
  QString
  getApiEndpoint(void) const {return m_apiUri;}
  void
  setSslPeerVerification(bool verifyPeer);
  int
  parseReply(QNetworkReply* reply);
  bool
  checkRPCResultStatus(void);
  int
  openSession(const SourceT& srcInfo);
  int
  processLoginReply(QNetworkReply* reply);
  int
  fecthApiVersion(const SourceT& srcInfo);
  int
  processGetApiVersionReply(QNetworkReply* reply);
  int
  processModuleReply(QNetworkReply* reply, ChecksT& checks);
  int
  loadChecks(const SourceT& srcInfo,
             ChecksT& checks,
             const QString& filterValue,
             ngrt4n::RequestFilterT filterType = ngrt4n::HostFilter);


public Q_SLOTS:
  void processError(const QNetworkReply::NetworkError& code) { m_evlHandler->exit(code);}

Q_SIGNALS:
  void propagateError(QNetworkReply::NetworkError);

private :
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

  void setSslReplyErrorHandlingOptions(QNetworkReply* reply);
  std::string parseHostGroups(const QScriptValue& json);
  std::string parseHost(const QScriptValue& json);
};

#endif /* ZABBIXHELPER_HPP_ */
