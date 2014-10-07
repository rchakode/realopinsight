/*
 * DbSession.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
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

#ifndef DBSESSION_HPP
#define DBSESSION_HPP

#include "DbObjects.hpp"
#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/User>
#include <Wt/WGlobal>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/Auth/Dbo/UserDatabase>
#include <Wt/Auth/Login>

typedef Wt::Auth::Dbo::AuthInfo<DbUserT> AuthInfo;
typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class DbSession : public dbo::Session
{
public:
  DbSession(void);
  ~DbSession();
  void setupDb(void);
  std::string lastError(void) const {return m_lastError;}
  Wt::Auth::AbstractUserDatabase& users() const {return *m_dbUsers;}
  Wt::Auth::AuthService& auth();
  Wt::Auth::PasswordService* passwordAuthentificator(void);
  Wt::Auth::Login& loginObject(void);
  bool isLogged(void) {return loginObject().loggedIn();}
  void configureAuth(void);
  const DbUserT& loggedUser(void)const {return m_loggedUser;}
  void setLoggedUser(void);

  int addUser(const DbUserT& user);
  int updateUser(DbUserT user);
  int deleteUser(std::string uname);
  int deleteAuthSystemUsers(int authSystem);
  bool findUser(const std::string& username, DbUserT& user);
  int updatePassword(const std::string& uname, const std::string& currentPass,const std::string& newPass);
  void updateUserList(void);
  DbUsersT& userList(void) {return m_userList;}

  int addView(const DbViewT& qos);
  int deleteView(std::string vname);
  int assignView(const std::string& uname, const std::string& vname);
  int revokeView(const std::string& uname, const std::string& vname);

  int addQosEntry(const DbQosEntryT& qos);

  void updateViewList(void);
  void updateViewList(const std::string& uname);
  DbViewsT& viewList(void) {return m_viewList;}
  bool findView(const std::string& vname, DbViewT& view);

  void updateUserViewList(void);
  UserViewsT& userViewList(void) {return m_userViewList;}

  int addSession(const DbLoginSession& session);
  int checkUserCookie(const DbLoginSession& session);

private:
  std::string m_dbPath;
  dbo::backend::Sqlite3* m_sqlite3Db;
  UserDatabase* m_dbUsers;
  DbUserT m_loggedUser;
  DbUsersT m_userList;
  DbViewsT m_viewList;
  UserViewsT m_userViewList;
  std::string m_lastError;
  Wt::Auth::Login m_loginObj;
  Wt::Auth::AuthService m_basicAuthService;
  Wt::Auth::PasswordService* m_passAuthService;

  std::string hashPassword(const std::string& pass);
  void initDb(void);
};

#endif // DBSESSION_HPP
