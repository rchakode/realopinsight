/*
# LdapAuthModel.hpp
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

#ifndef LDAPAUTHMODEL_HPP
#define LDAPAUTHMODEL_HPP

#include "WebPreferences.hpp"
#include <Wt/Auth/AuthModel>
#include <Wt/Auth/Login>
#include <QString>
#include <string>


class LdapAuthModel : public Wt::Auth::AuthModel
{
public:
  LdapAuthModel(WebPreferences* preferences,
                const Wt::Auth::AuthService& baseAuth,
                Wt::Auth::AbstractUserDatabase& users,
                Wt::WObject* parent=0);
  virtual bool validate();
  virtual bool login(Wt::Auth::Login& login);
  virtual void logout(Wt::Auth::Login& login);
  virtual Wt::Auth::EmailTokenResult processEmailToken(const std::string& token);
  virtual Wt::Auth::User processAuthToken();

private:
  QString m_lastError;
  WebPreferences* m_preferences;

};

#endif // LDAPAUTHMODEL_HPP
