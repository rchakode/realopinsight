/*
 * User.hpp
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

#ifndef USER_HPP
#define USER_HPP

#include "WebUtils.hpp"
#include <string>
#include <list>
#include <string>
#include <set>
#include <QMap>
#include <QString>
#include <Wt/Dbo/Dbo.h>
#include <Wt/WDateTime.h>

namespace dbo = Wt::Dbo;

class DboView;
class DboUser;
class DboLoginSession;
struct PlatformStatusT;
class DboPlatformStatus;
class DboNotification;
class DboSource;
struct NotificationT;

namespace Wt {
  namespace Dbo {
    template<>
    struct dbo_traits<DboView> : public dbo_default_traits {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char* surrogateIdField() { return nullptr; }
    };

    template<>
    struct dbo_traits<DboUser> : public dbo_default_traits {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char* surrogateIdField() { return nullptr; }
    };

    template<>
    struct dbo_traits<DboSource> : public dbo_default_traits {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char* surrogateIdField() { return nullptr; }
    };
  }
}

class DboView
{
public:
  std::string name;
  std::string path;
  int service_count;
  dbo::collection< dbo::ptr<DboUser> > users;
  dbo::collection< dbo::ptr<DboPlatformStatus> > platformStatusCollection;
  dbo::collection< dbo::ptr<DboNotification> > notifications;

  template<class Action>
  void persist(Action& a) {
    dbo::id(a, name, "name");
    dbo::field(a, path, "path");
    dbo::field(a, service_count, "service_count");
    dbo::hasMany(a, users,dbo::ManyToMany, "user_view", std::string(), dbo::OnDeleteCascade);
    dbo::hasMany(a, platformStatusCollection, dbo::ManyToOne, "view");
    dbo::hasMany(a, notifications, dbo::ManyToOne, "view");
  }
};

struct DboUserT {
  std::string username;
  std::string password;
  std::string firstname;
  std::string lastname;
  std::string email;
  int role;
  int authsystem;
  int dashboardDisplayMode;
  int dashboardTilesPerRow;
  std::string registrationDate;
};

class DboUser {
public:
  enum RoleT {
    AdmRole = 100,
    OpRole = 101
  };

  enum DashboardModeT {
    CompleteDashboard = 0,
    NoReportDashboard = 1,
    TileDashboard = 2
  };

  std::string username;
  std::string password;
  std::string firstname;
  std::string lastname;
  std::string email;
  int role;
  std::string registrationDate;
  int authsystem;
  int dashboardDisplayMode;
  int dashboardTilesPerRow;

  dbo::collection< dbo::ptr<DboView> > views;
  dbo::collection< dbo::ptr<DboNotification> > ack_notifications;

  template<class Action>
  void persist(Action& a) {
    dbo::id(a, username, "name");
    dbo::field(a, firstname, "firstname");
    dbo::field(a, lastname, "lastname");
    dbo::field(a, email, "email");
    dbo::field(a, role, "role");
    dbo::field(a, registrationDate, "registrationDate");
    dbo::field(a, authsystem, "authsystem");
    dbo::field(a, dashboardDisplayMode, "dashboardDisplayMode");
    dbo::field(a, dashboardTilesPerRow, "dashboardTilesPerRow");
    dbo::hasMany(a, views, dbo::ManyToMany, "user_view", std::string(), dbo::OnDeleteCascade);
    dbo::hasMany(a, ack_notifications, dbo::ManyToOne, "ack_user");
  }

  void setData(const DboUserT& userInfo) {
    username = userInfo.username;
    password = userInfo.password;
    firstname = userInfo.firstname;
    lastname = userInfo.lastname;
    email = userInfo.email;
    role = userInfo.role;
    registrationDate = userInfo.registrationDate;
    authsystem = userInfo.authsystem;
    dashboardDisplayMode = userInfo.dashboardDisplayMode;
    dashboardTilesPerRow = userInfo.dashboardTilesPerRow;
  }

  DboUserT data(void) const {
    DboUserT u;
    u.username = username;
    u.password = password;
    u.firstname = firstname;
    u.lastname = lastname;
    u.email = email;
    u.role = role;
    u.registrationDate = registrationDate;
    u.authsystem = authsystem;
    u.dashboardDisplayMode = dashboardDisplayMode;
    u.dashboardTilesPerRow = dashboardTilesPerRow;
    return u;
  }

  static std::string role2Text(int role) {
    return role == AdmRole? "Administrator" : "Operator";
  }
  static int role2Int(const std::string& role) {
    return role == "Administrator" ? AdmRole : OpRole;
  }
  static std::string dashboardMode2Text(int mode) {
    std::string result = Q_TR("Default");
    switch(mode) {
      case TileDashboard:
        result = Q_TR("Only Tiles");
        break;
      case NoReportDashboard:
        result = Q_TR("No Report");
        break;
      case CompleteDashboard:
      default:
        result = Q_TR("Complete");
        break;
    }

    return result;

  }
};

/** holds platform status data without wt::dbo specific info */
struct PlatformStatusT {
  long timestamp;
  int status;
  float normal;
  float minor;
  float major;
  float critical;
  float unknown;
  std::string view_name;

  PlatformStatusT() : status(ngrt4n::Unknown) {}

  std::string toString(void) const {
    return QString("%1,%2,%3,%4,%5,%6,%7,%8")
        .arg(QString::number(timestamp),
             view_name.c_str(),
             QString::number(status),
             QString::number(static_cast<double>(normal)),
             QString::number(static_cast<double>(minor)),
             QString::number(static_cast<double>(major)),
             QString::number(static_cast<double>(critical)),
             QString::number(static_cast<double>(unknown))).toStdString();
  }
};


/** holds platform status like wt::dbo class */
class DboPlatformStatus {
public:
  long timestamp;
  int status;
  float normal;
  float minor;
  float major;
  float critical;
  float unknown;
  dbo::ptr<DboView> view;

  void setData(const PlatformStatusT& data)
  {
    timestamp = data.timestamp;
    status = data.status;
    normal = data.normal;
    minor = data.minor;
    major = data.major;
    critical = data.critical;
    unknown = data.unknown;
  }

  PlatformStatusT data(void) const
  {
    PlatformStatusT d;
    d.timestamp = timestamp;
    d.status = status;
    d.normal = normal;
    d.minor = minor;
    d.major = major;
    d.critical = critical;
    d.unknown = unknown;
    d.view_name = view ? view->name : "";
    return d;
  }

  template<class Action>
  void persist(Action& a) {
    dbo::field(a, timestamp, "timestamp");
    dbo::field(a, status, "status");
    dbo::field(a, normal, "normal");
    dbo::field(a, minor, "minor");
    dbo::field(a, major, "major");
    dbo::field(a, critical, "critical");
    dbo::field(a, unknown, "unknown");
    dbo::belongsTo(a, view, "view", dbo::OnDeleteCascade);
  }

  std::string toString(void) const {
    return QString("%1,%2,%3,%4,%5,%6,%7, %8")
        .arg(QString::number(timestamp),
             view->name.c_str(),
             QString::number(status),
             QString::number(static_cast<double>(normal)),
             QString::number(static_cast<double>(minor)),
             QString::number(static_cast<double>(major)),
             QString::number(static_cast<double>(critical)),
             QString::number(static_cast<double>(unknown))).toStdString();
  }
};

class DboLoginSession
{
public:
  enum {
    ExpiredCookie = 0,
    ActiveCookie = 1,
    InvalidSession = 2
  };

  std::string username;
  std::string sessionId;
  std::string firstAccess;
  std::string lastAccess;
  int status;

  template<class Action>
  void persist(Action& a) {
    dbo::field(a, username, "username");
    dbo::field(a, sessionId, "session_id");
    dbo::field(a, firstAccess, "first_access");
    dbo::field(a, lastAccess, "last_access");
    dbo::field(a, status, "status");
  }
};

/** holds notification info without wt::dbo specific properties (e.g dbo pointers)*/
struct NotificationT {
  long timestamp;
  long last_change;
  long ack_status;
  int view_status;
  std::string view_name;
  std::string ack_username;
  NotificationT():
    timestamp(0),
    last_change(0),
    ack_status(0),
    view_status(-1) { }
};

/** holds notification info like a wt::dbo object */
class DboNotification
{
public:
  enum AckStatusT {
    Unset = -1,
    Closed = 0,
    Open = 1,
    Acknowledged = 2
  };

  long timestamp;
  int view_status;
  int ack_status;
  long last_change;
  dbo::ptr<DboView> view;
  dbo::ptr<DboUser> ack_user;

  NotificationT data(void) const
  {
    NotificationT d;
    d.timestamp = timestamp;
    d.view_name = view? view->name : "";
    d.view_status = view_status;
    d.ack_status = ack_status;
    d.last_change = last_change;
    d.ack_username = ack_user ? ack_user->username : "";
    return d;
  }

  template<class Action>
  void persist(Action& a) {
    dbo::field(a, timestamp, "timestamp");
    dbo::field(a, view_status, "view_status");
    dbo::field(a, ack_status, "ack_status");
    dbo::field(a, last_change, "last_change");
    dbo::belongsTo(a, view, "view", dbo::OnDeleteCascade);
    dbo::belongsTo(a, ack_user, "ack_user", dbo::OnDeleteCascade);
  }
};



/** holds source info like a wt::dbo object */
class DboSource
{
public:
  std::string id;
  int mon_type;
  std::string mon_url;
  std::string ls_addr;
  int ls_port;
  std::string auth;
  int verify_ssl_peer;
  std::string icon;

  template<class Action>
  void persist(Action& a) {
    dbo::id(a, id, "id");
    dbo::field(a, mon_type, "monitor_type");
    dbo::field(a, mon_url, "monitor_url");
    dbo::field(a, icon, "icon");
    dbo::field(a, ls_addr, "livestatus_addr");
    dbo::field(a, ls_port, "livestatus_port");
    dbo::field(a, auth, "auth_string");
    dbo::field(a, verify_ssl_peer, "verify_ssl_peer");
  }
};


typedef std::set<std::string> UserViewsT;
typedef std::list<DboUser> DbUsersT;
typedef std::list<DboView> DbViewsT;
typedef std::list<DboLoginSession> LoginSessionListT;
typedef std::list<PlatformStatusT> PlatformStatusList;
typedef QMap<std::string, PlatformStatusList > PlatformStatusListMapT;
typedef QMap<std::string, NotificationT> NotificationMapT;
typedef dbo::collection< dbo::ptr<DboUser> > DboUserCollectionT;
typedef dbo::collection< dbo::ptr<DboView> > DboViewCollectionT;
typedef dbo::collection< dbo::ptr<DboPlatformStatus> > DboPlatformStatusCollectionT;
typedef dbo::collection< dbo::ptr<DboNotification> > DboNotificationCollectionT;
typedef dbo::collection< dbo::ptr<DboLoginSession> > DboLoginSessionCollectionT;
typedef dbo::collection< dbo::ptr<DboSource> > DboSourceCollectionT;

#endif // USER_HPP
