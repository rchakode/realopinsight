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
#include "UserMngtUI.hpp"
#include "WebUtils.hpp"
#include "Validators.hpp"
#include "LdapHelper.hpp"
#include <QObject>
#include <QDebug>

LdapAuthModel::LdapAuthModel(WebPreferences* preferences,
                             const Wt::Auth::AuthService& baseAuth,
                             Wt::Auth::AbstractUserDatabase& users,
                             Wt::WObject* parent)
  : Wt::Auth::AuthModel(baseAuth, users, parent),
    m_preferences(preferences)
{
}

//void LdapAuthModel::reset()
//{
//  Wt::Auth::AuthModel::reset();
//}
//bool LdapAuthModel::isVisible(Wt::WFormModel::Field field) const
//{
//  return Wt::Auth::AuthModel::isVisible(field);
//}

bool LdapAuthModel::validateField(Wt::WFormModel::Field field)
{
  //return true;
  return Wt::Auth::AuthModel::validateField(field);
}

//bool LdapAuthModel::validate()
//{
//  //  if (validateField(Wt::Auth::FormBaseModel::LoginNameField)
//  //      && validateField(Wt::Auth::AuthModel::PasswordField))
//  return true;

//  return false;
//}

//void LdapAuthModel::configureThrottling(Wt::WInteractWidget* button)
//{
//  Wt::Auth::AuthModel::configureThrottling(button);
//}

//void LdapAuthModel::updateThrottling(Wt::WInteractWidget* button)
//{
//  Wt::Auth::AuthModel::updateThrottling(button);
//}

bool LdapAuthModel::login(Wt::Auth::Login& login)
{
  if (! validate()) {
    qDebug() << "Validation failed";
    return false;
  }

  std::string username = valueText(Wt::Auth::FormBaseModel::LoginNameField).toUTF8();
  std::string password = valueText(Wt::Auth::AuthModel::PasswordField).toUTF8();

  if (username == "admin") {
    qDebug() << "Login through built-in database";
    return Wt::Auth::AuthModel::login(login);
  }

  LdapHelper ldapHelper(m_preferences->getLdapServerUri(), m_preferences->getLdapDnFormat(), m_preferences->getLdapVersion());

  qDebug() << "Login through LDAP"<< m_preferences->getLdapServerUri();
  LdapUsersT ldapUsers;
  if (ldapHelper.listUsers("ou=people,dc=realopinsight,dc=com", "Robert Smith", "rJsmitH", ldapUsers) == 0) {
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
