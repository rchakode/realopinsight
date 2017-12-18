/*
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
#include "SettingsHandler.hpp"
#include "BaseSettings.hpp"
#include "utilsCore.hpp"
#include "JsonHelper.hpp"
#include <QtScript/QScriptEngine>

const QString SettingsHandler::GLOBAL_LANGUAGE_KEY = "/General/language";
const QString SettingsHandler::GLOBAL_DB_STATE_KEY = "/General/DbState";
const QString SettingsHandler::GLOBAL_SRC_BUCKET_KEY = "/Sources/buckets";
const QString SettingsHandler::GLOBAL_UPDATE_INTERVAL_KEY = "/Monitor/updateInterval";


const QString SettingsHandler::DB_TYPE = "/Database/dbType";
const QString SettingsHandler::DB_SERVER_ADDR = "/Database/dbServerAddr";
const QString SettingsHandler::DB_SERVER_PORT = "/Database/dbServerPort";
const QString SettingsHandler::DB_NAME = "/Database/dbName";
const QString SettingsHandler::DB_USER = "/Database/dbUser";
const QString SettingsHandler::DB_PASSWORD = "/Database/dbPassword";


const QString SettingsHandler::AUTH_ADM_UNSERNAME_KEY = "/Auth/admUser";
const QString SettingsHandler::AUTH_OP_UNSERNAME_KEY = "/Auth/opUsername";
const QString SettingsHandler::AUTH_ADM_PASSWD_KEY = "/Auth/admPasswd";
const QString SettingsHandler::AUTH_OP_PASSWD_KEY = "/Auth/opPasswd";
const QString SettingsHandler::AUTH_MODE_KEY = "/Auth/authMode";
const QString SettingsHandler::AUTH_LDAP_SERVER_URI = "/Auth/ldapServerUri";
const QString SettingsHandler::AUTH_LDAP_BIND_USER_DN = "/Auth/ldapBindUserDn";
const QString SettingsHandler::AUTH_LDAP_BIND_USER_PASSWORD = "/Auth/ldapBindUserPassword";
const QString SettingsHandler::AUTH_LDAP_SEARCH_BASE = "/Auth/ldapUserSearchBase";
const QString SettingsHandler::AUTH_LDAP_VERSION = "/Auth/ldapVersion";
const QString SettingsHandler::AUTH_LDAP_ID_FIELD = "/Auth/ldapIdField";
const QString SettingsHandler::AUTH_LDAP_SSL_USE_CERT = "/Auth/ldapSslUseCert";
const QString SettingsHandler::AUTH_LDAP_SSL_CERT_FILE = "/Auth/ldapSslCertFile";
const QString SettingsHandler::AUTH_LDAP_SSL_CA_FILE = "/Auth/ldapSslCaFile";

const QString SettingsHandler::NOTIF_TYPE = "/Notification/notificationType";
const QString SettingsHandler::NOTIF_MAIL_SMTP_SERVER_ADRR = "/Notification/mailSmtpServer";
const QString SettingsHandler::NOTIF_MAIL_SMTP_SERVER_PORT = "/Notification/mailSmtpPort";
const QString SettingsHandler::NOTIF_MAIL_SMTP_USE_SSL = "/Notification/mailSmtpUseSsl";
const QString SettingsHandler::NOTIF_MAIL_SMTP_USERNAME = "/Notification/mailSmtpUsername";
const QString SettingsHandler::NOTIF_MAIL_SMTP_PASSWORD = "/Notification/mailSmtpPassword";

const QString SettingsHandler::DASHBOARD_THUMBNAILS_PER_ROW = "/Dashboard/thumbnailsPerRow";


SettingsHandler::SettingsHandler(): QSettings(COMPANY.toLower(), APP_NAME.toLower().replace(" ", "-"))
{
  init();
}


SettingsHandler::SettingsHandler(const QString& path): QSettings(path, QSettings::NativeFormat)
{
  init();
}


SettingsHandler::~SettingsHandler(void)
{
}

void SettingsHandler::init(void)
{
  QString updateInterval = QSettings::value(SettingsHandler::GLOBAL_UPDATE_INTERVAL_KEY).toString();
  QString admUser = QSettings::value(SettingsHandler::AUTH_ADM_UNSERNAME_KEY).toString();
  QString admPasswd = QSettings::value(SettingsHandler::AUTH_ADM_PASSWD_KEY).toString();
  QString opUser = QSettings::value(SettingsHandler::AUTH_OP_UNSERNAME_KEY).toString();
  QString opPasswd = QSettings::value(SettingsHandler::AUTH_OP_PASSWD_KEY).toString();

  if (updateInterval.isEmpty()) {
    QSettings::setValue(SettingsHandler::GLOBAL_UPDATE_INTERVAL_KEY, QString::number(ngrt4n::DefaultUpdateInterval));
  }
  if (admUser.isEmpty()) {
    QSettings::setValue(SettingsHandler::AUTH_ADM_UNSERNAME_KEY, ngrt4n::AdmUser.c_str());
  }
  if (admPasswd.isEmpty()) {
    QString passwd = QCryptographicHash::hash(ngrt4n::AdmUser.c_str(), QCryptographicHash::Md5) ;
    QSettings::setValue(SettingsHandler::AUTH_ADM_PASSWD_KEY, passwd);
  }
  if (opUser.isEmpty()) {
    QSettings::setValue(SettingsHandler::AUTH_OP_UNSERNAME_KEY, ngrt4n::OpUser.c_str());
  }
  if (opPasswd.isEmpty()) {
    QString passwd = QCryptographicHash::hash(ngrt4n::OpUser.c_str(), QCryptographicHash::Md5) ;
    QSettings::setValue(SettingsHandler::AUTH_OP_PASSWD_KEY, passwd);
  }

  sync();
}

void SettingsHandler::setKeyValue(const QString & _key, const QString & _value)
{
  setValue(_key, _value.trimmed()) ;
  sync() ;
}


qint32 SettingsHandler::updateInterval() const
{
  qint32 interval = QSettings::value(GLOBAL_UPDATE_INTERVAL_KEY).toInt();
  return (interval > 0)? interval : ngrt4n::DefaultUpdateInterval;
}

void SettingsHandler::setEntry(const QString& key, const QString& value)
{
  QSettings::setValue(key, value);
}

bool SettingsHandler::loadSource(qint32 _idx, SourceT& _src)
{
  return setSource(QSettings::value(ngrt4n::sourceKey(_idx)).toString(), _src);
}

bool SettingsHandler::loadSource(const QString& _id, SourceT& _src)
{
  return setSource(QSettings::value(ngrt4n::sourceKey(_id)).toString(), _src);
}


bool SettingsHandler::setSource(const QString& _info, SourceT& _src)
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

QString SettingsHandler::language(void)
{
  QString lang = SettingsHandler().entry(GLOBAL_LANGUAGE_KEY);
  return lang.isEmpty()? QLocale::system().name() : lang;
}

