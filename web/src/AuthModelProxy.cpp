/*
# LdapAuthModel.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 25-07-2014                                                  #
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

#include "WebAuthSettings.hpp"
#include "dbo/src/UserManagement.hpp"
#include "AuthModelProxy.hpp"
#include "WebUtils.hpp"
#include "Validators.hpp"
#include "LdapHelper.hpp"
#include <QObject>
#include <QDebug>

AuthModelProxy::AuthModelProxy(DbSession* dbSessionRef)
  : Wt::Auth::AuthModel(dbSessionRef->auth(), dbSessionRef->users()),
    m_dbSessionRef(dbSessionRef)
{
}


bool AuthModelProxy::login(Wt::Auth::Login& login)
{
  auto username = valueText(Wt::Auth::FormBaseModel::LoginNameField).toUTF8();
  auto password = valueText(Wt::Auth::AuthModel::PasswordField).toUTF8();

  auto user = m_dbSessionRef->findAuthUser(username, password);
  if (! user.isValid()) {
    login.login(user, Wt::Auth::LoginState::Disabled);
    return false;
  }

  WebAuthSettings config;
  if (username == "admin" || config.getAuthenticationMode() == WebBaseSettings::BuiltIn) {
    CORE_LOG("info", QObject::tr("Built-in authentication succeeded with user %1 ").arg(QString::fromStdString(username)).toStdString());
    login.login(user, Wt::Auth::LoginState::Strong);
    return true;
  }

  // handle LDAP auth
  std::string ldapServerUri = config.getLdapServerUri();
  std::string ldapIdField = config.getLdapIdField();
  std::string ldapSearchBase = config.getLdapSearchBase();
  std::string ldapBindUserDn =config.getLdapBindUserDn();
  std::string ldapBindPassword =  config.getLdapBindUserPassword();
  int ldapVersion = config.getLdapVersion();

  LdapHelper ldapHelper(ldapServerUri, ldapVersion);
  QString ldapFilter = QString("(&(%1=%2)(userPassword=%3))").arg(ldapIdField.c_str(),
                                                                  QString::fromStdString(username),
                                                                  QString::fromStdString(password));
  LdapUserMapT ldapUsers;
  int count = ldapHelper.listUsers(ldapSearchBase, ldapBindUserDn, ldapBindPassword, ldapFilter.toStdString().c_str(), ldapUsers);
  if (count == 1) {
    login.login(user, Wt::Auth::LoginState::Strong);
    CORE_LOG("info", QObject::tr("LDAP authentication succeeded with user %1 ").arg(QString::fromStdString(username)).toStdString());
    return true;
  }
  login.login(user, Wt::Auth::LoginState::Disabled);
  m_loginFailed.emit("LDAP authentication failed");
  CORE_LOG("error", QString("LDAP authentication failed with user %1").arg(QString::fromStdString(username)).toStdString());
  return false;
}

