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

ZabbixHelper::ZabbixHelper(const QString & server, const QString & protocol)
: QNetworkAccessManager(),
  uriPattern("%1://%2/zabbix/api_jsonrpc.php"),
  requestHandler(new QNetworkRequest()){

    this->server = server ;
    this->protocol = protocol ;
    requestHandler->setRawHeader("Content-Type", "application/json");
    requestHandler->setUrl(QUrl(uriPattern.arg(protocol).arg(server)));
    setRequestsPatterns();
}

ZabbixHelper::~ZabbixHelper() {
	delete requestHandler;
}

void ZabbixHelper::setServer(const QString &server) {
    this->server = server ;
    requestHandler->setUrl(QUrl(uriPattern.arg(protocol).arg(server)));
}

void ZabbixHelper::setProtocol(const QString &protocol) {
    this->protocol = protocol ;
    requestHandler->setUrl(QUrl(uriPattern.arg(protocol).arg(server)));
}

void ZabbixHelper::get(const qint32 & reqId, const QStringList & params) {
    QString request = requestsPatterns[reqId];
    foreach(const QString &param, params) {
        request=request.arg(param) ;
    }
    this->post(*requestHandler, request.toAscii());
}

void ZabbixHelper::setRequestsPatterns(){

    requestsPatterns[LOGIN] = "{\"jsonrpc\": \"2.0\", \
            \"method\": \"user.login\", \
            \"params\": {\"user\": \"%1\",\"password\": \"%2\"}, \
            \"auth\": null, \
            \"id\": 0}" ;

    requestsPatterns[TRIGGER] = "{\"jsonrpc\": \"2.0\", \
            \"method\": \"trigger.get\", \
            \"params\": { \
               \"filter\": { \
                   \"host\": [\"Zabbix server\"] \
               }, \
               \"selectHosts\": \"extend\", \
               \"selectItems\": \"extend\", \
               \"output\": \"extend\", \
               \"limit\": 1 \
             }, \
            \"auth\": \"%1\", \
             \"id\": 2 \
            }";
}
