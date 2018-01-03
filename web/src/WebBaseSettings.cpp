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


int WebBaseSettings::activeSourceIds(QVector<std::string>& result)
{
  result.clear();
  for (int i = 0; i < MAX_SRCS; ++i) {
    if (m_sourceStates.at(i)) {
      result.push_back(ngrt4n::sourceId(i).toStdString());
    }
  }
  return result.size();
}
