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

typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class DbSession : public dbo::Session
{
public:
  DbSession(bool initializeDb = false);
  ~DbSession();
  void setup(bool initializeDb);

  Wt::Auth::AbstractUserDatabase& users() const {return *m_dbUsers;}
  static Wt::Auth::AuthService& auth();
  static Wt::Auth::PasswordService& passwordAuthentificator(void);
  static void configureAuth(void);
  const User& loggedUser(void)const {return m_loggedUser;}
  void setLoggedUser(const std::string& uid);
  UserListT& getUserList(void) {return m_userList;}
  void updateUserList(void);
  int addUser(User user);
  int updateUser(User user);
  int updatePassword(const std::string& login,
                     const std::string& currentPass,
                     const std::string& newPass);
  int deleteUser(std::string username);

private:
  dbo::backend::Sqlite3* m_sqlite3Db;
  UserDatabase* m_dbUsers;
  User m_loggedUser;
  UserListT m_userList;

  std::string hashPassword(const std::string& pass);
  void initDb(void);
};

#endif // DBSESSION_HPP
