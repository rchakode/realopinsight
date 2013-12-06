/*
 * DbSession.cpp
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
#include "DbSession.hpp"
#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/HashFunction>


DbSession::DbSession():
  m_sqlite3Db(new Wt::Dbo::backend::Sqlite3("/tmp/realopinsight.db")),
  m_dbsession(new dbo::Session()),
  m_basicAuthService(new Wt::Auth::AuthService()),
  m_authService(new Wt::Auth::PasswordService(*m_basicAuthService)),
  m_users(new UserDatabase(*m_dbsession))
{
  m_sqlite3Db->setProperty("show-queries", "true");
  m_dbsession->setConnection(*m_sqlite3Db);

  setup();

  Wt::Auth::PasswordVerifier *verifier = new Wt::Auth::PasswordVerifier();
  verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
  m_authService->setVerifier(verifier);
  m_authService->setAttemptThrottlingEnabled(true);
  //FIXME: m_authService->setStrengthValidator(new Wt::Auth::PasswordStrengthValidator());
  Wt::Auth::User user("0", *m_users);
  user.setIdentity();
  std::cout << m_authService->verifyPassword(user, "ngrt4n_adm") <<"VERIFFIIIIIIIIIIIII\n";
}

DbSession::~DbSession()
{
  delete m_sqlite3Db;
  delete m_users;
  delete m_basicAuthService;
  delete m_authService;
  delete m_dbsession;
}

void DbSession::setup(void)
{
  m_dbsession->mapClass<User>("user");
  m_dbsession->mapClass<AuthInfo>("auth_info");
  m_dbsession->mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  m_dbsession->mapClass<AuthInfo::AuthTokenType>("auth_token");

  try {
    m_dbsession->createTables();
  } catch (...) { }
  addUser("ngrt4n_adm", "ngrt4n_adm", Auth::AdmUserRole);
  addUser("ngrt4n_op", "ngrt4n_op", Auth::OpUserRole);
}

void DbSession::addUser(const std::string& username, const std::string& pass, int role)
{
  try {
    dbo::Transaction transaction(*m_dbsession);
    User *user = new User();
    user->username = username;
    user->password = pass;
    user->role =  role;
    dbo::ptr<User> userPtr = m_dbsession->add(user);
    transaction.commit();
  } catch (...) {
    //FIXME: handle error
  }
}