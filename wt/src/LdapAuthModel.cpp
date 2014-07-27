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

#include "LdapAuthModel.hpp"
#include "UserManagement.hpp"
#include "WebUtils.hpp"
#include "Validators.hpp"
#include "LdapHelper.hpp"
#include <QObject>
#include <QDebug>

LdapAuthModel::LdapAuthModel(const Wt::Auth::AuthService& baseAuth,
                             Wt::Auth::AbstractUserDatabase& users,
                             Wt::WObject* parent)
  : Wt::Auth::AuthModel(baseAuth, users, parent)
{
}


bool LdapAuthModel::validate()
{
  if (valueText(Wt::Auth::FormBaseModel::LoginNameField).toUTF8() != "admin")
    return true;

  return Wt::Auth::AuthModel::validate();
}

bool LdapAuthModel::login(Wt::Auth::Login& login)
{
  std::string username = valueText(Wt::Auth::FormBaseModel::LoginNameField).toUTF8();
  std::string password = valueText(Wt::Auth::AuthModel::PasswordField).toUTF8();

  if (username == "admin") {
    return Wt::Auth::AuthModel::login(login);
  }

  WebPreferences preferences;
  LdapHelper ldapHelper(preferences.getLdapServerUri(), preferences.getLdapDnFormat(), preferences.getLdapVersion());

  qDebug() << "Login through LDAP"<< preferences.getLdapServerUri();
  LdapUsersT ldapUsers;
  if (ldapHelper.listUsers( preferences.getLdapSearchBase().toStdString(),
                           preferences.getLdapBindUserDn().toStdString(),
                           preferences.getLdapBindUserPassword().toStdString(),
                           ldapUsers) == 0) {
    qDebug() << "list users succeed: "<< ldapUsers.size();
  } else {
    qDebug() << "list users succeed: "<< ldapHelper.lastError();
  }

  if (! ldapHelper.loginWithUsername(username, password))
    return false;

  Wt::Auth::User user;
  login.login(user);
  std::cout << std::boolalpha<< user.isValid()<<"\n";
  qDebug() << m_lastError;
  return true;
}

void LdapAuthModel::logout(Wt::Auth::Login& login)
{
  login.logout();
  //This also removes the remember-me cookie for the user.
}

Wt::Auth::EmailTokenResult LdapAuthModel::processEmailToken(const std::string& token)
{
  return Wt::Auth::AuthModel::processEmailToken(token);
}

Wt::Auth::User LdapAuthModel::processAuthToken()
{
  return Wt::Auth::AuthModel::processAuthToken();
}
