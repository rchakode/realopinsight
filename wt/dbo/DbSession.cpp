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
#include <Wt/Auth/Identity>


DbSession::DbSession():
  m_sqlite3Db(new Wt::Dbo::backend::Sqlite3("/tmp/realopinsight.db")),
  m_basicAuthService(new Wt::Auth::AuthService()),
  m_passAuthService(new Wt::Auth::PasswordService(*m_basicAuthService))
{
  m_sqlite3Db->setProperty("show-queries", "true");
  setConnection(*m_sqlite3Db);

  setup();

  Wt::Auth::PasswordVerifier* verifier = new Wt::Auth::PasswordVerifier();
  verifier->addHashFunction(new Wt::Auth::BCryptHashFunction());
  m_passAuthService->setVerifier(verifier);
  m_passAuthService->setAttemptThrottlingEnabled(true);
  //FIXME: m_authService->setStrengthValidator(new Wt::Auth::PasswordStrengthValidator());
  //  try {
  //    Wt::Auth::User user("1", *m_users);
  //    std::cout << m_authService->verifyPassword(user, "ngrt4n_adm")<<"VERIFFIIIIIIIIIIIII\n";
  //  } catch (const std::exception& ex) {
  //    std::cout << ex.what()<< "==>>>>>>>>>>>>>>>>>>>>>>>\n";
  //  }
}

DbSession::~DbSession()
{
  delete m_sqlite3Db;
  delete m_users;
  delete m_basicAuthService;
  delete m_passAuthService;
}

void DbSession::setup(void)
{
  mapClass<User>("user");
  mapClass<AuthInfo>("auth_info");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");

  try {
    createTables();
    std::cerr << "Created database\n";
  } catch (std::exception& ex) {
    std::cerr << ex.what() << "\n";
    std::cerr << "Using existing database\n";
  }
  m_users = new UserDatabase(*this);
  addUser("ngrt4n_adm", "ngrt4n_adm", Auth::AdmUserRole);
  //addUser("ngrt4n_op", "ngrt4n_op", Auth::OpUserRole);
}

void DbSession::addUser(const std::string& username, const std::string& pass, int role)
{
  try {
    dbo::Transaction transaction(*this);
    User user;
    user.username = username;
    user.password = hashPassword(pass);
    user.role =  role;
    m_users->setUser(user);
    Wt::Auth::User auser = m_users->registerNew();
    auser.addIdentity(Wt::Auth::Identity::LoginName, username);
    m_passAuthService->updatePassword(auser, user.password);
    transaction.commit();
  } catch (const std::exception& ex) {
    Wt::log("[realopinsight] error") << ex.what();
  }
}

std::string DbSession::hashPassword(const std::string& pass)
{
  Wt::Auth::BCryptHashFunction h;
  return h.compute(pass, "salt");
}
