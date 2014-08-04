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


class AuthModelProxy : public Wt::Auth::AuthModel
{
public:
  AuthModelProxy(const Wt::Auth::AuthService& baseAuth,
                Wt::Auth::AbstractUserDatabase& users,
                Wt::WObject* parent=0);
  virtual bool login(Wt::Auth::Login& login);

private:
  QString m_lastError;
};

#endif // LDAPAUTHMODEL_HPP
