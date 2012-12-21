/*
 * ZnsHelper.cpp
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

#include "ZnsHelper.hpp"
#include <QtScript/QScriptEngine>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkCookieJar>

const QString API_CONTEXT = "/zport/dmd";
const QString API_LOGIN_CONTEXT = "/zport/acl_users/cookieAuthHelper/login";

ZnsHelper::ZnsHelper(const QString & baseUrl)
    : QNetworkAccessManager(),
      apiBaseUrl(baseUrl),
      requestHandler(new QNetworkRequest())
{
    requestHandler->setUrl(QUrl(apiBaseUrl+API_CONTEXT));
    setRequestsPatterns();
}

ZnsHelper::~ZnsHelper()
{
    delete requestHandler;
}

QString ZnsHelper::getApiBaseUrl(void) const
{
    return apiBaseUrl;
}

QString ZnsHelper::getApiContextUrl(void) const
{
    return apiBaseUrl+API_CONTEXT;
}

void ZnsHelper::setBaseUrl(const QString & url)
{
    apiBaseUrl = url;
    requestHandler->setUrl(QUrl(apiBaseUrl+API_LOGIN_CONTEXT));
}


void ZnsHelper::setRequestUrl(const QUrl & url)
{
    requestHandler->setUrl(url);
}

QString ZnsHelper::getRequestUrl(void) const
{
    return requestHandler->url().toString();
}

void ZnsHelper::performPostRequest(const qint32 & reqId, const QByteArray & params)
{
    requestHandler->setRawHeader("Content-Type", contentTypes[reqId]);
    QNetworkReply* reply = this->post(*requestHandler, params);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));
}

void ZnsHelper::performGetRequest(const qint32 & reqId)
{
    requestHandler->setRawHeader("Content-Type", contentTypes[reqId]);
    QNetworkReply* reply = this->get(*requestHandler);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));
}

void ZnsHelper::processError(QNetworkReply::NetworkError code)
{
    emit propagateError(code);
}

void ZnsHelper::setRequestsPatterns()
{
    contentTypes[LOGIN_REQUEST] = "application/x-www-form-urlencoded";
    contentTypes[EVENT_REQUEST] = "application/json; charset=utf-8";
}
