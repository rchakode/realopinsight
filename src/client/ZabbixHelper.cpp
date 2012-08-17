/*
 * ZabbixHelper.cpp
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

#include "ZabbixHelper.hpp"
#include <QtScript/QScriptEngine>
#include <QDebug>
#include <QMessageBox>

const QString apiContext = "/api_jsonrpc.php";

ZabbixHelper::ZabbixHelper(const QString & baseUrl)
    : QNetworkAccessManager(),
      apiUri(baseUrl + apiContext),
      requestHandler(new QNetworkRequest()) {
    requestHandler->setRawHeader("Content-Type", "application/json");
    requestHandler->setUrl(QUrl(apiUri));
    setRequestsPatterns();
}

ZabbixHelper::~ZabbixHelper() {
    delete requestHandler;
}

void ZabbixHelper::setBaseUrl(const QString & url) {
    apiUri = url + apiContext ;
    requestHandler->setUrl(QUrl(apiUri));
}

QString ZabbixHelper::getApiUri(void) const {
    return apiUri ;
}

void ZabbixHelper::get(const qint32 & reqId, const QStringList & params) {
    QString request = requestsPatterns[reqId];
    foreach(const QString &param, params) {
        request = request.arg(param) ;
    }
    QNetworkReply* reply = this->post(*requestHandler, request.toAscii()); //Handle error
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));
}

void ZabbixHelper::processError(QNetworkReply::NetworkError code) {
    emit propagateError(code);
}

void ZabbixHelper::setRequestsPatterns(){

    requestsPatterns[LOGIN] = "{\"jsonrpc\": \"2.0\", \
            \"method\": \"user.login\", \
            \"params\": {\"user\": \"%1\",\"password\": \"%2\"}, \
            \"auth\": null, \
            \"id\": 0}" ;

    requestsPatterns[TRIGGER] = "{\"jsonrpc\": \"2.0\", \
            \"auth\": \"%1\", \
            \"method\": \"trigger.get\", \
            \"params\": { \
            \"filter\": { \"host\":[\"%2\"]}, \
            \"selectHosts\": [\"host\"], \
            \"selectItems\": [\"key_\",\"name\",\"lastclock\"], \
            \"output\": [\"description\",\"value\",\"error\",\"comments\"], \
            \"limit\": -1}, \
            \"id\": 1}";
}
