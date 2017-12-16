/*
# DbSession.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
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
#include "WebUtils.hpp"
#include "DbSession.hpp"
#include "WebPreferencesBase.hpp"
#include <tuple>
#include <QFile>
#include <Wt/Auth/HashFunction>
#include <Wt/Auth/Identity>
#include <Wt/Auth/PasswordStrengthValidator>
#include <Wt/Dbo/Exception>

namespace Wt {
  namespace Dbo {
    namespace backend {
      class Sqlite3Exception : public Exception
      {
        public:
          Sqlite3Exception(const std::string& msg)
            : Exception(msg) { }
      };
    }
  }
}

DbSession::DbSession(int dbType, const std::string& db)
{
  m_dboUserDb = new UserDatabase(*this);
  m_passAuthService = new Wt::Auth::PasswordService(m_basicAuthService);

  switch (dbType) {
  case PostgresqlDb:
    m_dboSqlConncetion = new Wt::Dbo::backend::Postgres(db);
    m_dboSqlConncetion->setProperty("show-queries", "false");
    setConnection(*m_dboSqlConncetion);
    break;

    // Sqlite3 is the default database
  case Sqlite3Db:
  default:
    m_dboSqlConncetion = new Wt::Dbo::backend::Sqlite3(db);
    m_dboSqlConncetion->setProperty("show-queries", "false");
    setConnection(*m_dboSqlConncetion);
    break;
  }

  // do this before doing anything to avoid authorized access
  configureAuth();

  setupDb();
  updateUserList();
  updateViewList();
}

DbSession::~DbSession()
{
  delete m_dboUserDb;
  delete m_dboSqlConncetion;
  delete m_passAuthService;
}

void DbSession::setupDb(void)
{
  mapClass<DboUser>("user");
  mapClass<DboView>("view");
  mapClass<AuthInfo>("auth_info");
  mapClass<DboLoginSession>("login_session");
  mapClass<DboQosData>("qosdata");
  mapClass<DboNotification>("notification");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");
  initDb();
}


int DbSession::addUser(const DboUserT& userInfo)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    DboUserCollectionT users = find<DboUser>().where("name=?").bind(userInfo.username);
    if (users.size() > 0) {
      m_lastError = "Failed: a user with the same username already exist.";
      CORE_LOG("error", m_lastError);
      retValue = 1;
    } else {
      Wt::Auth::User dbuser = m_dboUserDb->registerNew();
      dbo::ptr<AuthInfo> info = m_dboUserDb->find(dbuser);
      info.modify()->setEmail(userInfo.email);
      m_passAuthService->updatePassword(dbuser, userInfo.password);
      DboUser* userTmpPtr(new DboUser());
      userTmpPtr->setData(userInfo);
      info.modify()->setUser( add(userTmpPtr) );
      dbuser.addIdentity(Wt::Auth::Identity::LoginName, userInfo.username);
      retValue = 0;
    }
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to add the user.";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  updateUserList();
  return retValue;
}

int DbSession::updateUser(const DboUserT& userInfo)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<AuthInfo> authInfo = find<AuthInfo>().where("user_name=?").bind(userInfo.username);
    dbo::ptr<DboUser> userPtr = authInfo.modify()->user();
    userPtr.modify()->username = userInfo.username;
    userPtr.modify()->lastname = userInfo.lastname;
    userPtr.modify()->firstname = userInfo.firstname;
    userPtr.modify()->email = userInfo.email;
    userPtr.modify()->role = userInfo.role;
    userPtr.modify()->dashboardDisplayMode = userInfo.dashboardDisplayMode;
    userPtr.modify()->dashboardTilesPerRow = userInfo.dashboardTilesPerRow;
    authInfo.modify()->setEmail(userInfo.email);
    retValue = 0;
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to update the user.";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  updateUserList();
  return retValue;
}

int DbSession::updatePassword(const std::string& uname, const std::string& currentPass, const std::string& newpass)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    Wt::Auth::User dbuser = m_dboUserDb->findWithIdentity(Wt::Auth::Identity::LoginName, uname);
    switch (m_passAuthService->verifyPassword(dbuser, currentPass)) {
    case Wt::Auth::PasswordValid:
      m_passAuthService->updatePassword(dbuser, newpass);
      retValue = 0;
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
  } catch (const dbo::Exception& ex) {
    retValue = -1;
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  updateUserList();
  return retValue;
}


int DbSession::deleteUser(const std::string& username)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> usr = find<DboUser>().where("name=?").bind(username);
    usr.remove();
    retValue = 0;
  } catch (const dbo::Exception& ex) {
    retValue = 1;
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  updateUserList();
  return retValue;
}


int DbSession::deleteAuthSystemUsers(int authSystem)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> usr = find<DboUser>().where("authsystem=?").bind(authSystem);
    usr.remove();
    retValue = 0;
  } catch (const dbo::Exception& ex) {
    retValue = 1;
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  updateUserList();
  return retValue;
}


bool DbSession::findUser(const std::string& username, DboUserT& user)
{
  DbUsersT::const_iterator it = std::find_if(m_userList.cbegin(),
                                             m_userList.cend(),
                                             [&username](const DboUser& u){return u.username == username;});
  bool found = false;
  if (it != m_userList.end()) {
    found = true;
    user = it->data();
  }
  return found;
}


std::string DbSession::hashPassword(const std::string& pass)
{
  Wt::Auth::BCryptHashFunction h;
  return h.compute(pass, "$ngrt4n$salt");
}

Wt::Auth::AuthService& DbSession::auth()
{
  return m_basicAuthService;
}

Wt::Auth::PasswordService* DbSession::passwordAuthentificator(void)
{
  return m_passAuthService;
}

Wt::Auth::Login& DbSession::loginObject(void)
{
  rereadAll();
  return m_loginObj;
}

void DbSession::configureAuth(void)
{
  m_basicAuthService.setAuthTokensEnabled(true, "realopinsightcookie");
  m_basicAuthService.setEmailVerificationEnabled(true);
  Wt::Auth::PasswordVerifier* verifier = new Wt::Auth::PasswordVerifier();
  verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
  m_passAuthService->setVerifier(verifier);
  m_passAuthService->setStrengthValidator(new Wt::Auth::PasswordStrengthValidator());
  m_passAuthService->setAttemptThrottlingEnabled(true);
}

void DbSession::setLoggedUser(void)
{
  dbo::Transaction transaction(*this);
  try {
    std::string dbUserId = loginObject().user().id();
    dbo::ptr<AuthInfo> info = find<AuthInfo>().where("id=?").bind(dbUserId);
    m_loggedUser = *(info.modify()->user());
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
}

void DbSession::updateUserList(void)
{
  dbo::Transaction transaction(*this);
  try {
    m_userList.clear();
    DboUserCollectionT users = find<DboUser>();
    for (auto &user : users) {
      m_userList.push_back(*user);
    }
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
}

void DbSession::updateViewList(void)
{
  dbo::Transaction transaction(*this);
  try {
    m_viewList.clear();
    DboViewCollectionT views = find<DboView>();
    for (auto& view : views) {
      m_viewList.push_back(*view);
    }
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
}

void DbSession::updateViewList(const std::string& uname)
{
  dbo::Transaction transaction(*this);
  try {
    m_viewList.clear();
    dbo::ptr<DboUser> userDboPtr = find<DboUser>().where("name=?").bind(uname);
    for (auto& view : userDboPtr.modify()->views) {
      m_viewList.push_back(*view);
    }
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
}

bool DbSession::findView(const std::string& vname, DboView& view)
{
  DbViewsT::const_iterator it = std::find_if(m_viewList.cbegin(),
                                             m_viewList.cend(),
                                             [&vname](const DboView& v){return v.name == vname;});
  bool found = false;
  if (it != m_viewList.end()) {
    found = true;
    view = *it;
  }
  return found;
}

void DbSession::initDb(void)
{
  try {
    WebPreferencesBase pref;
    if (pref.dbInitializationState() != DbInitialized) {
      createTables();
      DboUserT adm;
      adm.username = "admin";
      adm.password = "password";
      adm.firstname = "Default";
      adm.lastname = "Administrator";
      adm.role = DboUser::AdmRole;
      adm.registrationDate = QDateTime::currentDateTime().toString().toStdString();;
      addUser(adm);
      pref.updateDbInitializationState(1);
      CORE_LOG("info", Q_TR("Database initialized"));
    }
  } catch (dbo::Exception& ex) {
    CORE_LOG("error", "Failed initializing the database");
    CORE_LOG("error", QObject::tr("%1: Failed initializing the database. %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
}

int DbSession::addView(const DboView& view)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    DboViewCollectionT views = find<DboView>().where("name=?").bind(view.name);
    if (views.size() > 0) {
      m_lastError = QObject::tr("Add view failed: a view named '%1' already exists").arg(view.name.c_str()).toStdString();
      CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, m_lastError.c_str()).toStdString());
      retValue = 1;
    } else {
      DboView* viewTmpPtr(new DboView());
      *viewTmpPtr =  view;
      add(viewTmpPtr);
      retValue = 0;
    }
  } catch (const dbo::Exception& ex) {
    m_lastError = "Add view failed, please check the log file";
    CORE_LOG("error", ex.what());
  } catch(const std::exception& ex) {
    m_lastError = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  updateViewList();
  return retValue;
}


int DbSession::deleteView(std::string viewId)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    execute("DELETE FROM view WHERE name = ?;").bind(viewId);
    retValue = 0;
  } catch (const Wt::Dbo::backend::Sqlite3Exception& ex) {
    m_lastError = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  } catch (const dbo::Exception& ex) {
    m_lastError = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  updateViewList();
  return retValue;
}


void DbSession::updateUserViewList(void)
{
  m_userViewList.clear();
  dbo::Transaction transaction(*this);
  DboUserCollectionT users = find<DboUser>();
  for (auto& user : users) {
    for (const auto& view: user->views) {
      m_userViewList.insert(user->username+":"+view->name);
    }
  }
  transaction.commit();
}



int DbSession::assignView(const std::string& userId, const std::string& vname)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> dboUserPtr = find<DboUser>().where("name=?").bind(userId);
    dbo::ptr<DboView> dboViewPtr = find<DboView>().where("name=?").bind(vname);
    dboUserPtr.modify()->views.insert(dboViewPtr);
    retValue = 0;
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  } catch(const std::exception& ex) {
    m_lastError = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::revokeView(const std::string& userId, const std::string& viewId)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> userPtr = find<DboUser>().where("name=?").bind(userId);
    dbo::ptr<DboView> viewPtr = find<DboView>().where("name=?").bind(viewId);
    userPtr.modify()->views.erase(viewPtr);
    retValue = 0;
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  } catch(const std::exception& ex) {
    m_lastError = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::listAssignedUsersEmails(QStringList& emails, const std::string& viewId)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    std::string sql = QString("SELECT user.email"
                              " FROM user, user_view"
                              " WHERE user.name = user_view.user_name"
                              "   AND user_view.view_name = '%1'"
                              "   AND user.email != ''"
                              ).arg(viewId.c_str()).toStdString();

    dbo::collection<std::string> results = query<std::string>(sql);
    emails.clear();
    for (const auto& entry : results) {
      emails.push_back(QString::fromStdString(entry));
    }
    retValue = emails.size();
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::addSession(const DboLoginSession& session)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    if (checkUserCookie(session) != DboLoginSession::ActiveCookie) {
      DboLoginSession* sessionPtr(new DboLoginSession());
      *sessionPtr = session;
      add(sessionPtr);
      retValue = 0;
    } else {
      m_lastError = "Already active session";
      CORE_LOG("error", m_lastError);
      retValue = 1;
    }
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to add the session, please check the log file";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::checkUserCookie(const DboLoginSession& session)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    DboLoginSessionCollectionT sessions = find<DboLoginSession>()
        .where("username=? AND session_id=? AND status = ?")
        .bind(session.username)
        .bind(session.sessionId)
        .bind(DboLoginSession::ExpiredCookie);
    retValue = sessions.size()? DboLoginSession::ActiveCookie : DboLoginSession::InvalidSession;
  } catch (const dbo::Exception& ex) {
    m_lastError = "Error checking the session, please check the log file";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::addQosData(const QosDataT& qosData)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    DboQosData* ptr_qosDboData = new DboQosData();
    ptr_qosDboData->setData(qosData);
    ptr_qosDboData->view = find<DboView>().where("name=?").bind(qosData.view_name);;
    dbo::ptr<DboQosData> dboEntry = add(ptr_qosDboData);
    retValue = 0;
    m_lastError = Q_TR("QoS entry added: ") + dboEntry->toString();
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to add QoS entry, please check the log file";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::addQosDataList(const QosDataList& qosDataList)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    m_lastError.clear();
    for (const auto& qosData : qosDataList) {
      DboQosData* ptr_qosDboData = new DboQosData();
      ptr_qosDboData->setData(qosData);
      ptr_qosDboData->view = find<DboView>().where("name=?").bind(qosData.view_name);;
      dbo::ptr<DboQosData> dboEntry = add(ptr_qosDboData);
      m_lastError.append(Q_TR("QoS entry added: ")).append(dboEntry->toString());
    }
    retValue = 0;
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to add QoS entries to database.";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::listQosData(QosDataListMapT& qosDataMap, const std::string& viewId, long fromDate, long toDate)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    DboQosDataCollectionT dbEntries;
    if (viewId.empty()) {
      dbEntries = find<DboQosData>()
          .where("timestamp >= ? AND timestamp <= ?")
          .orderBy("timestamp")
          .bind(fromDate).bind(toDate);
    } else {
      dbEntries = find<DboQosData>()
          .where("view_name = ? AND timestamp >= ? AND timestamp <= ?")
          .orderBy("timestamp")
          .bind(viewId).bind(fromDate).bind(toDate);
    }

    qosDataMap.clear();
    for (auto& entry : dbEntries) {
      qosDataMap[entry->view->name].push_back(entry->data());
    }

    retValue = 0;
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to fetch QoS entries, please check the log file";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::getLastQosData(QosDataT& qosData, const std::string& viewId)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    DboQosDataCollectionT queryResults;
    if (viewId.empty()) {
      queryResults = find<DboQosData>()
          .orderBy("timestamp DESC")
          .limit(1);
    } else {
      queryResults = find<DboQosData>()
          .orderBy("timestamp DESC")
          .limit(1);
    }

    if (queryResults.size() == 1) {
      retValue = 0;
      qosData =  queryResults.begin()->modify()->data();
    }
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to fetch last QoS entry.";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::addNotification(const std::string& viewId, int viewStatus)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    DboNotification* entryPtr = new DboNotification();
    entryPtr->timestamp = time(NULL);
    entryPtr->last_change = entryPtr->timestamp;
    entryPtr->view = find<DboView>().where("name=?").bind(viewId);
    entryPtr->view_status = viewStatus;
    entryPtr->ack_status = DboNotification::Open;
    add(entryPtr);

    retValue = 0;
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed to add notification entry into database.";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::updateNotificationAckStatusForUser(const std::string& userId, const std::string& viewId, int newAckStatus)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> dboUser = find<DboUser>().where("name = ?").bind(userId);
    if (! dboUser) {
      m_lastError = QObject::tr("No user with username %1)").arg(userId.c_str()).toStdString();
      CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, m_lastError.c_str()).toStdString());
    } else {
      long lastChange = time(NULL);
      DboViewCollectionT dboViews;
      if (dboUser->role == DboUser::AdmRole) {
        dboViews = find<DboView>();
      } else {
        dboViews = dboUser->views;
      }

      for (auto& dboView : dboViews) {
        std::string realViewId = viewId.empty()? dboView->name : viewId;
        dbo::ptr<DboNotification> notifDbEntry = find<DboNotification>()
            .where("view_name = ?").bind(realViewId)
            .orderBy("timestamp DESC")
            .limit(1);
        DboNotification* notifDbPtr = new DboNotification();
        notifDbPtr->ack_user    = dboUser;
        notifDbPtr->ack_status  = newAckStatus;
        notifDbPtr->view        = dboView;
        notifDbPtr->last_change = lastChange;
        notifDbPtr->view_status = notifDbEntry->view_status;
        notifDbPtr->timestamp   = notifDbEntry->timestamp;
        add(notifDbPtr);
      }
      retValue = 0;
    }
  } catch (const dbo::Exception& ex) {
    m_lastError = "Database error: failed changing notification state.";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::updateNotificationAckStatusForView(const std::string& userId, const std::string& viewId, int newAckStatus)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> dboUser = find<DboUser>().where("name = ?").bind(userId);
    if (! dboUser) {
      m_lastError = QObject::tr("No user with username %1)").arg(userId.c_str()).toStdString();
      CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, m_lastError.c_str()).toStdString());
    } else {
      long lastChange = time(NULL);
      dbo::ptr<DboNotification> notifDbEntry = find<DboNotification>()
          .where("view_name = ?").bind(viewId)
          .orderBy("timestamp DESC")
          .limit(1);
      DboNotification* notifDbPtr = new DboNotification();
      notifDbPtr->ack_user    = dboUser;
      notifDbPtr->ack_status  = newAckStatus;
      notifDbPtr->view        = find<DboView>().where("name = ?").bind(viewId);
      notifDbPtr->last_change = lastChange;
      notifDbPtr->timestamp   = notifDbEntry->timestamp;
      notifDbPtr->view_status = notifDbEntry->view_status;
      add(notifDbPtr);
      retValue = 0;
    }
  } catch (const dbo::Exception& ex) {
    m_lastError = "Database error: failed changing notification state.";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


void DbSession::getLastNotificationInfo(NotificationT& lastNotifInfo, const std::string& viewId)
{
  lastNotifInfo = NotificationT();
  dbo::Transaction transaction(*this);
  try {
    if (! viewId.empty()) {
      dbo::ptr<DboNotification>
          dbNotifEntry = find<DboNotification>()
          .where("view_name = ?").bind(viewId)
          .orderBy("timestamp DESC")
          .limit(1);
      if (dbNotifEntry) {
        lastNotifInfo = dbNotifEntry->data();
      }
    }
  } catch (const dbo::Exception& ex) {
    m_lastError = "Failed fetching notification data";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
}


int DbSession::listViewRelatedNotifications(NotificationMapT& notifications, const std::string& userId)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> dboUser = find<DboUser>().where("name = ?").bind(userId);
    if (! dboUser) {
      m_lastError = QObject::tr("No user with username %1)").arg(userId.c_str()).toStdString();
      CORE_LOG("error", QObject::tr("DbSession::fetchUserRelatedNotifications: %1").arg(m_lastError.c_str()).toStdString());
    } else {
      DboNotificationCollectionT dboNotifications;
      notifications.clear();
      if (dboUser->role == DboUser::AdmRole) {
        dboNotifications = find<DboNotification>().orderBy("last_change");
        for (const auto& entry: dboNotifications) {
          NotificationT data = entry->data();
          notifications.insert(data.view_name, data);
        }
      } else {
        std::string sql = QString("SELECT n.view_name, view_status, ack_status, last_change, ack_user_name"
                                  " FROM notification n, user_view uv"
                                  " WHERE uv.user_name = '%1'"
                                  "   AND n.view_name = uv.view_name"
                                  " ORDER BY last_change"
                                  ).arg(userId.c_str()).toStdString();

        dbo::collection< boost::tuple<std::string, int, int, int, std::string> >
            results = query< boost::tuple<std::string, int, int, int, std::string> >(sql);

        for (const auto& entry: results) {
          NotificationT data;
          data.view_name    = boost::get<0>(entry);
          data.view_status  = boost::get<1>(entry);
          data.ack_status   = boost::get<2>(entry);
          data.last_change  = boost::get<3>(entry);
          data.ack_username = boost::get<4>(entry);
          notifications.insert(data.view_name, data);
        }
      }
      retValue = notifications.size();
    }
  } catch (const dbo::Exception& ex) {
    m_lastError = "Query failed when fetching notification data";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}
