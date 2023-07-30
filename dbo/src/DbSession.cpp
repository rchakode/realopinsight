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
#include <regex>
#include <QFile>
#include <Wt/Auth/Identity.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/Dbo/Exception.h>

namespace Wt
{
  namespace Dbo
  {
    namespace backend
    {
      class Sqlite3Exception : public Exception
      {
      public:
        Sqlite3Exception(const std::string &msg)
          : Exception(msg) {}
      };
    }
  }
}

DbSession::DbSession()
  : m_dbIsReady(false)
{
  m_usersDb = new UserDatabase(*this);
  m_passAuthService = new Wt::Auth::PasswordService(m_basicAuthService);
  WebBaseSettings settings;
  try
  {
    switch (settings.getDbType())
    {
      case PostgresqlDb:
        setConnection(std::make_unique<Wt::Dbo::backend::Postgres>(settings.getDbConnectionName()));
        break;
      case Sqlite3Db:
      default:
        setConnection(std::make_unique<Wt::Dbo::backend::Sqlite3>(settings.getDbConnectionName()));
        break;
    }
    configureAuth();
    setupDbMapping();
    m_dbIsReady = true;
  }
  catch (const std::exception& ex)
  {
    CORE_LOG("fatal", QObject::tr("failed to connect to database %1 (%2)")
             .arg(settings.getDbConnectionNameDebug().c_str(), ex.what()).toStdString());
  }
}

DbSession::~DbSession()
{
  delete m_usersDb;
  delete m_passAuthService;
}


void DbSession::setupDbMapping(void)
{
  mapClass<DboUser>("user");
  mapClass<DboView>("view");
  mapClass<AuthInfo>("auth_info");
  mapClass<DboLoginSession>("login_session");
  mapClass<DboPlatformStatus>("qosdata");
  mapClass<DboNotification>("notification");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");
  mapClass<DboSource>("data_source");
}

std::pair<int, QString>
DbSession::addUser(const DboUserT &userInfo)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};
  dbo::Transaction transaction(*this);
  try
  {
    DboUserCollectionT users = find<DboUser>().where("name=?").bind(userInfo.username);
    if (users.size() > 0)
    {
      out.second = "a user with the same username already exist.";
      CORE_LOG("error", out.second.toStdString());
      out.first = ngrt4n::RcDbDuplicationError;
    }
    else
    {
      Wt::Auth::User dbuser = m_usersDb->registerNew();
      dbo::ptr<AuthInfo> dbouserPtr = m_usersDb->find(dbuser);
      dbouserPtr.modify()->setEmail(userInfo.email);
      m_passAuthService->updatePassword(dbuser, userInfo.password);
      auto userPtr = std::make_unique<DboUser>();
      userPtr->setData(userInfo);
      dbouserPtr.modify()->setUser(add(std::move(userPtr)));
      dbuser.addIdentity(Wt::Auth::Identity::LoginName, userInfo.username);
      out.first = ngrt4n::RcSuccess;
    }
  }
  catch (const dbo::Exception &ex)
  {
    out.second = "Failed to add the user.";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

std::pair<int, QString>
DbSession::updateUser(const DboUserT &userInfo)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try
  {
    dbo::ptr<AuthInfo> authInfo = find<AuthInfo>().where("user_name=?").bind(userInfo.username);
    dbo::ptr<DboUser> userPtr = authInfo.modify()->user();
    userPtr.modify()->username = userInfo.username;
    userPtr.modify()->lastname = userInfo.lastname;
    userPtr.modify()->firstname = userInfo.firstname;
    userPtr.modify()->email = userInfo.email;
    userPtr.modify()->role = userInfo.role;
    userPtr.modify()->dashboardDisplayMode = userInfo.opsProfileMode;
    userPtr.modify()->dashboardTilesPerRow = userInfo.opsProfileTilesPerRow;
    authInfo.modify()->setEmail(userInfo.email);
    out.first = ngrt4n::RcSuccess;
    transaction.commit();
  }
  catch (const dbo::Exception &ex)
  {
    out.second = "Failed to update user";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

std::pair<int, QString>
DbSession::updatePassword(const std::string &uname, const std::string &currentPass, const std::string &newpass)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try
  {
    Wt::Auth::User dbuser = m_usersDb->findWithIdentity(Wt::Auth::Identity::LoginName, uname);
    switch (m_passAuthService->verifyPassword(dbuser, currentPass))
    {
      case Wt::Auth::PasswordResult::PasswordValid:
        m_passAuthService->updatePassword(dbuser, newpass);
        out.first = ngrt4n::RcSuccess;
        break;
      case Wt::Auth::PasswordResult::PasswordInvalid:
        out.second = "Your current password doesn't match";
        break;
      case Wt::Auth::PasswordResult::LoginThrottling:
        out.second = "The account has been blocked. Retry later or contact your administrator";
        break;
      default:
        out.second = "Unknown error concerning your current password";
        break;
    }
  }
  catch (const dbo::Exception &ex)
  {
    out.first = ngrt4n::RcDbError;
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

int DbSession::deleteUser(const std::string &username)
{
  int rc = ngrt4n::RcDbError;
  dbo::Transaction transaction(*this);
  try
  {
    dbo::ptr<DboUser> usr = find<DboUser>().where("name=?").bind(username);
    usr.remove();
    rc = ngrt4n::RcSuccess;
  }
  catch (const dbo::Exception &ex)
  {
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
  try
  {
    dbo::ptr<DboUser> theUser = find<DboUser>().where("authsystem=?").bind(authSystem);
    theUser.remove();
    retValue = ngrt4n::RcSuccess;
  }
  catch (const dbo::Exception &ex)
  {
    retValue = ngrt4n::RcDbError;
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}

bool DbSession::findUser(const std::string &username, DboUserT &user)
{
  auto ulist = listUsers();
  auto theUser = std::find_if(ulist.cbegin(), ulist.cend(), [&username](const DboUser &u) { return u.username == username; });
  bool found = false;
  if (theUser != ulist.end()) {
    found = true;
    user = theUser->data();
  }
  return found;
}


Wt::Auth::User DbSession::findAuthUser(const std::string& username, const std::string& password)
{
  Wt::Auth::User user = m_usersDb->findWithIdentity(Wt::Auth::Identity::LoginName, username);
  if (m_passAuthService->verifyPassword(user, password) == Wt::Auth::PasswordResult::PasswordValid
      && user.status() ==  Wt::Auth::AccountStatus::Normal) {
    return user;
  }
  return Wt::Auth::User();
}

void DbSession::configureAuth(void)
{
  m_basicAuthService.setAuthTokensEnabled(true, "realopinsightcookie");
  m_basicAuthService.setEmailVerificationEnabled(true);
  auto verifier = std::make_unique<Wt::Auth::PasswordVerifier>();
  verifier->addHashFunction(std::make_unique<Wt::Auth::BCryptHashFunction>(7));
  m_passAuthService->setVerifier(std::move(verifier));
  m_passAuthService->setStrengthValidator(std::make_unique<Wt::Auth::PasswordStrengthValidator>());
  m_passAuthService->setAttemptThrottlingEnabled(true);
}

void DbSession::decodeLoggedUser(void)
{
  dbo::Transaction transaction(*this);
  try
  {
    std::string dbUserId = wtAuthLogin().user().id();
    dbo::ptr<AuthInfo> info = find<AuthInfo>().where("id=?").bind(dbUserId);
    m_loggedUser = *(info.modify()->user());
  }
  catch (const dbo::Exception &ex)
  {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
}

DbUsersT DbSession::listUsers(void)
{
  DbUsersT ulist;
  dbo::Transaction transaction(*this);
  try
  {
    DboUserCollectionT users = find<DboUser>();
    for (auto &user : users)
    {
      if (user.get())
      {
        ulist.push_back(*user);
      }
    }
  }
  catch (const dbo::Exception &ex)
  {
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
  try
  {
    DboViewCollectionT views = find<DboView>();
    for (auto &view : views)
    {
      if (view.get())
      {
        vlist.push_back(*view);
      }
    }
  }
  catch (const dbo::Exception &ex)
  {
    vlist.clear();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return vlist;
}

DbViewsT DbSession::listAssignedViewsByUser(const std::string &uname)
{
  DbViewsT vlist;
  dbo::Transaction transaction(*this);
  try
  {
    dbo::ptr<DboUser> userDboPtr = find<DboUser>().where("name=?").bind(uname);
    for (auto &view : userDboPtr.modify()->views)
    {
      if (view.get())
      {
        vlist.push_back(*view);
      }
    }
  }
  catch (const dbo::Exception &ex)
  {
    vlist.clear();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return vlist;
}

bool DbSession::findView(const std::string &vname, DboView &view)
{
  auto vlist = listViews();
  DbViewsT::const_iterator it = std::find_if(vlist.cbegin(), vlist.cend(), [&vname](const DboView &v) { return v.name == vname; });
  bool found = false;
  if (it != vlist.end())
  {
    found = true;
    view = *it;
  }
  return found;
}

int DbSession::initDb(void)
{
  int rc = ngrt4n::RcDbError;
  try
  {
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
      auto &&msg = Q_TR("Database initialized successfully");
      CORE_LOG("info", msg);
    }
    rc = addUserOut.first;
  }
  catch (dbo::Exception &ex) {
    auto &&msg = QObject::tr("%1: failed initializing the database: %2").arg(Q_FUNC_INFO, ex.what()).toStdString();
    CORE_LOG("error", msg);
  }

  return rc;
}

std::pair<int, QString>
DbSession::addView(const DboView &vInfo)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try
  {
    DboViewCollectionT views = find<DboView>().where("name=?").bind(vInfo.name);
    if (views.size() > 0) {
      out.first = ngrt4n::RcDbDuplicationError;
      out.second = QObject::tr("a view with name '%1' already exists").arg(vInfo.name.c_str());
      CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, out.second).toStdString());
    } else {
      auto dboViewPtr = std::make_unique<DboView>();
      auto dboViewRawPtr = dboViewPtr.get();
      *dboViewRawPtr = vInfo;
      add(std::move(dboViewPtr));
      out.first = ngrt4n::RcSuccess;
    }
  }
  catch (const dbo::Exception &ex)
  {
    out.second = "Add view failed, please check the log file";
    CORE_LOG("error", ex.what());
  }
  catch (const std::exception &ex)
  {
    out.second = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

std::pair<int, QString>
DbSession::updateViewWithPath(const DboView &vinfo, const std::string &vpath)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try
  {
    dbo::ptr<DboView> viewDbo = find<DboView>().where("path=?").bind(vpath);
    viewDbo.modify()->name = vinfo.name;
    viewDbo.modify()->service_count = vinfo.service_count;
    transaction.commit();
    out.first = ngrt4n::RcSuccess;
  }
  catch (const dbo::Exception &ex)
  {
    out.second = "Failed to update view";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

std::pair<int, QString>
DbSession::deleteViewWithName(const std::string &vname)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try
  {
    execute("DELETE FROM view WHERE name = ?;").bind(vname);
    out.first = ngrt4n::RcSuccess;
  }
  catch (const Wt::Dbo::backend::Sqlite3Exception &ex)
  {
    out.second = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  catch (const dbo::Exception &ex)
  {
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
  for (auto &user : users)
  {
    for (const auto &view : user->views)
    {
      userViewList.insert(user->username + ":" + view->name);
    }
  }
  transaction.commit();

  return userViewList;
}

std::pair<int, QString>
DbSession::assignView(const std::string &userId, const std::string &vname)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try
  {
    dbo::ptr<DboUser> dboUserPtr = find<DboUser>().where("name=?").bind(userId);
    dbo::ptr<DboView> dboViewPtr = find<DboView>().where("name=?").bind(vname);
    dboUserPtr.modify()->views.insert(dboViewPtr);
    out.first = ngrt4n::RcSuccess;
  }
  catch (const dbo::Exception &ex)
  {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  catch (const std::exception &ex)
  {
    out.second = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

std::pair<int, QString>
DbSession::revokeView(const std::string &userId, const std::string &viewId)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try
  {
    dbo::ptr<DboUser> userPtr = find<DboUser>().where("name=?").bind(userId);
    dbo::ptr<DboView> viewPtr = find<DboView>().where("name=?").bind(viewId);
    userPtr.modify()->views.erase(viewPtr);
    out.first = ngrt4n::RcSuccess;
  }
  catch (const dbo::Exception &ex)
  {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  catch (const std::exception &ex)
  {
    out.second = ex.what();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

int DbSession::listAssignedUsersEmails(QStringList &emails, const std::string &vname)
{
  int retValue = -1;
  dbo::Transaction transaction(*this);
  try
  {
    std::string sql = QString("SELECT user.email"
                              " FROM user, user_view"
                              " WHERE user.name = user_view.user_name"
                              "   AND user_view.view_name = '%1'"
                              "   AND user.email != ''")
        .arg(vname.c_str())
        .toStdString();

    dbo::collection<std::string> results = query<std::string>(sql);
    emails.clear();
    for (const auto &entry : results) {
      emails.push_back(QString::fromStdString(entry));
    }
    retValue = emails.size();
  }
  catch (const dbo::Exception &ex)
  {
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}

int DbSession::addSession(const DboLoginSession &session)
{
  int rc = ngrt4n::RcDbError;
  dbo::Transaction transaction(*this);
  try
  {
    if (checkUserCookie(session) != DboLoginSession::ActiveCookie) {
      auto dboSessionPtr = std::make_unique<DboLoginSession>();
      auto dboSessionRawPtr = dboSessionPtr.get();
      *dboSessionRawPtr = session;
      add(std::move(dboSessionPtr));
      rc = ngrt4n::RcSuccess;
    } else {
      CORE_LOG("debug", "Already active session");
      rc = ngrt4n::RcGenericFailure;
    }
  }
  catch (const dbo::Exception &ex)
  {
    CORE_LOG("error", QObject::tr("Failed adding session at %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return rc;
}

int DbSession::checkUserCookie(const DboLoginSession &session)
{
  int rc = DboLoginSession::InvalidSession;
  dbo::Transaction transaction(*this);
  try
  {
    DboLoginSessionCollectionT sessions = find<DboLoginSession>()
        .where("username=? AND session_id=? AND status = ?")
        .bind(session.username)
        .bind(session.sessionId)
        .bind(DboLoginSession::ExpiredCookie);
    rc = sessions.size() ? DboLoginSession::ActiveCookie : DboLoginSession::InvalidSession;
  }
  catch (const dbo::Exception &ex) {
    CORE_LOG("error", QObject::tr("failed checking session at %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return rc;
}

int DbSession::addPlatformStatus(const PlatformStatusT &platformStatus)
{
  int retValue = ngrt4n::RcGenericFailure;
  dbo::Transaction transaction(*this);
  try
  {
    auto pfsInfo = std::make_unique<DboPlatformStatus>();
    pfsInfo->setData(platformStatus);
    pfsInfo->view = find<DboView>().where("name=?").bind(platformStatus.view_name);
    if (pfsInfo->view.get() != nullptr) {
      dbo::ptr<DboPlatformStatus> dboEntry = add(std::move(pfsInfo));
      retValue = ngrt4n::RcSuccess;
    } else {
      retValue = ngrt4n::RcDbError;
      REPORTD_LOG("error", QObject::tr("%1: Cannot find view: %2").arg(Q_FUNC_INFO, platformStatus.view_name.c_str()).toStdString());
    }
  }
  catch (const dbo::Exception &ex) {
    REPORTD_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}

std::pair<int, QString>
DbSession::addPlatformStatusList(const ListofPlatformStatusT &platformStatusList)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try
  {
    for (const auto &pfsItem : platformStatusList) {
      auto dboPFS = std::make_unique<DboPlatformStatus>();
      dboPFS->setData(pfsItem);
      dboPFS->view = find<DboView>().where("name=?").bind(pfsItem.view_name);
      dbo::ptr<DboPlatformStatus> dboEntry = add(std::move(dboPFS));
      out.second.append(QString("Platform status entry added: %1").arg(dboEntry->toString().c_str()));
    }
    out.first = 0;
  }
  catch (const dbo::Exception &ex)
  {
    out.second = "Failed to add platform status entries to database.";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

int DbSession::listStatusHistory(PlatformMappedStatusHistoryT& statusHistory, const std::string& view, long startDate, long endDate)
{
  int count = 0;
  dbo::Transaction transaction(*this);
  try
  {
    DboPlatformStatusCollectionT dbEntries;
    if (view.empty() || view == "ALL") {
      dbEntries = find<DboPlatformStatus>()
          .where("timestamp >= ? AND timestamp <= ?")
          .orderBy("timestamp")
          .bind(startDate)
          .bind(endDate);
    } else {
      dbEntries = find<DboPlatformStatus>()
          .where("view_name = ? AND timestamp >= ? AND timestamp <= ?")
          .orderBy("timestamp")
          .bind(view)
          .bind(startDate)
          .bind(endDate);
    }

    statusHistory.clear();
    for (auto &entry : dbEntries) {
      auto viewDashboardAliasName = entry->data().view_name;
      statusHistory[viewDashboardAliasName].push_back(entry->data());
      ++count;
    }
  }
  catch (const dbo::Exception &ex)
  {
    count = -1;
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return count;
}

int DbSession::getLastPlatformStatus(PlatformStatusT &platformStatus, const std::string &view)
{
  int count = -1;
  dbo::Transaction transaction(*this);
  try
  {
    DboPlatformStatusCollectionT queryResults;
    if (view.empty()) {
      queryResults = find<DboPlatformStatus>()
          .orderBy("timestamp DESC")
          .limit(1);
    } else {
      queryResults = find<DboPlatformStatus>()
          .orderBy("timestamp DESC")
          .limit(1);
    }

    if (queryResults.size() == 1) {
      count = 0;
      platformStatus = queryResults.begin()->modify()->data();
    }
  }
  catch (const dbo::Exception &ex)
  {
    CORE_LOG("error", QObject::tr("Failed to fetch last platform status entry at %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return count;
}

int DbSession::addNotification(const std::string &viewId, int viewStatus)
{
  int retValue = ngrt4n::RcDbError;
  dbo::Transaction transaction(*this);
  try
  {
    auto dboNotifPtr = std::make_unique<DboNotification>();
    dboNotifPtr.get()->timestamp = time(nullptr);
    dboNotifPtr.get()->last_change = dboNotifPtr->timestamp;
    dboNotifPtr.get()->view = find<DboView>().where("name=?").bind(viewId);
    dboNotifPtr.get()->view_status = viewStatus;
    dboNotifPtr.get()->ack_status = DboNotification::Open;
    add(std::move(dboNotifPtr));

    retValue = ngrt4n::RcSuccess;
  }
  catch (const dbo::Exception &ex)
  {
    CORE_LOG("error", QObject::tr("Failed to add notification entry at %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}

int DbSession::updateNotificationAckStatusForUser(const std::string &userId, const std::string &viewId, int newAckStatus)
{
  int retValue = ngrt4n::RcDbError;
  dbo::Transaction transaction(*this);
  try
  {
    dbo::ptr<DboUser> dboUser = find<DboUser>().where("name = ?").bind(userId);
    if (!dboUser) {
      CORE_LOG("error", QObject::tr("No user with username %1 at %2").arg(userId.c_str(), Q_FUNC_INFO).toStdString());
    }
    else {
      long lastChange = time(nullptr);
      DboViewCollectionT dboViews;
      if (dboUser->role == DboUser::AdmRole) {
        dboViews = find<DboView>();
      } else {
        dboViews = dboUser->views;
      }

      for (auto &dboView : dboViews) {
        std::string realViewId = viewId.empty() ? dboView->name : viewId;
        dbo::ptr<DboNotification> notifDbEntry = find<DboNotification>()
            .where("view_name = ?")
            .bind(realViewId)
            .orderBy("timestamp DESC")
            .limit(1);
        auto dboNotifPtr = std::make_unique<DboNotification>();
        dboNotifPtr->ack_user = dboUser;
        dboNotifPtr->ack_status = newAckStatus;
        dboNotifPtr->view = dboView;
        dboNotifPtr->last_change = lastChange;
        dboNotifPtr->view_status = notifDbEntry->view_status;
        dboNotifPtr->timestamp = notifDbEntry->timestamp;
        add(std::move(dboNotifPtr));
      }
      retValue = ngrt4n::RcSuccess;
    }
  }
  catch (const dbo::Exception &ex)
  {
    CORE_LOG("error", QObject::tr("failed changing notification state at %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}

int DbSession::updateNotificationAckStatusForView(const std::string &userId, const std::string &viewId, int newAckStatus)
{
  int retValue = ngrt4n::RcDbError;
  dbo::Transaction transaction(*this);
  try
  {
    dbo::ptr<DboUser> dboUser = find<DboUser>().where("name = ?").bind(userId);
    if (!dboUser) {
      CORE_LOG("error", QObject::tr("No user with username %1 when running %2").arg(userId.c_str(), Q_FUNC_INFO).toStdString());
    }
    else {
      long lastChange = time(nullptr);
      dbo::ptr<DboNotification> notifDbEntry = find<DboNotification>()
          .where("view_name = ?")
          .bind(viewId)
          .orderBy("timestamp DESC")
          .limit(1);
      auto dboNotifPtr = std::make_unique<DboNotification>();
      dboNotifPtr->ack_user = dboUser;
      dboNotifPtr->ack_status = newAckStatus;
      dboNotifPtr->view = find<DboView>().where("name = ?").bind(viewId);
      dboNotifPtr->last_change = lastChange;
      dboNotifPtr->timestamp = notifDbEntry->timestamp;
      dboNotifPtr->view_status = notifDbEntry->view_status;
      add(std::move(dboNotifPtr));
      retValue = ngrt4n::RcSuccess;
    }
  }
  catch (const dbo::Exception &ex)
  {
    CORE_LOG("error", QObject::tr("failed changing notification state when running %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
  return retValue;
}

void DbSession::getLastNotificationInfo(NotificationT &lastNotifInfo, const std::string &viewId)
{
  lastNotifInfo = NotificationT();
  dbo::Transaction transaction(*this);
  try
  {
    if (!viewId.empty())
    {
      dbo::ptr<DboNotification>
          dbNotifEntry = find<DboNotification>()
          .where("view_name = ?")
          .bind(viewId)
          .orderBy("timestamp DESC")
          .limit(1);
      if (dbNotifEntry)
      {
        lastNotifInfo = dbNotifEntry->data();
      }
    }
  }
  catch (const dbo::Exception &ex)
  {
    CORE_LOG("error", QObject::tr("Failed fetching notification data when running %1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();
}

std::pair<int, QString>
DbSession::listViewRelatedNotifications(NotificationMapT &notifications, const std::string &userId)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);
  try
  {
    dbo::ptr<DboUser> dboUser = find<DboUser>().where("name = ?").bind(userId);
    if (!dboUser)
    {
      CORE_LOG("error", QObject::tr("No user with username %1 when running %2").arg(userId.c_str(), Q_FUNC_INFO).toStdString());
    }
    else
    {
      DboNotificationCollectionT dboNotifications;
      notifications.clear();
      if (dboUser->role == DboUser::AdmRole)
      {
        dboNotifications = find<DboNotification>().orderBy("last_change");
        for (const auto &entry : dboNotifications)
        {
          NotificationT data = entry->data();
          notifications.insert(data.view_name, data);
        }
      }
      else
      {
        std::string sql = QString("SELECT n.view_name, view_status, ack_status, last_change, ack_user_name"
                                  " FROM notification n, user_view uv"
                                  " WHERE uv.user_name = '%1'"
                                  "   AND n.view_name = uv.view_name"
                                  " ORDER BY last_change")
            .arg(userId.c_str())
            .toStdString();

        dbo::collection<std::tuple<std::string, int, int, int, std::string>>
            results = query<std::tuple<std::string, int, int, int, std::string>>(sql);

        for (const auto &entry : results) {
          NotificationT data;
          data.view_name = std::get<0>(entry);
          data.view_status = std::get<1>(entry);
          data.ack_status = std::get<2>(entry);
          data.last_change = std::get<3>(entry);
          data.ack_username = std::get<4>(entry);
          notifications.insert(data.view_name, data);
        }
      }
      out.first = ngrt4n::RcSuccess;
    }
  }
  catch (const dbo::Exception &ex)
  {
    out.second = "Query failed when fetching notification data";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

std::pair<int, QString>
DbSession::addSource(const SourceT &sinfo)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};
  dbo::Transaction transaction(*this);
  try
  {
    DboSourceCollectionT sources = find<DboSource>().where("id=?").bind(sinfo.id.toStdString());
    if (sources.size() < 1) {
      auto dboSourcePtr = std::make_unique<DboSource>();
      dboSourcePtr->id = sinfo.id.toStdString();
      dboSourcePtr->mon_type = sinfo.mon_type;
      dboSourcePtr->mon_url = sinfo.mon_url.toStdString();
      dboSourcePtr->verify_ssl_peer = sinfo.verify_ssl_peer;
      dboSourcePtr->ls_addr = sinfo.ls_addr.toStdString();
      dboSourcePtr->ls_port = sinfo.ls_port;
      dboSourcePtr->auth = sinfo.auth.toStdString();
      dboSourcePtr->icon = sinfo.icon.toStdString();
      add(std::move(dboSourcePtr));
      out.first = ngrt4n::RcSuccess;
    } else {
      out.first = ngrt4n::RcDbDuplicationError;
    }
  }
  catch (const dbo::Exception &ex)
  {
    out.second = "Failed adding source, check log files for more details";
    CORE_LOG("error", QObject::tr("error at %1 adding source in database (%2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return out;
}

std::pair<int, QString>
DbSession::updateSource(const SourceT &sinfo)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};

  dbo::Transaction transaction(*this);

  try
  {
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
  }
  catch (const dbo::Exception &ex)
  {
    out.second = "Failed to update view";
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }

  transaction.commit();

  return out;
}

std::pair<int, QString>
DbSession::deleteSource(const QString &sid)
{
  std::pair<int, QString> out{ngrt4n::RcDbError, ""};
  dbo::Transaction transaction(*this);
  try
  {
    execute("DELETE FROM data_source WHERE id = ?;").bind(sid.toStdString());
    out.first = ngrt4n::RcSuccess;
  }
  catch (const Wt::Dbo::backend::Sqlite3Exception &ex)
  {
    out.second = QObject::tr("failed deleting source");
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  catch (const dbo::Exception &ex)
  {
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
  try
  {
    DboSourceCollectionT dboSources = find<DboSource>();
    for (const auto &dboSrc : dboSources)
    {

      if (!dboSrc.get())
      {
        continue;
      }
      if (dboSrc->mon_type != monType && monType != MonitorT::Any)
      {
        continue;
      }

      SourceT sinfo;
      sinfo.id = QString::fromStdString(dboSrc->id);
      sinfo.mon_type = static_cast<qint8>(dboSrc->mon_type);
      sinfo.mon_url = QString::fromStdString(dboSrc->mon_url);
      sinfo.ls_addr = QString::fromStdString(dboSrc->ls_addr);
      sinfo.ls_port = static_cast<qint32>(dboSrc->ls_port);
      sinfo.auth = QString::fromStdString(dboSrc->auth);
      sinfo.icon = QString::fromStdString(dboSrc->id);
      sinfo.verify_ssl_peer = static_cast<qint8>(dboSrc->verify_ssl_peer);

      sources.insert(sinfo.id, sinfo);
    }
  }
  catch (const dbo::Exception &ex)
  {
    sources.clear();
    CORE_LOG("error", QObject::tr("%1: %2").arg(Q_FUNC_INFO, ex.what()).toStdString());
  }
  transaction.commit();

  return sources;
}

std::pair<bool, SourceT>
DbSession::findSourceById(const QString &sid)
{
  auto sourceList = listSources(MonitorT::Any);
  auto sinfo = sourceList.find(sid);
  if (sinfo != std::end(sourceList))
  {
    return {true, *sinfo};
  }
  return {false, SourceT{}};
}
