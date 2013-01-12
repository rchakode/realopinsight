/*
 * ZABBIXHelper.hpp
 # ------------------------------------------------------------------------ #
 # Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
 # Last Update : 6 Decembre 2012                                            #
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

#ifndef ZENOSSHELPER_HPP_
#define ZENOSSHELPER_HPP_
#include "Base.hpp"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

const QString ZNS_API_CONTEXT = "/zport/dmd";
const QString ZNS_LOGIN_API_CONTEXT = "/zport/acl_users/cookieAuthHelper/login";

class ZnsHelper : public QNetworkAccessManager {
  Q_OBJECT

public:
  enum RequesT{
    Login = 1,
    Component = 2,
    Device = 3
  };
  const static RequestListT ReQPatterns;
  const static RequestListT ContentTypes;
  const static RequestListT Routers;

  ZnsHelper(const QString & baseUrl="http://localhost:8080");
  virtual ~ZnsHelper();

  static RequestListT getContentTypes();
  static RequestListT getRequestsPatterns();
  static RequestListT getRouters();
  void setBaseUrl(const QString & url);
  void postRequest(const qint32 & reqId,  const QByteArray & data);
  void setRouter(const int & reqType);
  inline QString getRequestUrl(void) const {return mrequestHandler->url().toString();}
  inline void setRequestUrl(const QString & url) {mrequestHandler->setUrl(QUrl(url));}
  inline void setRequestUrl(const QUrl & url) {mrequestHandler->setUrl(url);}
  inline QString getApiContextUrl(void) const {return mapiBaseUrl+ZNS_API_CONTEXT;}
  inline QString getApiBaseUrl(void) const {return mapiBaseUrl;}
  inline static QString getDeviceName(const QString& uid) {return uid.mid(uid.lastIndexOf("/")+1, -1);}

public slots:
  inline void processError(const QNetworkReply::NetworkError& code) {if(code <200 && code >599) emit propagateError(code);}

signals:
  void propagateError(QNetworkReply::NetworkError);

private :
  QString mapiBaseUrl;
  QNetworkRequest* mrequestHandler;
};

#endif /* ZENOSSHELPER_HPP_ */
