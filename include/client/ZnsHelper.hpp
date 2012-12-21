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
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

class ZnsHelper : public QNetworkAccessManager {
    Q_OBJECT

public:
    enum {
        LOGIN_REQUEST=1,
        EVENT_REQUEST=2
    };

public:
    ZnsHelper(const QString & baseUrl="http://localhost/zenoss");
    virtual ~ZnsHelper();
    void setBaseUrl(const QString & url);
    QString getApiBaseUrl(void) const;
    QString getApiContextUrl(void) const;

    void setRequestUrl(const QUrl & url);
    QString getRequestUrl(void) const;
    void performPostRequest(const qint32 & reqId,  const QByteArray & params);
    void performGetRequest(const qint32 & reqId);

public slots:
    void processError(QNetworkReply::NetworkError code);

signals:
    void propagateError(QNetworkReply::NetworkError);

private :
    typedef QMap<qint32, QByteArray> RequestListT;
    QString apiBaseUrl;
    QNetworkRequest* requestHandler;
    RequestListT contentTypes;
    void setRequestsPatterns();
};

#endif /* ZENOSSHELPER_HPP_ */
