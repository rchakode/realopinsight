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

typedef Wt::Auth::Dbo::AuthInfo<RoiDboUser> AuthInfo;
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
  const RoiDboUser& loggedUser(void)const {return m_loggedUser;}
  void setLoggedUser(void);

  int addUser(const RoiDboUser& user);
  int updateUser(RoiDboUser user);
  int deleteUser(std::string uname);
  int updatePassword(const std::string& uname,
                     const std::string& currentPass,
                     const std::string& newPass);
  void updateUserList(void);
  RoiDboUsersT& userList(void) {return m_userList;}

  int addView(const RoiDboView& view);
  int deleteView(std::string vname);
  int assignView(const std::string& uname, const std::string& vname);
  int revokeView(const std::string& uname, const std::string& vname);

  void updateViewList(void);
  void updateViewList(const std::string& uname);
  RoiDboViewsT& viewList(void) {return m_viewList;}
  bool findView(const std::string& vname, RoiDboView& view);

  void updateUserViewList(void);
  RoiUserViewsT& userViewList(void) {return m_userViewList;}

  int addSession(const RoiDboLoginSession& session);
  int checkUserCookie(const RoiDboLoginSession& session);

private:
  std::string m_dbPath;
  dbo::backend::Sqlite3* m_sqlite3Db;
  UserDatabase* m_dbUsers;
  RoiDboUser m_loggedUser;
  RoiDboUsersT m_userList;
  RoiDboViewsT m_viewList;
  RoiUserViewsT m_userViewList;
  std::string m_lastError;
  Wt::Auth::Login m_loginObj;
  Wt::Auth::AuthService m_basicAuthService;
  Wt::Auth::PasswordService* m_passAuthService;

  std::string hashPassword(const std::string& pass);
  void initDb(void);
};

#endif // DBSESSION_HPP
