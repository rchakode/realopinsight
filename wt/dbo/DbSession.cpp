/*
 * DbSession.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 06-12-2013                                                 #
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
#include "WebUtils.hpp"
#include "DbSession.hpp"
#include <QFile>
#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/HashFunction>
#include <Wt/Auth/Identity>
#include <Wt/Auth/PasswordStrengthValidator>

namespace {
  Wt::Auth::AuthService basicAuthService;
  Wt::Auth::PasswordService passAuthService(basicAuthService);
}

DbSession::DbSession(void)
  : m_dbPath(ngrt4n::sqliteDbPath()),
    m_sqlite3Db(new Wt::Dbo::backend::Sqlite3(m_dbPath)),
    m_dbUsers(new UserDatabase(*this))
{
  m_sqlite3Db->setProperty("show-queries", "true");
  setConnection(*m_sqlite3Db);
  setupDb();
  updateUserList();
  updateViewList();
}

DbSession::~DbSession()
{
  delete m_sqlite3Db;
  delete m_dbUsers;
}

void DbSession::setupDb(void)
{
  mapClass<User>("user");
  mapClass<View>("view");
  mapClass<AuthInfo>("auth_info");
  mapClass<LoginSession>("login_session");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");
  initDb();
}


int DbSession::addUser(const User& user)
{
  int retCode = -1;
  dbo::Transaction transaction(*this);
  try {
    UserCollectionT users = find<User>().where("name=?").bind(user.username);
    if (users.size() > 0) {
      m_lastError = "Failed: a user with the same username already exist.";
      LOG("error", m_lastError);
      retCode = 1;
    } else {
      Wt::Auth::User dbuser = m_dbUsers->registerNew();
      dbo::ptr<AuthInfo> info = m_dbUsers->find(dbuser);
      info.modify()->setEmail(user.email);
      passAuthService.updatePassword(dbuser, user.password);
      User* userTmpPtr(new User());
      *userTmpPtr = user;
      info.modify()->setUser(add(userTmpPtr));
      dbuser.addIdentity(Wt::Auth::Identity::LoginName, user.username);
      retCode = 0;
    }
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to add the user. More details in log.";
    LOG("error", ex.what());
  }
  transaction.commit();
  updateUserList();
  return retCode;
}

int DbSession::updateUser(User user)
{
  int retCode = -1;
  try {
    dbo::Transaction transaction(*this);
    dbo::ptr<AuthInfo> authInfo = find<AuthInfo>().where("user_name=?").bind(user.username);
    dbo::ptr<User> userPtr = authInfo.modify()->user();
    userPtr.modify()->username = user.username;
    userPtr.modify()->lastname = user.lastname;
    userPtr.modify()->firstname = user.firstname;
    userPtr.modify()->email = user.email;
    userPtr.modify()->role = user.role;
    authInfo.modify()->setEmail(user.email);
    retCode = 0;
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to update the user. More details in log.";
    LOG("error", ex.what());
  }
  updateUserList();
  return retCode;
}

int DbSession::updatePassword(const std::string& uname,
                              const std::string& currentPass,
                              const std::string& newpass)
{
  int retCode = -1;
  try {
    dbo::Transaction transaction(*this);
    Wt::Auth::User dbuser = m_dbUsers->findWithIdentity(Wt::Auth::Identity::LoginName, uname);
    switch (passAuthService.verifyPassword(dbuser, currentPass)) {
      case Wt::Auth::PasswordValid:
        passAuthService.updatePassword(dbuser, newpass);
        retCode = 0;
        break;
      case Wt::Auth::PasswordInvalid:
        m_lastError = "Your current password doesn't match";
        break;
      case Wt::Auth::LoginThrottling:
        m_lastError = "The account has been blocked. Retry later or contact your administrator";
        break;
      default:m_lastError = "Unknown error concerning your current password";
        break;
    }
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    retCode = -1;
    LOG("error", ex.what());
  }
  updateUserList();
  return retCode;
}

int DbSession::deleteUser(std::string uname)
{
  int retCode = -1;
  try {
    dbo::Transaction transaction(*this);
    dbo::ptr<User> usr = find<User>().where("name=?").bind(uname);
    usr.remove();
    retCode = 0;
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    retCode = 1;
    LOG("error", ex.what());
  }
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

Wt::Auth::Login& DbSession::loginObject(void)
{
  return loginObj;
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
  try {
    dbo::Transaction transaction(*this);
    dbo::ptr<AuthInfo> info = find<AuthInfo>().where("id=?").bind(uid);
    m_loggedUser = *(info.modify()->user());
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    LOG("error", ex.what());
  }
}

void DbSession::updateUserList(void)
{
  try {
    m_userList.clear();
    dbo::Transaction transaction(*this);
    UserCollectionT users = find<User>();
    for (auto &user : users) {
      m_userList.push_back(*user);
    }
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    LOG("error", ex.what());
  }
}

void DbSession::updateViewList(void)
{
  try {
    m_viewList.clear();
    dbo::Transaction transaction(*this);
    ViewCollectionT views = find<View>();
    for (auto& view :views) {
      m_viewList.push_back(*view);
    }
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    LOG("error", ex.what());
  }
}

void DbSession::updateViewList(const std::string& uname)
{
  try {
    m_viewList.clear();
    dbo::Transaction transaction(*this);
    dbo::ptr<User> userDboPtr = find<User>().where("name=?").bind(uname);
    for (auto& view : userDboPtr.modify()->views) {
      m_viewList.push_back(*view);
    }
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    LOG("error", ex.what());
  }

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
    LOG("notice", "Database created: "+m_dbPath);
  } catch (dbo::Exception& ex) {
    LOG("error", "Failed initializing the database");
    LOG("error", ex.what());
  }
}

int DbSession::addView(const View& view)
{
  int retCode = -1;
  dbo::Transaction transaction(*this);
  try {
    ViewCollectionT views = find<View>().where("name=?").bind(view.name);
    if (views.size() > 0) {
      m_lastError = "Failed: a view with the same name already exist.";
      LOG("error", m_lastError);
      retCode = 1;
    } else {
      View* viewTmpPtr(new View());
      *viewTmpPtr =  view;
      add(viewTmpPtr);
      retCode = 0;
    }
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to add the view. More details in log.";
    LOG("error", ex.what());
  }
  transaction.commit();
  updateViewList();
  return retCode;
}


int DbSession::deleteView(std::string vname)
{
  int retCode = -1;
  try {
    dbo::Transaction transaction(*this);
    execute("DELETE FROM user_view WHERE view_name=?;").bind(vname);
    execute("DELETE FROM view WHERE name=?;").bind(vname);
    retCode = 0;
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    retCode = 1;
    m_lastError = ex.what();
    LOG("error", ex.what());
  }
  updateViewList();
  return retCode;
}


void DbSession::updateUserViewList(void)
{
  m_userViewList.clear();
  dbo::Transaction transaction(*this);
  UserCollectionT users = find<User>();
  for (auto& user : users) {
    for (const auto& view: user->views) {
      m_userViewList.insert(user->username+":"+view->name);
    }
  }
  transaction.commit();
}



int DbSession::assignView(const std::string& uname, const std::string& vname)
{
  int retCode = -1;
  try {
    dbo::Transaction transaction(*this);
    dbo::ptr<User> dboUserPtr = find<User>().where("name=?").bind(uname);
    dbo::ptr<View> dboViewPtr = find<View>().where("name=?").bind(vname);
    dboUserPtr.modify()->views.insert(dboViewPtr);
    retCode = 0;
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    LOG("error", ex.what());
  }
  return retCode;
}


int DbSession::revokeView(const std::string& uname, const std::string& vname)
{
  int retCode = -1;
  try {
    dbo::Transaction transaction(*this);
    dbo::ptr<User> dboUserPtr = find<User>().where("name=?").bind(uname);
    dbo::ptr<View> dboViewPtr = find<View>().where("name=?").bind(vname);
    dboUserPtr.modify()->views.erase(dboViewPtr);
    retCode = 0;
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    LOG("error", ex.what());
  }
  return retCode;
}


int DbSession::addSession(const LoginSession& session)
{
  int retCode = -1;
  dbo::Transaction transaction(*this);
  try {
    if (checkUserCookie(session) != LoginSession::ActiveCookie) {
      LoginSession* sessionPtr(new LoginSession());
      *sessionPtr = session;
      add(sessionPtr);
      retCode = 0;
    } else {
      m_lastError = "Already active session";
      LOG("error", m_lastError);
      retCode = 1;
    }
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to add the session. More details in log.";
    LOG("error", ex.what());
  }
  transaction.commit();
  return retCode;
}


int DbSession::checkUserCookie(const LoginSession& session)
{
  int retCode = -1;

  dbo::Transaction transaction(*this);
  try {
    LoginSessionCollectionT sessions = find<LoginSession>()
        .where("username=? AND session_id=? AND status = ?")
        .bind(session.username)
        .bind(session.sessionId)
        .bind(LoginSession::ExpiredCookie);
    retCode = sessions.size()? LoginSession::ActiveCookie : LoginSession::InvalidSession;
  } catch (const dbo::Exception& ex) {
    m_lastError = "Error checking the session. More details in log.";
    LOG("error", ex.what());
  }
  transaction.commit();

  return retCode;
}
