/*
 * Settings.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-03-2014                                                  #
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


#include "Base.hpp"
#include "Settings.hpp"
#include "Preferences.hpp"
#include "global.hpp"
#include "utilsCore.hpp"
#include "JsonHelper.hpp"
#include <QtScript/QScriptEngine>

const QString Settings::GLOBAL_SRC_BUCKET_KEY = "/Sources/buckets";
const QString Settings::GLOBAL_UPDATE_INTERVAL_KEY = "/Monitor/updateInterval";
const QString Settings::AUTH_ADM_UNSERNAME_KEY = "/Auth/admUser";
const QString Settings::AUTH_OP_UNSERNAME_KEY = "/Auth/opUsername";
const QString Settings::AUTH_ADM_PASSWD_KEY = "/Auth/admPasswd";
const QString Settings::AUTH_OP_PASSWD_KEY = "/Auth/opPasswd";
const QString Settings::AUTH_MODE_KEY = "/Auth/authMode";
const QString Settings::AUTH_LDAP_SERVER_URI = "/Auth/ldapServerUri";
const QString Settings::AUTH_LDAP_DN_FORMAT = "/Auth/ldapDnFormat";

Settings::Settings(): QSettings(COMPANY.toLower(), APP_NAME.toLower().replace(" ", "-"))
{
  init();
}


Settings::Settings(const QString& path): QSettings(path, QSettings::NativeFormat)
{
  init();
}


Settings::~Settings(void)
{
}

void Settings::init(void)
{
  QString updateInterval = QSettings::value(Settings::GLOBAL_UPDATE_INTERVAL_KEY).toString();
  QString admUser = QSettings::value(Settings::AUTH_ADM_UNSERNAME_KEY).toString();
  QString admPasswd = QSettings::value(Settings::AUTH_ADM_PASSWD_KEY).toString();
  QString opUser = QSettings::value(Settings::AUTH_OP_UNSERNAME_KEY).toString();
  QString opPasswd = QSettings::value(Settings::AUTH_OP_PASSWD_KEY).toString();

  if (updateInterval.isEmpty()) {
    QSettings::setValue(Settings::GLOBAL_UPDATE_INTERVAL_KEY, QString::number(ngrt4n::DefaultUpdateInterval));
  }
  if (admUser.isEmpty()) {
    QSettings::setValue(Settings::AUTH_ADM_UNSERNAME_KEY, ngrt4n::AdmUser.c_str());
  }
  if (admPasswd.isEmpty()) {
    QString passwd = QCryptographicHash::hash(ngrt4n::AdmUser.c_str(), QCryptographicHash::Md5) ;
    QSettings::setValue(Settings::AUTH_ADM_PASSWD_KEY, passwd);
  }
  if (opUser.isEmpty()) {
    QSettings::setValue(Settings::AUTH_OP_UNSERNAME_KEY, ngrt4n::OpUser.c_str());
  }
  if (opPasswd.isEmpty()) {
    QString passwd = QCryptographicHash::hash(ngrt4n::OpUser.c_str(), QCryptographicHash::Md5) ;
    QSettings::setValue(Settings::AUTH_OP_PASSWD_KEY, passwd);
  }
  sync();
}

void Settings::setKeyValue(const QString & _key, const QString & _value)
{
  setValue(_key, _value) ;
  sync() ;
}


qint32 Settings::updateInterval() const
{
  qint32 interval = QSettings::value(GLOBAL_UPDATE_INTERVAL_KEY).toInt();
  return (interval > 0)? interval : ngrt4n::DefaultUpdateInterval;
}

void Settings::setEntry(const QString& key, const QString& value)
{
  QSettings::setValue(key, value);
}

bool Settings::loadSource(qint32 _idx, SourceT& _src)
{
  return setSource(QSettings::value(ngrt4n::sourceKey(_idx)).toString(), _src);
}

bool Settings::loadSource(const QString& _id, SourceT& _src)
{
  return setSource(QSettings::value(ngrt4n::sourceKey(_id)).toString(), _src);
}


bool Settings::setSource(const QString& _info, SourceT& _src)
{
  if (_info.isEmpty()) {
    return false;
  }

  JsonHelper jsHelper(_info);
  _src.id = jsHelper.getProperty("sid").toString();
  _src.mon_type = jsHelper.getProperty("mon_type").toInt32();
  _src.mon_url = jsHelper.getProperty("mon_url").toString();
  _src.auth = jsHelper.getProperty("auth").toString();
  _src.use_ngrt4nd = jsHelper.getProperty("use_ngrt4nd").toInt32();
  _src.ls_addr = jsHelper.getProperty("ls_addr").toString();
  _src.ls_port = jsHelper.getProperty("ls_port").toInt32();
  _src.verify_ssl_peer = jsHelper.getProperty("verify_ssl_peer").toInt32();

  return true;
}
