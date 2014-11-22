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

#ifndef PANDORAHELPER_HPP_
#define PANDORAHELPER_HPP_
#include "Base.hpp"
#include "JsonHelper.hpp"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QSslConfiguration>


namespace {
  const QString PANDORA_API_CONTEXT = "/pandora_console/include/api.php";
  }

class PandoraHelper : public QNetworkAccessManager {
  Q_OBJECT
public:
  enum {
    LoginTest=1,
    GetModuleInfo=2,
    GetTreeAgents = 3
  };
  static const RequestListT ReqPatterns;

public:
  PandoraHelper(const QString& baseUrl="http://localhost/");
  virtual ~PandoraHelper();
  int
  loadChecks(const SourceT& srcInfo,  ChecksT& checks, const QString& agentName);
  QNetworkReply*
  postRequest(const qint32& reqId, const QStringList& params);
  void
  setBaseUrl(const QString& url);
  QString
  getPandoraVersion(void) const {return m_pandoraVersion;}
  void
  setIsLogged(bool state) {m_isLogged = state;}
  bool
  getIsLogged(void) const {return m_isLogged;}
  QString
  lastError(void) const {return m_lastError;}
  QString
  getApiEndpoint(void) const {return m_apiUri;}
  void
  setSslPeerVerification(bool verifyPeer);
  int
  processReply(QNetworkReply* reply, QString& data);
  int
  openSession(const SourceT& srcInfo);
  int
  processModuleReply(QNetworkReply* reply, ChecksT& checks);


public Q_SLOTS:
  void processError(const QNetworkReply::NetworkError& code) { m_evlHandler->exit(code);}

Q_SIGNALS:
  void propagateError(QNetworkReply::NetworkError);

private :
  QString m_apiUri;
  QNetworkRequest* m_reqHandler;
  QString m_pandoraVersion;
  static RequestListT requestsPatterns();
  QEventLoop* m_evlHandler;
  bool m_isLogged;
  QString m_pandoraUsername;
  QString m_pandoraPassword;
  QString m_pandoraApiPass;
  QSslConfiguration m_sslConfig;
  QString m_lastError;
  QString m_replyData;

  void setSslReplyErrorHandlingOptions(QNetworkReply* reply);
  std::string parseHostGroups(const QScriptValue& json);
  std::string parseHost(const QScriptValue& json);
  int parseLoginTestResult(const QString& data);
  int checkCredentialsInfo(const QString& authString);
};

#endif /* PANDORAHELPER_HPP_ */
