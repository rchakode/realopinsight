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



const QString SettingFactory::GLOBAL_LANGUAGE_KEY = "/General/language";
const QString SettingFactory::GLOBAL_DB_STATE_KEY = "/General/DbState";
const QString SettingFactory::GLOBAL_GRAPH_LAYOUT = "/General/graphLayout";
const QString SettingFactory::GLOBAL_UPDATE_INTERVAL_KEY = "/Monitor/updateInterval";
const QString SettingFactory::DB_TYPE = "/Database/dbType";
const QString SettingFactory::DB_SERVER_ADDR = "/Database/dbServerAddr";
const QString SettingFactory::DB_SERVER_PORT = "/Database/dbServerPort";
const QString SettingFactory::DB_NAME = "/Database/dbName";
const QString SettingFactory::DB_USER = "/Database/dbUser";
const QString SettingFactory::DB_PASSWORD = "/Database/dbPassword";
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


static const std::string BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";


SettingFactory::SettingFactory(void)
  : QSettings(SettingFactory::coreConfigPath(), QSettings::NativeFormat)
{
  m_configPath = QSettings::fileName();
  QString updateInterval = QSettings::value(SettingFactory::GLOBAL_UPDATE_INTERVAL_KEY).toString();
  if (updateInterval.isEmpty()) {
    QSettings::setValue(SettingFactory::GLOBAL_UPDATE_INTERVAL_KEY, QString::number(ngrt4n::DefaultUpdateInterval));
  }
  QSettings::sync();
}


SettingFactory::~SettingFactory(void)
{
}


int SettingFactory::getGraphLayout(void) const
{
  return entry(SettingFactory::GLOBAL_GRAPH_LAYOUT).toInt();
}

void SettingFactory::setKeyValue(const QString & key, const QString & value)
{
  QSettings::setValue(key, value.trimmed());
  QSettings::sync();
}


qint32 SettingFactory::updateInterval() const
{
  qint32 interval = QSettings::value(GLOBAL_UPDATE_INTERVAL_KEY).toInt();
  return (interval > 0)? interval : ngrt4n::DefaultUpdateInterval;
}

QString SettingFactory::language(void)
{
  QString lang = SettingFactory().entry(GLOBAL_LANGUAGE_KEY);
  return lang.isEmpty()? QLocale::system().name() : lang;
}

QString SettingFactory::coreAppDir(void)
{
  QString param = QString::fromLocal8Bit(qgetenv("REALOPINSIGHT_ROOT_DIR"));
  if (! param.isEmpty()) {
    return param;
  }
  return "/opt/realopinsight";
}

QString SettingFactory::coreDataDir(void)
{
  QString param = QString::fromLocal8Bit(qgetenv("REALOPINSIGHT_DATA_DIR"));
  if (! param.isEmpty()) {
    return param;
  }
  return QString("%1/data").arg(coreAppDir());
}


QString SettingFactory::coreLogDir(void)
{
  return QString("%1/log").arg(coreAppDir());
}

QString SettingFactory::coreConfigPath(void)
{
  return QString("%1/etc/realopinsight.conf").arg(coreAppDir());
}

std::string SettingFactory::webConfigPath(void)
{
  return QString("%1/etc/wt_config.xml").arg(SettingFactory::coreAppDir()).toStdString();
}

std::string SettingFactory::base64Encode(unsigned char const* bytesToEncode, unsigned int inLen)
{
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (inLen--) {
    char_array_3[i++] = *(bytesToEncode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++) {
        ret += BASE64_CHARS[char_array_4[i]];
      }
      i = 0;
    }
  }

  if (i) {
    for(j = i; j < 3; j++) {
      char_array_3[j] = '\0';
    }
    char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    for (j = 0; (j < i + 1); j++) {
      ret += BASE64_CHARS[char_array_4[j]];
    }
    while((i++ < 3)) {
      ret += '=';
    }
  }

  return ret;

}

std::string SettingFactory::base64Decode(std::string const& encoded_string)
{
  size_t in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && isBase64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++) {
        char_array_4[i] = BASE64_CHARS.find(char_array_4[i]) & 0xff;
      }

      char_array_3[0] = ( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];

      for (i = 0; (i < 3); i++) {
        ret += char_array_3[i];
      }
      i = 0;
    }
  }

  if (i) {
    for (j = 0; j < i; j++) {
      char_array_4[j] = BASE64_CHARS.find(char_array_4[j]) & 0xff;
    }

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

    for (j = 0; (j < i - 1); j++) {
      ret += char_array_3[j];
    }
  }

  return ret;
}

