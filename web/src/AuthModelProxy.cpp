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

#include "dbo/UserManagement.hpp"
#include "AuthModelProxy.hpp"
#include "WebUtils.hpp"
#include "Validators.hpp"
#include "LdapHelper.hpp"
#include <QObject>
#include <QDebug>

AuthModelProxy::AuthModelProxy(const Wt::Auth::AuthService& baseAuth,
                               Wt::Auth::AbstractUserDatabase& users,
                               Wt::WObject* parent)
  : Wt::Auth::AuthModel(baseAuth, users, parent),
    m_loginFailed(this)
{

}


bool AuthModelProxy::login(Wt::Auth::Login& login)
{
  WebPreferences preferences;

  // Check authentication mode
  if (preferences.getAuthenticationMode() == WebPreferences::BuiltIn)
    return Wt::Auth::AuthModel::login(login);

  QString username = QString::fromStdString(valueText(Wt::Auth::FormBaseModel::LoginNameField).toUTF8());
  QString password = QString::fromStdString(valueText(Wt::Auth::AuthModel::PasswordField).toUTF8());

  if (username == "admin") {
    return Wt::Auth::AuthModel::login(login);
  }

  // Get LDAP settings info
  std::string ldapServerUri = preferences.getLdapServerUri();
  std::string ldapIdField = preferences.getLdapIdField();
  std::string ldapSearchBase = preferences.getLdapSearchBase();
  std::string ldapBindUserDn =preferences.getLdapBindUserDn();
  std::string ldapBindPassword =  preferences.getLdapBindUserPassword();
  int ldapVersion = preferences.getLdapVersion();

  LdapHelper ldapHelper(ldapServerUri, ldapVersion);
  QString ldapFilter = QString("(&(%1=%2)(userPassword=%3))").arg(ldapIdField.c_str(),
                                                                  username,
                                                                  password);
  LdapUserMapT ldapUsers;
  int result = ldapHelper.listUsers(ldapSearchBase,
                                    ldapBindUserDn,
                                    ldapBindPassword,
                                    ldapFilter.toStdString().c_str(),
                                    ldapUsers);
  if (result != 1) {
    std::string erroMsg  = Q_TR("LDAP authentication failed");
    m_loginFailed.emit(erroMsg);
    LOG("error", QString("%1: %2").arg(erroMsg.c_str(), username).toStdString());
  } else {
    LOG("info", Q_TR("LDAP authentication succeeded: ")+username.toStdString());
    return Wt::Auth::AuthModel::login(login);
  }
  return false;
}

