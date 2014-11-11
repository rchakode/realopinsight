/*
# AuthManager.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
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

#ifndef AUTHWIDGET_HPP
#define AUTHWIDGET_HPP


#include <Wt/Auth/AuthWidget>
#include <Wt/WContainerWidget>
#include <Wt/Auth/Login>
#include "WebPreferences.hpp"

class DbSession;
class WebMainUI;

class AuthManager : public Wt::Auth::AuthWidget
{

public:
  AuthManager(DbSession* dbSession);
  DbSession* session(void) {return m_dbSession;}
  void logout(void);
  bool isLogged(void);
  WebPreferences* preferences(void) {return m_preferences;}

protected:
  virtual void createLoggedInView(void);
  virtual void createLoginView(void);

private:
  DbSession* m_dbSession;
  WebMainUI* m_mainUI;
  WebPreferences* m_preferences;
  Wt::WText* m_infoBox;

  void handleAuthentication(void);
  void handleLoginFailed(std::string data);
};

#endif // AUTHWIDGET_HPP
