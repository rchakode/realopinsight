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


void DbSession::addUser(User user, const std::string& password)
{
  dbo::Transaction transaction(*this);
  try {
    Wt::Auth::User dbuser = m_dbUsers->registerNew();
    dbo::ptr<AuthInfo> info = m_dbUsers->find(dbuser);
    info.modify()->setUser(add(&user));
    info->setEmail(user.email); //FIXME: take this into account
    passAuthService.updatePassword(dbuser, password);
    dbuser.addIdentity(Wt::Auth::Identity::LoginName, user.username);
    flush();
  } catch (const std::exception& ex) {
    Wt::log("error")<<"[realopinsight]" << ex.what();
  }
  transaction.commit();
  updateUserList();
}

void DbSession::updateUser(User user)
{
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<User> usr = find<User>().where("name='"+user.username+"'");
    usr.modify()->username = user.username;
    usr.modify()->lastname = user.lastname;
    usr.modify()->firstname = user.firstname;
    usr.modify()->email = user.email;
    usr.modify()->role = user.role;
    //authinfo->setEmail(user.email); //FIXME: take this into account
  } catch (const std::exception& ex) {
    Wt::log("error")<<"[realopinsight]" << ex.what();
  }
  transaction.commit();
  updateUserList();
}

void DbSession::deleteUser(std::string username)
{
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<User> usr = find<User>().where("name='"+username+"'");
    usr.remove();
  } catch (const std::exception& ex) {
    Wt::log("error")<<"[realopinsight]" << ex.what();
  }
  transaction.commit();
  updateUserList();
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
    adm.firstname = "Default";
    adm.lastname = "Administrator";
    adm.role = User::AdmRole;
    adm.registrationDate = Wt::WDateTime::currentDateTime().toString().toUTF8();
    addUser(adm, "ngrt4n_adm");
    Wt::log("notice")<<"[realopinsight][dbo] "<< "Created database";
  } catch (std::exception& ex) {
    Wt::log("error")<<"[realopinsight] "<< "Failed initializing the database";
    Wt::log("error")<<"[realopinsight][dbo] "<< ex.what();
  }
}
