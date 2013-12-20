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

DbSession::DbSession(bool initializeDb):
  m_sqlite3Db(new Wt::Dbo::backend::Sqlite3("/tmp/realopinsight.db")),
  m_dbUsers(new UserDatabase(*this))
{
  m_sqlite3Db->setProperty("show-queries", "true");
  setConnection(*m_sqlite3Db);
  setup(initializeDb);
}

DbSession::~DbSession()
{
  delete m_sqlite3Db;
  delete m_dbUsers;
}

void DbSession::setup(bool initializeDb)
{
  mapClass<User>("user");
  mapClass<AuthInfo>("auth_info");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");
  if (initializeDb) {
    initDb();
  }
}


int DbSession::addUser(User user)
{
  int retCode = -1;
  dbo::Transaction transaction(*this);
  try {
    Wt::Auth::User dbuser = m_dbUsers->registerNew();
    dbo::ptr<AuthInfo> info = m_dbUsers->find(dbuser);
    info.modify()->setUser(add(&user));
    info.modify()->setEmail(user.email); //FIXME: take this into account
    passAuthService.updatePassword(dbuser, user.password);
    dbuser.addIdentity(Wt::Auth::Identity::LoginName, user.username);
    flush();
    retCode = 0;
  } catch (const std::exception& ex) {
    retCode = 1;
    Wt::log("error")<<"[realopinsight]" << ex.what();
  }
  transaction.commit();
  updateUserList();
  return retCode;
}

int DbSession::updateUser(User user)
{
  int retCode = -1;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<AuthInfo> authInfo = find<AuthInfo>().where("user_name='"+user.username+"'");
    dbo::ptr<User> userPtr = authInfo.modify()->user();
    userPtr.modify()->username = user.username;
    userPtr.modify()->lastname = user.lastname;
    userPtr.modify()->firstname = user.firstname;
    userPtr.modify()->email = user.email;
    userPtr.modify()->role = user.role;
    authInfo.modify()->setEmail(user.email); //FIXME: take this into account
    retCode = 0;
  } catch (const std::exception& ex) {
    retCode = -1;
    Wt::log("error")<<"[realopinsight]" << ex.what();
  }
  transaction.commit();
  updateUserList();
  return retCode;
}

int DbSession::updatePassword(const std::string& login,
                              const std::string& currentPass,
                              const std::string& newpass)
{
  int retCode = -1;
  dbo::Transaction transaction(*this);
  try {
    Wt::Auth::User dbuser = m_dbUsers->findWithIdentity(Wt::Auth::Identity::LoginName, login);
    switch (passAuthService.verifyPassword(dbuser, currentPass)) {
      case Wt::Auth::PasswordValid:
        passAuthService.updatePassword(dbuser, newpass);
        retCode = 0;
        break;
      default:
        retCode = 1;
        break;
    }
  } catch (const std::exception& ex) {
    retCode = -1;
    Wt::log("error")<<"[realopinsight]" << ex.what();
  }
  transaction.commit();
  updateUserList();
  return retCode;
}

int DbSession::deleteUser(std::string login)
{
  int retCode = -1;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<User> usr = find<User>().where("name='"+login+"'");
    usr.remove();
    retCode = 0;
  } catch (const std::exception& ex) {
    retCode = 1;
    Wt::log("error")<<"[realopinsight]" << ex.what();
  }
  transaction.commit();
  updateUserList();
  return retCode;
}

std::string DbSession::hashPassword(const std::string& pass)
{
  Wt::Auth::BCryptHashFunction h;
  return h.compute(pass, "$ngrt4n$salt");
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

void DbSession::updateUserList(void)
{
  m_userList.clear();
  dbo::Transaction transaction(*this);
  typedef dbo::collection< dbo::ptr<User> > UserCollectionT;
  UserCollectionT users = find<User>();
  for (UserCollectionT::const_iterator it = users.begin(), end = users.end(); it != end; ++it) {
    m_userList.push_back(*(*it));
  }
  transaction.commit();
}

void DbSession::initDb(void)
{
  try {
    createTables();
    User adm;
    adm.username = "ngrt4n_adm";
    adm.password = "ngrt4n_adm";
    adm.firstname = "Default";
    adm.lastname = "Administrator";
    adm.role = User::AdmRole;
    adm.registrationDate = Wt::WDateTime::currentDateTime().toString().toUTF8();
    addUser(adm);
    Wt::log("notice")<<"[realopinsight][dbo] "<< "Created database";
  } catch (std::exception& ex) {
    Wt::log("error")<<"[realopinsight] "<< "Failed initializing the database";
    Wt::log("error")<<"[realopinsight][dbo] "<< ex.what();
  }
}
