/*
 * WebPreferencesBase.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)        #
# Creation: 11-04-2015                                                     #
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


#include "WebPreferencesBase.hpp"
#include <ldap.h>

WebPreferencesBase::WebPreferencesBase(void)
  : Preferences("/opt/realopinsight/etc/realopinsight.conf")
{
}


int WebPreferencesBase::getLdapVersion(void) const
{
  std::string val = m_settings->keyValue(Settings::AUTH_LDAP_VERSION).toStdString();
  if (val != LDAP_VERSION3_LABEL)
    return LDAP_VERSION2;

  return LDAP_VERSION3;
}


int WebPreferencesBase::getAuthenticationMode(void) const
{
  int val = m_settings->keyValue(Settings::AUTH_MODE_KEY).toInt();
  if (val != LDAP)
    return BuiltIn;

  return val;
}



std::string WebPreferencesBase::getLdapIdField(void) const
{
  QString val = m_settings->keyValue(Settings::AUTH_LDAP_ID_FIELD);
  if (val.isEmpty())
    return "uid";

  return val.toStdString();
}
