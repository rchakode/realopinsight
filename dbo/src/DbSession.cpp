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
#include "WebBaseSettings.hpp"
#include <tuple>
#include <QFile>
#include <Wt/Auth/HashFunction>
#include <Wt/Auth/Identity>
#include <Wt/Auth/PasswordStrengthValidator>
#include <Wt/Dbo/Exception>
#include <regex>

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
  : m_isConnected(false)
{
  m_dboUserDb = new UserDatabase(*this);
  m_passAuthService = new Wt::Auth::PasswordService(m_basicAuthService);

  try {
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

    // do this before doing anything to avoid unauthorized access
    configureAuth();
    setupDbMapping();

    m_isConnected = true;

  } catch (const std::exception& ex) {
    auto errorMsg = QObject::tr("Connection to database failed: %1").arg(ex.what()).toStdString();
    CORE_LOG("fatal", errorMsg);
    REPORTD_LOG("fatal", errorMsg);
  }
}

DbSession::~DbSession()
{
  delete m_dboUserDb;
  delete m_dboSqlConncetion;
  delete m_passAuthService;
}

void DbSession::setupDbMapping(void)
{
  mapClass<DboUser>("user");
  mapClass<DboView>("view");
  mapClass<AuthInfo>("auth_info");
  mapClass<DboLoginSession>("login_session");
  mapClass<DboQosData>("qosdata");
  mapClass<DboNotification>("notification");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");
  mapClass<DboSource>("data_source");
}


std::pair<int, QString>
DbSession::addUser(const DboUserT& userInfo)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try {
    DboUserCollectionT users = find<DboUser>().where("name=?").bind(userInfo.username);
    if (users.size() > 0) {
      out.second = "a user with the same username already exist.";
      CORE_LOG("error", out.second.toStdString());
      out.first = ngrt4n::RcDbDuplicationError;
    } else {
      Wt::Auth::User dbuser = m_dboUserDb->registerNew();
      dbo::ptr<AuthInfo> info = m_dboUserDb->find(dbuser);
      info.modify()->setEmail(userInfo.email);
      m_passAuthService->updatePassword(dbuser, userInfo.password);
      DboUser* userTmpPtr(new DboUser());
      userTmpPtr->setData(userInfo);
      info.modify()->setUser( add(userTmpPtr) );
      dbuser.addIdentity(Wt::Auth::Identity::LoginName, userInfo.username);
      out.first = ngrt4n::RcSuccess;
    }
  } catch (const dbo::Exception& ex) {
    out.second = "Failed to add the user.";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

std::pair<int, QString>
DbSession::updateUser(const DboUserT& userInfo)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

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
    out.first = ngrt4n::RcSuccess;
    transaction.commit();
  } catch (const dbo::Exception& ex) {
    out.second = "Failed to update user";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

std::pair<int, QString>
DbSession::updatePassword(const std::string& uname, const std::string& currentPass, const std::string& newpass)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try {
    Wt::Auth::User dbuser = m_dboUserDb->findWithIdentity(Wt::Auth::Identity::LoginName, uname);
    switch (m_passAuthService->verifyPassword(dbuser, currentPass)) {
      case Wt::Auth::PasswordValid:
        m_passAuthService->updatePassword(dbuser, newpass);
        out.first = ngrt4n::RcSuccess;
        break;
      case Wt::Auth::PasswordInvalid:
        out.second = "Your current password doesn't match";
        break;
      case Wt::Auth::LoginThrottling:
        out.second = "The account has been blocked. Retry later or contact your administrator";
        break;
      default:
        out.second = "Unknown error concerning your current password";
        break;
    }
  } catch (const dbo::Exception& ex) {
    out.first = ngrt4n::RcDbError;
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}


int DbSession::deleteUser(const std::string& username)
{
  int rc = ngrt4n::RcDbError;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> usr = find<DboUser>().where("name=?").bind(username);
    usr.remove();
    rc = ngrt4n::RcSuccess;
  } catch (const dbo::Exception& ex) {
    rc = ngrt4n::RcDbError;
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return rc;
}


int DbSession::deleteAuthSystemUsers(int authSystem)
{
  int retValue = ngrt4n::RcDbError;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> usr = find<DboUser>().where("authsystem=?").bind(authSystem);
    usr.remove();
    retValue = ngrt4n::RcSuccess;
  } catch (const dbo::Exception& ex) {
    retValue = ngrt4n::RcDbError;
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


bool DbSession::findUser(const std::string& username, DboUserT& user)
{
  auto ulist = listUsers();
  DbUsersT::const_iterator it = std::find_if(ulist.cbegin(), ulist.cend(), [&username](const DboUser& u){return u.username == username;});
  bool found = false;
  if (it != ulist.end()) {
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

DbUsersT DbSession::listUsers(void)
{
  DbUsersT ulist;
  dbo::Transaction transaction(*this);
  try {
    DboUserCollectionT users = find<DboUser>();
    for (auto& user : users) {
      if (user.get()) {
        ulist.push_back(*user);
      }
    }
  } catch (const dbo::Exception& ex) {
    ulist.clear();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return ulist;
}

DbViewsT DbSession::listViews(void)
{
  DbViewsT vlist;
  dbo::Transaction transaction(*this);
  try {
    DboViewCollectionT views = find<DboView>();
    for (auto& view : views) {
      if (view.get()) {
        vlist.push_back(*view);
      }
    }
  } catch (const dbo::Exception& ex) {
    vlist.clear();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return vlist;
}

DbViewsT DbSession::listViewListByAssignedUser(const std::string& uname)
{
  DbViewsT vlist;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> userDboPtr = find<DboUser>().where("name=?").bind(uname);
    for (auto& view : userDboPtr.modify()->views) {
      if (view.get()) {
        vlist.push_back(*view);
      }
    }
  } catch (const dbo::Exception& ex) {
    vlist.clear();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return vlist;
}


bool DbSession::findView(const std::string& vname, DboView& view)
{
  auto vlist = listViews();
  DbViewsT::const_iterator it = std::find_if(vlist.cbegin(), vlist.cend(), [&vname](const DboView& v){return v.name == vname;});
  bool found = false;
  if (it != vlist.end()) {
    found = true;
    view = *it;
  }
  return found;
}


int DbSession::initDb(void)
{
  int  rc = ngrt4n::RcDbError;
  try {
    createTables();
    DboUserT adm;
    adm.username = "admin";
    adm.password = "password";
    adm.firstname = "Default";
    adm.lastname = "Administrator";
    adm.role = DboUser::AdmRole;
    adm.registrationDate = QDateTime::currentDateTime().toString().toStdString();

    auto addUserOut = addUser(adm);
    if (addUserOut.first == ngrt4n::RcSuccess) {
      auto&& msg = Q_TR("Database initialized successfully");
      CORE_LOG("info", msg);
      REPORTD_LOG("info", msg);
    }
    rc = addUserOut.first;
  } catch (dbo::Exception& ex) {
    auto&& msg = QObject::tr("%1: failed initializing the database: %2").arg(Q_FUNC_INFO, ex.what()).toStdString();
    CORE_LOG("error", msg);
    REPORTD_LOG("error", msg);
  }

  return rc;
}

std::pair<int, QString>
DbSession::addView(const DboView& vInfo)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try {
    DboViewCollectionT views = find<DboView>().where("name=?").bind(vInfo.name);
    if (views.size() > 0) {
      out.first = ngrt4n::RcDbDuplicationError;
      out.second = QObject::tr("a view with name '%1' already exists").arg(vInfo.name.c_str());
      CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, out.second).toStdString());
    } else {
      DboView* viewTmpPtr(new DboView());
      *viewTmpPtr =  vInfo;
      add(viewTmpPtr);
      out.first = ngrt4n::RcSuccess;
    }
  } catch (const dbo::Exception& ex) {
    out.second = "Add view failed, please check the log file";
    CORE_LOG("error", ex.what());
  } catch(const std::exception& ex) {
    out.second = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}


std::pair<int, QString>
DbSession::updateViewWithPath(const DboView& vinfo, const std::string& vpath)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboView> viewDbo = find<DboView>().where("path=?").bind(vpath);
    viewDbo.modify()->name = vinfo.name;
    viewDbo.modify()->service_count = vinfo.service_count;
    transaction.commit();
    out.first = ngrt4n::RcSuccess;
  } catch (const dbo::Exception& ex) {
    out.second = "Failed to update view";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}


std::pair<int, QString>
DbSession::deleteViewWithName(const std::string& vname)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try {
    execute("DELETE FROM view WHERE name = ?;").bind(vname);
    out.first = ngrt4n::RcSuccess;
  } catch (const Wt::Dbo::backend::Sqlite3Exception& ex) {
    out.second = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  } catch (const dbo::Exception& ex) {
    out.second = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}



UserViewsT DbSession::updateUserViewList(void)
{
  UserViewsT userViewList;
  dbo::Transaction transaction(*this);
  DboUserCollectionT users = find<DboUser>();
  for (auto& user : users) {
    for (const auto& view: user->views) {
      userViewList.insert(user->username+":"+view->name);
    }
  }
  transaction.commit();

  return userViewList;
}



std::pair<int, QString>
DbSession::assignView(const std::string& userId, const std::string& vname)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> dboUserPtr = find<DboUser>().where("name=?").bind(userId);
    dbo::ptr<DboView> dboViewPtr = find<DboView>().where("name=?").bind(vname);
    dboUserPtr.modify()->views.insert(dboViewPtr);
    out.first = ngrt4n::RcSuccess;
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  } catch(const std::exception& ex) {
    out.second = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}


std::pair<int, QString>
DbSession::revokeView(const std::string& userId, const std::string& viewId)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> userPtr = find<DboUser>().where("name=?").bind(userId);
    dbo::ptr<DboView> viewPtr = find<DboView>().where("name=?").bind(viewId);
    userPtr.modify()->views.erase(viewPtr);
    out.first =  ngrt4n::RcSuccess;
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  } catch(const std::exception& ex) {
    out.second = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}


int DbSession::listAssignedUsersEmails(QStringList& emails, const std::string& vname)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try {
    std::string sql = QString("SELECT user.email"
                              " FROM user, user_view"
                              " WHERE user.name = user_view.user_name"
                              "   AND user_view.view_name = '%1'"
                              "   AND user.email != ''"
                              ).arg(vname.c_str()).toStdString();

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
  int rc = ngrt4n::RcDbError;

  dbo::Transaction transaction(*this);
  try {
    if (checkUserCookie(session) != DboLoginSession::ActiveCookie) {
      DboLoginSession* sessionPtr(new DboLoginSession());
      *sessionPtr = session;
      add(sessionPtr);
      rc = ngrt4n::RcSuccess;
    } else {
      CORE_LOG("debug", "Already active session");
      rc = ngrt4n::RcGenericFailure;
    }
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("Failed adding session at %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return rc;
}


int DbSession::checkUserCookie(const DboLoginSession& session)
{
  int rc = DboLoginSession::InvalidSession;
  dbo::Transaction transaction(*this);
  try {
    DboLoginSessionCollectionT sessions = find<DboLoginSession>()
                                          .where("username=? AND session_id=? AND status = ?")
                                          .bind(session.username)
                                          .bind(session.sessionId)
                                          .bind(DboLoginSession::ExpiredCookie);
    rc = sessions.size()? DboLoginSession::ActiveCookie : DboLoginSession::InvalidSession;
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("failed checking session at %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return rc;
}


int DbSession::addQosData(const QosDataT& qosData)
{
  REPORTD_LOG("info", QObject::tr("Adding QoS entry: %1").arg(qosData.toString().c_str()));

  int retValue = ngrt4n::RcGenericFailure;
  dbo::Transaction transaction(*this);
  try {
    DboQosData* qosDataDbo = new DboQosData();
    qosDataDbo->setData(qosData);
    qosDataDbo->view = find<DboView>().where("name=?").bind(qosData.view_name);
    if (qosDataDbo->view.get() != nullptr) {
      dbo::ptr<DboQosData> dboEntry = add(qosDataDbo);
      retValue = ngrt4n::RcSuccess;
    } else {
      retValue = ngrt4n::RcDbError;
      REPORTD_LOG("error", QObject::tr("%1: Cannot find view: %2").arg(Q_FUNC_INFO, qosData.view_name.c_str()).toStdString());
    }
  } catch (const dbo::Exception& ex) {
    REPORTD_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


std::pair<int, QString>
DbSession::addQosDataList(const QosDataList& qosDataList)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try {
    for (const auto& qosData : qosDataList) {
      DboQosData* ptr_qosDboData = new DboQosData();
      ptr_qosDboData->setData(qosData);
      ptr_qosDboData->view = find<DboView>().where("name=?").bind(qosData.view_name);;
      dbo::ptr<DboQosData> dboEntry = add(ptr_qosDboData);
      out.second.append(QString("QoS entry added: %1").arg(dboEntry->toString().c_str()));
    }
    out.first = 0;
  } catch (const dbo::Exception& ex) {
    out.second = "Failed to add QoS entries to database.";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}


int DbSession::listQosData(QosDataListMapT& qosDataMap, const std::string& viewId, long fromDate, long toDate)
{
  int count = 0;
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
      auto viewDashboardAliasName = entry->data().view_name;
      qosDataMap[viewDashboardAliasName].push_back(entry->data());
      ++count;
    }
  } catch (const dbo::Exception& ex) {
    count = -1;
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return count;
}


int DbSession::getLastQosData(QosDataT& qosData, const std::string& viewId)
{
  int count = -1;
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
      count = 0;
      qosData =  queryResults.begin()->modify()->data();
    }
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("Failed to fetch last QoS entry at %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return count;
}


int DbSession::addNotification(const std::string& viewId, int viewStatus)
{
  int retValue = ngrt4n::RcDbError;
  dbo::Transaction transaction(*this);
  try {
    DboNotification* entryPtr = new DboNotification();
    entryPtr->timestamp = time(nullptr);
    entryPtr->last_change = entryPtr->timestamp;
    entryPtr->view = find<DboView>().where("name=?").bind(viewId);
    entryPtr->view_status = viewStatus;
    entryPtr->ack_status = DboNotification::Open;
    add(entryPtr);

    retValue = ngrt4n::RcSuccess;
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("Failed to add notification entry at %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::updateNotificationAckStatusForUser(const std::string& userId, const std::string& viewId, int newAckStatus)
{
  int retValue = ngrt4n::RcDbError;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> dboUser = find<DboUser>().where("name = ?").bind(userId);
    if (! dboUser) {
      CORE_LOG("error", QObject::tr("No user with username %1 at %2").arg(userId.c_str(), Q_FUNC_INFO).toStdString());
    } else {
      long lastChange = time(nullptr);
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
      retValue =  ngrt4n::RcSuccess;
    }
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("failed changing notification state at %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}


int DbSession::updateNotificationAckStatusForView(const std::string& userId, const std::string& viewId, int newAckStatus)
{
  int retValue = ngrt4n::RcDbError;
  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> dboUser = find<DboUser>().where("name = ?").bind(userId);
    if (! dboUser) {
      CORE_LOG("error", QObject::tr("No user with username %1 when running %2").arg(userId.c_str(), Q_FUNC_INFO).toStdString());
    } else {
      long lastChange = time(nullptr);
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
      retValue =  ngrt4n::RcSuccess;
    }
  } catch (const dbo::Exception& ex) {
    CORE_LOG("error", QObject::tr("failed changing notification state when running %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
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
    CORE_LOG("error", QObject::tr("Failed fetching notification data when running %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
}


std::pair<int, QString>
DbSession::listViewRelatedNotifications(NotificationMapT& notifications, const std::string& userId)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try {
    dbo::ptr<DboUser> dboUser = find<DboUser>().where("name = ?").bind(userId);
    if (! dboUser) {
      CORE_LOG("error", QObject::tr("No user with username %1 when running %2").arg(userId.c_str(), Q_FUNC_INFO).toStdString());

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
      out.first = ngrt4n::RcSuccess;
    }
  } catch (const dbo::Exception& ex) {
    out.second = "Query failed when fetching notification data";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}


std::pair<int, QString>
DbSession::addSource(const SourceT& sinfo)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try {
    DboSourceCollectionT sources = find<DboSource>().where("id=?").bind(sinfo.id.toStdString());
    if (sources.size() < 1) {
      DboSource* dboSource = new DboSource();

      dboSource->id = sinfo.id.toStdString();
      dboSource->mon_type = sinfo.mon_type;
      dboSource->mon_url = sinfo.mon_url.toStdString();
      dboSource->verify_ssl_peer = sinfo.verify_ssl_peer;
      dboSource->ls_addr = sinfo.ls_addr.toStdString();
      dboSource->ls_port = sinfo.ls_port;
      dboSource->auth = sinfo.auth.toStdString();
      dboSource->icon = sinfo.icon.toStdString();

      add(dboSource);

      out.first = ngrt4n::RcSuccess;
    } else {
      out.first = ngrt4n::RcDbDuplicationError;
    }
  } catch (const dbo::Exception& ex) {
    out.second  = "Failed adding source, check log files for more details";
    CORE_LOG("error", QObject::tr("error at %1 adding source in database (%2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}


std::pair<int, QString>
DbSession::updateSource(const SourceT& sinfo)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);

  try {
    dbo::ptr<DboSource> source = find<DboSource>().where("id=?").bind(sinfo.id.toStdString());
    source.modify()->mon_type = sinfo.mon_type;
    source.modify()->mon_url = sinfo.mon_url.toStdString();
    source.modify()->ls_addr = sinfo.ls_addr.toStdString();
    source.modify()->ls_port = sinfo.ls_port;
    source.modify()->auth = sinfo.auth.toStdString();
    source.modify()->verify_ssl_peer = sinfo.verify_ssl_peer;
    source.modify()->icon = sinfo.icon.toStdString();

    transaction.commit();

    out.first = ngrt4n::RcSuccess;
  } catch (const dbo::Exception& ex) {
    out.second = "Failed to update view";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }

  transaction.commit();

  return out;
}


std::pair<int, QString>
DbSession::deleteSource(const QString& sid)
{
  std::pair<int, QString> out {ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try {
    execute("DELETE FROM data_source WHERE id = ?;").bind(sid.toStdString());
    out.first = ngrt4n::RcSuccess;
  } catch (const Wt::Dbo::backend::Sqlite3Exception& ex) {
    out.second = QObject::tr("failed deleting source");
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  } catch (const dbo::Exception& ex) {
    out.second = QObject::tr("failed deleting source");
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}


SourceListT DbSession::listSources(int monType)
{
  SourceListT sources;
  dbo::Transaction transaction(*this);
  try {
    DboSourceCollectionT dboSources = find<DboSource>();
    for (const auto& dboSrc : dboSources) {

      if (! dboSrc.get()) {
        continue;
      }
      if (dboSrc->mon_type != monType && monType != MonitorT::Any) {
        continue;
      }

      SourceT sinfo;
      sinfo.id       = QString::fromStdString(dboSrc->id);
      sinfo.mon_type = static_cast<qint8>(dboSrc->mon_type);
      sinfo.mon_url  = QString::fromStdString(dboSrc->mon_url);
      sinfo.ls_addr  = QString::fromStdString(dboSrc->ls_addr);
      sinfo.ls_port  = static_cast<qint32>(dboSrc->ls_port);
      sinfo.auth     = QString::fromStdString(dboSrc->auth);
      sinfo.icon     = QString::fromStdString(dboSrc->id);
      sinfo.verify_ssl_peer = static_cast<qint8>(dboSrc->verify_ssl_peer);

      sources.insert(sinfo.id, sinfo);
    }
  } catch (const dbo::Exception& ex) {
    sources.clear();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return sources;
}

std::pair<bool, SourceT>
DbSession::findSourceById(const QString& sid)
{
  auto sourceList = listSources(MonitorT::Any);
  auto sinfo = sourceList.find(sid);
  if (sinfo != std::end(sourceList)) {
    return {true, *sinfo};
  }
  return {false, SourceT{}};
}
