/*
 * DbSession.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 06-12-2012                                                 #
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

#ifndef DBSESSION_HPP
#define DBSESSION_HPP

#include "User.hpp"
#include "Auth.hpp"
#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/User>
#include <Wt/WGlobal>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/Auth/Dbo/UserDatabase>

namespace Wt {
  namespace Auth {
    class Login;
  }
}

typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> WtDboUserDatabase;

class UserDatabase : public WtDboUserDatabase
{
public:
  UserDatabase(Wt::Dbo::Session &session)
    : WtDboUserDatabase(session),
      m_session(session)
  {
  }

  void setUser(const User& userData)
  {
    m_userData = userData;
  }

  virtual Wt::Auth::User registerNew(void)
  {
   // User* user = new User();
    m_user = m_session.add(&m_userData);
    AuthInfo* authInfo = new AuthInfo();
    authInfo->setUser(m_user);
    m_userAuthInfo = m_session.add(authInfo);
    m_user.flush();
    m_userAuthInfo.flush();
    return Wt::Auth::User(boost::lexical_cast<std::string>(m_userAuthInfo.id()), *this);
  }

private:
  dbo::Session& m_session;
  dbo::ptr<User> m_user;
  dbo::ptr<AuthInfo> m_userAuthInfo;
  User m_userData;
};

class DbSession : public dbo::Session
{
public:
  DbSession();
  ~DbSession();
  void setup(void);

  Wt::Auth::AuthService* auth() const {return m_basicAuthService;}
  Wt::Auth::AbstractUserDatabase* users() const {return m_users;}
  static Wt::Auth::Login login();
  Wt::Auth::PasswordService* passwordAuth(void) const {return m_passAuthService;}

private:
  dbo::backend::Sqlite3* m_sqlite3Db;
  Wt::Auth::AuthService* m_basicAuthService;
  Wt::Auth::PasswordService* m_passAuthService;
  UserDatabase* m_users;

  void addUser(const std::string& username, const std::string& pass, int role);
  std::string hashPassword(const std::string& pass);
};

#endif // DBSESSION_HPP
