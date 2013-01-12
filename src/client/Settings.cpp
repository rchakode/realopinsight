/*
 * Base.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 24-05-2012                                                 #
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


#include "Base.hpp"
#include "Auth.hpp"
#include "Settings.hpp"
#include "Preferences.hpp"
#include "core/MonitorBroker.hpp"
#include "core/ns.hpp"
#include <QtScript/QScriptEngine>

Settings::Settings(): QSettings(Project.toLower(), AppName.toLower().replace(" ", "-"))
{
  Q_INIT_RESOURCE(ngrt4n);
  QString updateInterval = value(Preferences::UPDATE_INTERVAL_KEY).toString();
  QString monitorHome = value(Preferences::URL_KEY).toString() ;
  QString admUser = value(Preferences::ADM_UNSERNAME_KEY).toString();
  QString admPasswd = value(Preferences::ADM_PASSWD_KEY).toString();
  QString opUser = value(Preferences::OP_UNSERNAME_KEY).toString();
  QString opPasswd = value(Preferences::OP_PASSWD_KEY).toString();
  QString serverAddr = value(Preferences::SERVER_ADDR_KEY).toString();
  QString serverPort = value(Preferences::SERVER_PORT_KEY).toString();

  if (updateInterval.isEmpty()) {
      setValue(Preferences::UPDATE_INTERVAL_KEY, QString::number(MonitorBroker::DefaultUpdateInterval));
    }
  if ( serverAddr.isEmpty() ) {
      setValue(Preferences::SERVER_ADDR_KEY, "localhost");
    }
  if ( serverPort.isEmpty() )	{
      setValue(Preferences::SERVER_PORT_KEY, QString::number(MonitorBroker::DefaultPort));
    }
  if ( monitorHome.isEmpty() ) {
      setValue(Preferences::URL_KEY, "http://realopinsight.com/en/index.php?page=contribute");
    }
  if ( admUser.isEmpty() ) {
      setValue(Preferences::ADM_UNSERNAME_KEY, Auth::AdmUser);
    }
  if ( admPasswd.isEmpty() ) {
      QString passwd = QCryptographicHash::hash(Auth::AdmUser.toAscii(), QCryptographicHash::Md5) ;
      setValue(Preferences::ADM_PASSWD_KEY, passwd );
    }
  if ( opUser.isEmpty() ) {
      setValue(Preferences::OP_UNSERNAME_KEY, Auth::OpUser);
    }
  if ( opPasswd.isEmpty() ) {
      QString passwd = QCryptographicHash::hash(Auth::OpUser.toAscii(), QCryptographicHash::Md5) ;
      setValue(Preferences::OP_PASSWD_KEY, passwd);
    }
  translator = new QTranslator();
  translator->load("ngrt4n_la");
  qApp->installTranslator(translator);
  sync();
}

void Settings::setKeyValue(const QString & _key, const QString & _value)
{
  setValue(_key, _value ) ;
  sync() ;
}
