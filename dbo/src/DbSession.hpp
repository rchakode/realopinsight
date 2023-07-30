/*
 * DbSession.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
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

#ifndef DBSESSION_HPP
#define DBSESSION_HPP

#include <climits>
#include <semaphore.h>
#include "dbo/src/DbObjects.hpp"
#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/User.h>
#include <Wt/WGlobal.h>
#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/HashFunction.h>

typedef Wt::Auth::Dbo::AuthInfo<DboUser> AuthInfo;
typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

enum {
  Sqlite3Db = 0,
  PostgresqlDb = 1
};

enum {
  DbNotInitialized = 0,
  DbInitialized = 1
};

class DbSession : public dbo::Session
{
public:
  DbSession();
  ~DbSession();

  bool isReady() const {return m_dbIsReady;}

  void setupDbMapping(void);
  int initDb(void);
  void configureAuth(void);
  void decodeLoggedUser(void);
  Wt::Auth::AbstractUserDatabase& users() const {
    return *m_usersDb;
  }
  Wt::Auth::AuthService& auth() {
    return m_basicAuthService;
  }
  Wt::Auth::PasswordService* passwordAuthenticator(void) {
    return m_passAuthService;
  }
  Wt::Auth::Login& wtAuthLogin(void) {
    rereadAll();
    return m_wtAuthLogin;
  }
  bool isLogged(void) {
    return wtAuthLogin().loggedIn();
  }
  bool isLoggedAdmin(void) {
    return loggedUser().role == DboUser::AdmRole;
  }
  const DboUser& loggedUser(void) const {
    return m_loggedUser;
  }
  bool isCompleteUserDashboard(void) const {
    return loggedUser().dashboardDisplayMode == DboUser::OperationsProfileFull;
  }
  bool displayOnlyTiles(void) const {
    return loggedUser().dashboardDisplayMode == DboUser::OperationsProfileTacticalViewOnly;
  }
  bool isReportUserDashboard(void) const {
    return loggedUser().dashboardDisplayMode == DboUser::OperationsProfileNoAnalytics;
  }
  int boardCardsPerRow(void) const {
    return (loggedUser().dashboardTilesPerRow <= 0 ? 5 : loggedUser().dashboardTilesPerRow);
  }
  QString loggedUserName(void)const {
    return QString::fromStdString(loggedUser().username);
  }

  std::pair<int, QString> addUser(const DboUserT& userInfo);
  std::pair<int, QString> updateUser(const DboUserT& userInfo);
  int deleteUser(const std::string& username);
  int deleteAuthSystemUsers(int authSystem);
  bool findUser(const std::string& username, DboUserT& user);
  Wt::Auth::User findAuthUser(const std::string& username, const std::string& password);
  std::pair<int, QString> updatePassword(const std::string& uname, const std::string& currentPass, const std::string& newPass);
  DbUsersT listUsers(void);

  std::pair<int, QString> addView(const DboView& vinfo);
  std::pair<int, QString> updateViewWithPath(const DboView& vinfo, const std::string& vpath);
  std::pair<int, QString> deleteViewWithName(const std::string& vname);
  std::pair<int, QString> assignView(const std::string& userId, const std::string& vname);
  std::pair<int, QString> revokeView(const std::string& userId, const std::string& vname);
  int listAssignedUsersEmails(QStringList& emails, const std::string& vname);

  int addPlatformStatus(const PlatformStatusT& platformStatus);
  std::pair<int, QString>  addPlatformStatusList(const ListofPlatformStatusT& platformStatusList);
  int listStatusHistory(PlatformMappedStatusHistoryT& statusHistory, const std::string& view, long startDate = 0, long endDate = LONG_MAX);
  int getLastPlatformStatus(PlatformStatusT& platformStatus, const std::string& view);

  DbViewsT listViews(void);
  DbViewsT listAssignedViewsByUser(const std::string& uname);
  UserViewsT updateUserViewList(void);
  bool findView(const std::string& vname, DboView& view);

  int addSession(const DboLoginSession& session);
  int checkUserCookie(const DboLoginSession& session);

  int addNotification(const std::string& viewId, int viewStatus);
  int updateNotificationAckStatusForUser(const std::string& userId, const std::string& viewId, int newAckStatus);
  int updateNotificationAckStatusForView(const std::string& userId, const std::string& viewId, int newAckStatus);
  void getLastNotificationInfo(NotificationT& lastNotifInfo, const std::string& viewId);
  std::pair<int, QString> listViewRelatedNotifications(NotificationMapT& notifications, const std::string& userId);

  std::pair<int, QString> addSource(const SourceT& sinfo);
  std::pair<int, QString> updateSource(const SourceT& sinfo);
  std::pair<int, QString> deleteSource(const QString& sid);
  SourceListT listSources(int monType);
  std::pair<bool, SourceT> findSourceById(const QString& sid);

private:
  bool m_dbIsReady;
  UserDatabase* m_usersDb;
  DboUser m_loggedUser;
  Wt::Auth::Login m_wtAuthLogin;
  Wt::Auth::AuthService m_basicAuthService;
  Wt::Auth::PasswordService* m_passAuthService;

  std::string hashPassword(const std::string& pass) {
    Wt::Auth::BCryptHashFunction h;
    return h.compute(pass, "$ngrt4n$salt");
  }
};

#endif // DBSESSION_HPP
