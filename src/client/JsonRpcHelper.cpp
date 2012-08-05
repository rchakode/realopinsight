/*
 * JsonRpcHelper.cpp
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

#include "JsonRpcHelper.hpp"
#include <QtScript/QScriptEngine>
#include <QDebug>
#include <QMessageBox>

JsonRpcHelper::JsonRpcHelper()
    : QNetworkAccessManager() {
    QUrl url("http://192.168.135.3/zabbix/api_jsonrpc.php");

    url.addQueryItem("jsonrpc", "2.0");
    url.addQueryItem("method", "user.authenticate");
//    url.addQueryItem("params", "[<arg_1>, <arg_2>]");
    url.addQueryItem("params", "{\"user\":admin, \"password\":zabbix}");
    url.addQueryItem("auth", "null");
    url.addQueryItem("id", "0");


    QNetworkRequest request;
    qDebug()<<url.toString();
    request.setUrl(url);
    currentReply = this->get(request);
    connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(onResult(QNetworkReply*)));
}

JsonRpcHelper::~JsonRpcHelper() {}


void JsonRpcHelper::onResult(QNetworkReply* reply)
{
    if (currentReply->error() != QNetworkReply::NoError)
        return;  // ...only in a blog post

    QString data = (QString) reply->readAll();

    qDebug() << data << endl;
    QScriptEngine engine;
    QScriptValue result = engine.evaluate(data);
    qDebug()<< "authChain " << result.property("result").toString() << endl ;

    /*
      Google YouTube JSON looks like this :

      {
        "version": "1.0",
        "encoding": "UTF-8",
        "feed": {
          ..
          ..
          "entry": [{
            "title": {
                "$t": "Nickelback- When We Stand Together"
            },
            "content": {
                "type": "application/x-shockwave-flash",
                "src": "http://www.youtube.com/v/76vdvdll0Y?version=3&f=standard&app=youtube_gdata"
            },
            "yt$statistics": {
                "favoriteCount": "29182",
                "viewCount": "41513706"
            },
            ...
            ...
          },
          ...
          ...
          ]
        }
      }
     */

        // Now parse this JSON according to your needs !
//    QScriptValue entries = result.property("result");
//    QScriptValueIterator it(entries);
//    while (it.hasNext()) {
//        it.next();
//        QScriptValue entry = it.value();

//        QString hostid = entry.property("hostid").toString();
//        QString proxyHostid = entry.property("proxy_hostid").toString();
//        QString host = entry.property("proxy_hostid").toString();
//    }
}
