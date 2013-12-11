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
#include <Wt/Auth/PasswordStrengthValidator>

namespace {
  Wt::Auth::AuthService basicAuthService;
  Wt::Auth::PasswordService passAuthService(basicAuthService);
}

DbSession::DbSession():
  m_sqlite3Db(new Wt::Dbo::backend::Sqlite3("/tmp/realopinsight.db")),
  m_dbUsers(new UserDatabase(*this))
{
  m_sqlite3Db->setProperty("show-queries", "true");
  setConnection(*m_sqlite3Db);
  setup();
}

DbSession::~DbSession()
{
  delete m_sqlite3Db;
  delete m_dbUsers;
}

void DbSession::setup(void)
{
  mapClass<User>("user");
  mapClass<AuthInfo>("auth_info");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");

  try {
    createTables();
    addUser("ngrt4n_adm", "ngrt4n_adm", Auth::AdmUserRole);
    addUser("ngrt4n_op", "ngrt4n_op", Auth::OpUserRole);
    Wt::log("notice")<<"[realopinsight][dbo] "<< "Created database";
  } catch (std::exception& ex) {
    Wt::log("notice")<<"[realopinsight] "<< "Using existing database";
    Wt::log("notice")<<"[realopinsight][dbo] "<< ex.what();
  }
}

void DbSession::addUser(const std::string& username, const std::string& pass, int role)
{
  dbo::Transaction transaction(*this);
  try {
    Wt::Auth::User dbuser = m_dbUsers->registerNew();
    dbo::ptr<AuthInfo> info = m_dbUsers->find(dbuser);
    User u;
    u.username = username;
    u.role = role;
    info.modify()->setUser(add(&u));
    passAuthService.updatePassword(dbuser, pass);
    dbuser.addIdentity(Wt::Auth::Identity::LoginName, username);
  } catch (const std::exception& ex) {
    Wt::log("[realopinsight] error") << ex.what();
  }
  transaction.commit();
}

std::string DbSession::hashPassword(const std::string& pass)
{
  Wt::Auth::BCryptHashFunction h;
  return h.compute(pass, "salt");
}

Wt::Auth::AuthService& DbSession::auth()
{
  return basicAuthService;
}

Wt::Auth::PasswordService& DbSession::passwordAuthentificator(void)
{
  return passAuthService;
}

void DbSession::configureAuth(void)
{
  basicAuthService.setAuthTokensEnabled(true, "realopinsightcookie");
  basicAuthService.setEmailVerificationEnabled(true);
  Wt::Auth::PasswordVerifier* verifier = new Wt::Auth::PasswordVerifier();
  verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
  passAuthService.setVerifier(verifier);
  passAuthService.setStrengthValidator(new Wt::Auth::PasswordStrengthValidator());
  passAuthService.setAttemptThrottlingEnabled(true);
}

void DbSession::setLoggedUser(const std::string& uid)
{
  dbo::Transaction transaction(*this);
  dbo::ptr<AuthInfo> info = find<AuthInfo>().where("id="+uid);
  m_loggedUser = *(info.modify()->user());
  transaction.commit();
}
