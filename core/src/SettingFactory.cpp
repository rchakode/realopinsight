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
#include "SettingFactory.hpp"
#include "BaseSettings.hpp"
#include "utilsCore.hpp"
#include "JsonHelper.hpp"
#include <QtScript/QScriptEngine>

const QString SettingFactory::GLOBAL_LANGUAGE_KEY = "/General/language";
const QString SettingFactory::GLOBAL_DB_STATE_KEY = "/General/DbState";
const QString SettingFactory::GLOBAL_GRAPH_LAYOUT = "/General/graphLayout";
const QString SettingFactory::GLOBAL_SRC_BUCKET_KEY = "/Sources/buckets";
const QString SettingFactory::GLOBAL_UPDATE_INTERVAL_KEY = "/Monitor/updateInterval";

const QString SettingFactory::DB_TYPE = "/Database/dbType";
const QString SettingFactory::DB_SERVER_ADDR = "/Database/dbServerAddr";
const QString SettingFactory::DB_SERVER_PORT = "/Database/dbServerPort";
const QString SettingFactory::DB_NAME = "/Database/dbName";
const QString SettingFactory::DB_USER = "/Database/dbUser";
const QString SettingFactory::DB_PASSWORD = "/Database/dbPassword";


const QString SettingFactory::AUTH_ADM_UNSERNAME_KEY = "/Auth/admUser";
const QString SettingFactory::AUTH_OP_UNSERNAME_KEY = "/Auth/opUsername";
const QString SettingFactory::AUTH_ADM_PASSWD_KEY = "/Auth/admPasswd";
const QString SettingFactory::AUTH_OP_PASSWD_KEY = "/Auth/opPasswd";
const QString SettingFactory::AUTH_MODE_KEY = "/Auth/authMode";
const QString SettingFactory::AUTH_LDAP_SERVER_URI = "/Auth/ldapServerUri";
const QString SettingFactory::AUTH_LDAP_BIND_USER_DN = "/Auth/ldapBindUserDn";
const QString SettingFactory::AUTH_LDAP_BIND_USER_PASSWORD = "/Auth/ldapBindUserPassword";
const QString SettingFactory::AUTH_LDAP_SEARCH_BASE = "/Auth/ldapUserSearchBase";
const QString SettingFactory::AUTH_LDAP_VERSION = "/Auth/ldapVersion";
const QString SettingFactory::AUTH_LDAP_ID_FIELD = "/Auth/ldapIdField";
const QString SettingFactory::AUTH_LDAP_SSL_USE_CERT = "/Auth/ldapSslUseCert";
const QString SettingFactory::AUTH_LDAP_SSL_CERT_FILE = "/Auth/ldapSslCertFile";
const QString SettingFactory::AUTH_LDAP_SSL_CA_FILE = "/Auth/ldapSslCaFile";

const QString SettingFactory::NOTIF_TYPE = "/Notification/notificationType";
const QString SettingFactory::NOTIF_MAIL_SMTP_SERVER_ADRR = "/Notification/mailSmtpServer";
const QString SettingFactory::NOTIF_MAIL_SMTP_SERVER_PORT = "/Notification/mailSmtpPort";
const QString SettingFactory::NOTIF_MAIL_SMTP_USE_SSL = "/Notification/mailSmtpUseSsl";
const QString SettingFactory::NOTIF_MAIL_SMTP_USERNAME = "/Notification/mailSmtpUsername";
const QString SettingFactory::NOTIF_MAIL_SMTP_PASSWORD = "/Notification/mailSmtpPassword";

const QString SettingFactory::DASHBOARD_THUMBNAILS_PER_ROW = "/Dashboard/thumbnailsPerRow";


SettingFactory::SettingFactory(): QSettings(COMPANY.toLower(), APP_NAME.toLower().replace(" ", "-"))
{
  init();
}


SettingFactory::SettingFactory(const QString& path): QSettings(path, QSettings::NativeFormat)
{
  init();
}


SettingFactory::~SettingFactory(void)
{
}

void SettingFactory::init(void)
{
  QString updateInterval = QSettings::value(SettingFactory::GLOBAL_UPDATE_INTERVAL_KEY).toString();
  QString admUser = QSettings::value(SettingFactory::AUTH_ADM_UNSERNAME_KEY).toString();
  QString admPasswd = QSettings::value(SettingFactory::AUTH_ADM_PASSWD_KEY).toString();
  QString opUser = QSettings::value(SettingFactory::AUTH_OP_UNSERNAME_KEY).toString();
  QString opPasswd = QSettings::value(SettingFactory::AUTH_OP_PASSWD_KEY).toString();

  if (updateInterval.isEmpty()) {
    QSettings::setValue(SettingFactory::GLOBAL_UPDATE_INTERVAL_KEY, QString::number(ngrt4n::DefaultUpdateInterval));
  }
  if (admUser.isEmpty()) {
    QSettings::setValue(SettingFactory::AUTH_ADM_UNSERNAME_KEY, ngrt4n::AdmUser.c_str());
  }
  if (admPasswd.isEmpty()) {
    QString passwd = QCryptographicHash::hash(ngrt4n::AdmUser.c_str(), QCryptographicHash::Md5) ;
    QSettings::setValue(SettingFactory::AUTH_ADM_PASSWD_KEY, passwd);
  }
  if (opUser.isEmpty()) {
    QSettings::setValue(SettingFactory::AUTH_OP_UNSERNAME_KEY, ngrt4n::OpUser.c_str());
  }
  if (opPasswd.isEmpty()) {
    QString passwd = QCryptographicHash::hash(ngrt4n::OpUser.c_str(), QCryptographicHash::Md5) ;
    QSettings::setValue(SettingFactory::AUTH_OP_PASSWD_KEY, passwd);
  }

  sync();
}


int SettingFactory::getGraphLayout(void) const
{
  return entry(SettingFactory::GLOBAL_GRAPH_LAYOUT).toInt();
}

void SettingFactory::setKeyValue(const QString & key, const QString & value)
{
  setValue(key, value.trimmed()) ;
  sync() ;
}


qint32 SettingFactory::updateInterval() const
{
  qint32 interval = QSettings::value(GLOBAL_UPDATE_INTERVAL_KEY).toInt();
  return (interval > 0)? interval : ngrt4n::DefaultUpdateInterval;
}

void SettingFactory::setEntry(const QString& key, const QString& value)
{
  QSettings::setValue(key, value);
}

bool SettingFactory::loadSource(qint32 in_sourceIndex, SourceT& out_sinfo)
{
  auto sourceKey = ngrt4n::sourceKey(in_sourceIndex);
  return bindSourceInfo(QSettings::value(sourceKey).toString(), out_sinfo);
}

bool SettingFactory::loadSource(const QString& in_sourceId, SourceT& out_sinfo)
{
  auto sourceKey = ngrt4n::sourceKey(in_sourceId);
  return bindSourceInfo(QSettings::value(sourceKey).toString(), out_sinfo);
}


bool SettingFactory::bindSourceInfo(const QString& sourceData, SourceT& sinfo)
{
  if (sourceData.isEmpty()) {
    return false;
  }

  JsonHelper jsHelper(sourceData);
  sinfo.id = jsHelper.getProperty("sid").toString();
  sinfo.mon_type = static_cast<qint8>(jsHelper.getProperty("mon_type").toInt32());
  sinfo.mon_url = jsHelper.getProperty("mon_url").toString();
  sinfo.auth = jsHelper.getProperty("auth").toString();
  sinfo.ls_addr = jsHelper.getProperty("ls_addr").toString();
  sinfo.ls_port = jsHelper.getProperty("ls_port").toInt32();
  sinfo.verify_ssl_peer = static_cast<qint8>(jsHelper.getProperty("verify_ssl_peer").toInt32());

  return true;
}

QString SettingFactory::language(void)
{
  QString lang = SettingFactory().entry(GLOBAL_LANGUAGE_KEY);
  return lang.isEmpty()? QLocale::system().name() : lang;
}

