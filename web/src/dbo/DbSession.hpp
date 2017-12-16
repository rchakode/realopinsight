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

#include "DbObjects.hpp"
#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Dbo/backend/Postgres>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/User>
#include <Wt/WGlobal>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/Auth/Dbo/UserDatabase>
#include <Wt/Auth/Login>
#include <climits>
#include <semaphore.h>

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
  DbSession(int dbType, const std::string& db);
  ~DbSession();
  void setupDb(void);
  std::string lastError(void) const {return m_lastError;}
  Wt::Auth::AbstractUserDatabase& users() const {return *m_dboUserDb;}
  Wt::Auth::AuthService& auth();
  Wt::Auth::PasswordService* passwordAuthentificator(void);
  Wt::Auth::Login& loginObject(void);
  bool isLogged(void) {return loginObject().loggedIn();}
  bool isLoggedAdmin(void) {return loggedUser().role == DboUser::AdmRole;}
  void configureAuth(void);
  const DboUser& loggedUser(void) const {return m_loggedUser;}
  bool isCompleteUserDashboard(void) const {return loggedUser().dashboardDisplayMode == DboUser::CompleteDashboard;}
  bool displayOnlyTiles(void) const {return loggedUser().dashboardDisplayMode == DboUser::TileDashboard;}
  bool isReportUserDashboard(void) const {return loggedUser().dashboardDisplayMode == DboUser::NoReportDashboard;}
  int dashboardTilesPerRow(void) const {return (loggedUser().dashboardTilesPerRow <= 0 ? 5 : loggedUser().dashboardTilesPerRow);}
  void setLoggedUser(void);
  QString loggedUserName(void)const {return QString::fromStdString(loggedUser().username);}

  int addUser(const DboUserT& userInfo);
  int updateUser(const DboUserT& userInfo);
  int deleteUser(const std::string& username);
  int deleteAuthSystemUsers(int authSystem);
  bool findUser(const std::string& username, DboUserT& user);
  int updatePassword(const std::string& uname, const std::string& currentPass, const std::string& newPass);
  void updateUserList(void);
  DbUsersT& userList(void) {return m_userList;}

  int addView(const DboView& qos);
  int deleteView(std::string viewId);
  int assignView(const std::string& userId, const std::string& viewId);
  int revokeView(const std::string& userId, const std::string& viewId);
  int listAssignedUsersEmails(QStringList& emails, const std::string& viewId);

  int addQosData(const QosDataT& qosData);
  int addQosDataList(const QosDataList& qosDataList);
  int listQosData(QosDataListMapT& qosDataMap, const std::string& viewId, long fromDate = 0, long toDate = LONG_MAX);
  int getLastQosData(QosDataT& qosData, const std::string& viewId);

  void updateViewList(void);
  void updateViewList(const std::string& uname);
  DbViewsT& viewList(void) {return m_viewList;}
  int viewCount(void) const {return m_viewList.size();}
  bool findView(const std::string& vname, DboView& view);

  void updateUserViewList(void);
  UserViewsT& userViewList(void) {return m_userViewList;}

  int addSession(const DboLoginSession& session);
  int checkUserCookie(const DboLoginSession& session);

  int addNotification(const std::string& viewId, int viewStatus);
  int updateNotificationAckStatusForUser(const std::string& userId, const std::string& viewId, int newAckStatus);
  int updateNotificationAckStatusForView(const std::string& userId, const std::string& viewId, int newAckStatus);
  void getLastNotificationInfo(NotificationT& lastNotifInfo, const std::string& viewId);
  int listViewRelatedNotifications(NotificationMapT& notifications, const std::string& userId);

private:
  dbo::SqlConnection* m_dboSqlConncetion;


  UserDatabase* m_dboUserDb;
  DboUser m_loggedUser;
  DbUsersT m_userList;
  DbViewsT m_viewList;
  UserViewsT m_userViewList;
  std::string m_lastError;
  Wt::Auth::Login m_loginObj;
  Wt::Auth::AuthService m_basicAuthService;
  Wt::Auth::PasswordService* m_passAuthService;

  std::string hashPassword(const std::string& pass);
  void initDb(void);
};

#endif // DBSESSION_HPP
