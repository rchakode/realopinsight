/*
# ------------------------------------------------------------------------ #
# Copyright (c) 2015 Rodrigue Chakode (rodrigue.chakode@gmail.com)         #
# Last Change: 17-12-2017                                                  #
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


#include "WebBaseSettings.hpp"
#include "utilsCore.hpp"
#include <ldap.h>

WebBaseSettings::WebBaseSettings(void)
  : BaseSettings("/opt/realopinsight/etc/realopinsight.conf")
{
}


int WebBaseSettings::getLdapVersion(void) const
{
  std::string val = m_settingFactory->keyValue(SettingFactory::AUTH_LDAP_VERSION).toStdString();
  if (val != LDAP_VERSION3_LABEL)
    return LDAP_VERSION2;

  return LDAP_VERSION3;
}


int WebBaseSettings::getAuthenticationMode(void) const
{
  int val = m_settingFactory->keyValue(SettingFactory::AUTH_MODE_KEY).toInt();
  if (val != LDAP)
    return BuiltIn;

  return val;
}



std::string WebBaseSettings::getLdapIdField(void) const
{
  QString val = m_settingFactory->keyValue(SettingFactory::AUTH_LDAP_ID_FIELD);
  if (val.isEmpty())
    return "uid";

  return val.toStdString();
}


int WebBaseSettings::getDbType(void) const
{
  QString configValueStr = QString::fromLocal8Bit( qgetenv("REALOPINSIGHT_DB_TYPE") );
  if (! configValueStr.isEmpty()) {
    return configValueStr.toInt();
  }
  return m_settingFactory->keyValue(SettingFactory::DB_TYPE).toInt();
}

std::string WebBaseSettings::getDbServerAddr(void) const
{
  QString configValueStr = QString::fromLocal8Bit( qgetenv("REALOPINSIGHT_DB_SERVER_ADDR") );
  if (! configValueStr.isEmpty()) {
    return configValueStr.toStdString();
  }
  return m_settingFactory->keyValue(SettingFactory::DB_SERVER_ADDR).toStdString();
}


int WebBaseSettings::getDbServerPort(void) const
{
  QString configValueStr = QString::fromLocal8Bit( qgetenv("REALOPINSIGHT_DB_SERVER_PORT") );
  if (! configValueStr.isEmpty()) {
    return configValueStr.toInt();
  }
  return m_settingFactory->keyValue(SettingFactory::DB_SERVER_PORT).toInt();
}


std::string WebBaseSettings::getDbName(void) const
{
  QString configValueStr = QString::fromLocal8Bit( qgetenv("REALOPINSIGHT_DB_NAME") );
  if (! configValueStr.isEmpty()) {
    return configValueStr.toStdString();
  }
  return m_settingFactory->keyValue(SettingFactory::DB_NAME).toStdString();
}


std::string WebBaseSettings::getDbUser(void) const
{
  QString configValueStr = QString::fromLocal8Bit( qgetenv("REALOPINSIGHT_DB_USER") );
  if (! configValueStr.isEmpty()) {
    return configValueStr.toStdString();
  }
  return m_settingFactory->keyValue(SettingFactory::DB_USER).toStdString();
}


std::string WebBaseSettings::getDbPassword(void) const
{
  QString configValueStr = QString::fromLocal8Bit( qgetenv("REALOPINSIGHT_DB_PASSWORD") );
  if (! configValueStr.isEmpty()) {
    return configValueStr.toStdString();
  }
  return m_settingFactory->keyValue(SettingFactory::DB_PASSWORD).toStdString();
}

std::string WebBaseSettings::getDbConnectionString(void) const
{
  std::string connectionString = "";
  if (getDbType() == PostgresqlDb) {
    CORE_LOG("info", Q_TR("Using PostgreSQL database"));
    connectionString = Wt::WString("host={1} port={2} dbname={3} user={4} password={5}")
                       .arg(getDbServerAddr())
                       .arg(getDbServerPort())
                       .arg(getDbName())
                       .arg(getDbUser())
                       .arg(getDbPassword())
                       .toUTF8();
  } else { // use Sqlite3 as default database
    CORE_LOG("info", Q_TR("Using Sqlite3 database"));
    connectionString = ngrt4n::sqliteDbPath();
  }

  return connectionString;
}
