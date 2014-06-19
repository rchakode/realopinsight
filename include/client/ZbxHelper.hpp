/*
 * ZABBIXHelper.hpp
 # ------------------------------------------------------------------------ #
 # Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
 # Last Update : 4 août 2012                                                #
 #                                                                          #
 # This file is part of NGRT4N (http://ngrt4n.com).                         #
 #                                                                          #
 # NGRT4N is free software: you can redistribute it and/or modify           #
 # it under the terms of the GNU General Public License as published by     #
 # the Free Software Foundation, either version 3 of the License, or        #
 # (at your option) any later version.                                      #
 #                                                                          #
 # NGRT4N is distributed in the hope that it will be useful,                #
 # but WITHOUT ANY WARRANTY; without even the implied warranty of           #
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
 # GNU General Public License for more details.                             #
 #                                                                          #
 # You should have received a copy of the GNU General Public License        #
 # along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
 #--------------------------------------------------------------------------#
 */

#ifndef ZABBIXHELPER_HPP_
#define ZABBIXHELPER_HPP_
#include "Base.hpp"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>



const QString ZBX_API_CONTEXT = "/api_jsonrpc.php";

class ZbxHelper : public QNetworkAccessManager {
  Q_OBJECT

public:
  enum {
    Login=1,
    ApiVersion=2,
    Trigger=3,
    TriggerV18=4,
    Logout=5
  };

public:
  ZbxHelper(const QString& baseUrl="http://localhost/zabbix");
  virtual ~ZbxHelper();
  void postRequest(const qint32& reqId, const QStringList& params);
  void setBaseUrl(const QString& url) {apiUri = url%ZBX_API_CONTEXT; mrequestHandler->setUrl(QUrl(apiUri));}
  inline QString getApiUri(void) const {return apiUri;}
  inline void updateTrid(const QString& apiv) {mtrid = (apiv.startsWith("1"))? TriggerV18 : Trigger;}
  inline int getTrid(void) const {return mtrid;}
  void setSslConf(bool verifyPeer);

public slots:
  inline void processError(const QNetworkReply::NetworkError& code) {if(code <200 && code >=599) emit propagateError(code);}

signals:
  void propagateError(QNetworkReply::NetworkError);

private :
  QString apiUri;
  QNetworkRequest* mrequestHandler;
  RequestListT mrequestsPatterns;
  int mtrid;
  QSslConfiguration* sslConf;
  void setRequestsPatterns();
};

#endif /* ZABBIXHELPER_HPP_ */
