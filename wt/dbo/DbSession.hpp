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

#include "DbObjects.hpp"
#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/User>
#include <Wt/WGlobal>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/Auth/Dbo/UserDatabase>

typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class DbSession : public dbo::Session
{
public:
  DbSession(bool initializeDb = false);
  ~DbSession();
  void setup(bool initializeDb);
  std::string lastError(void) const {return m_lastError;}
  Wt::Auth::AbstractUserDatabase& users() const {return *m_dbUsers;}
  static Wt::Auth::AuthService& auth();
  static Wt::Auth::PasswordService& passwordAuthentificator(void);
  static void configureAuth(void);
  const User& loggedUser(void)const {return m_loggedUser;}
  void setLoggedUser(const std::string& uid);

  int addUser(const User& user);
  int updateUser(User user);
  int deleteUser(std::string uname);
  int updatePassword(const std::string& uname,
                     const std::string& currentPass,
                     const std::string& newPass);
  void updateUserList(void);
  UserListT& userList(void) {return m_userList;}

  int addView(const View& view);
  int deleteView(std::string vname);
  int assignView(const std::string& uname, const std::string& vname);
  int revokeView(const std::string& uname, const std::string& vname);
  void updateViewList(void);
  void updateViewList(const std::string& uname);
  ViewListT& viewList(void) {return m_viewList;}

  void updateUserViewList(void);
  UserViewListT& userViewList(void) {return m_userViewList;}

private:
  dbo::backend::Sqlite3* m_sqlite3Db;
  UserDatabase* m_dbUsers;
  User m_loggedUser;
  UserListT m_userList;
  ViewListT m_viewList;
  UserViewListT m_userViewList;
  std::string m_lastError;

  std::string hashPassword(const std::string& pass);
  void initDb(void);
};

#endif // DBSESSION_HPP
