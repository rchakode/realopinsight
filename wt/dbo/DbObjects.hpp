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

#include <string>
#include <list>
#include <Wt/Dbo/Dbo>
#include <string>
#include <set>
#include <Wt/WDateTime>
#include <QString>

namespace dbo = Wt::Dbo;

class DbViewT;
class DbUserT;
class DbLoginSession;
class DbQosInfoT;

namespace Wt {
  namespace Dbo {
    template<>
    struct dbo_traits<DbViewT> : public dbo_default_traits
    {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char* surrogateIdField() { return 0; }
    };

    template<>
    struct dbo_traits<DbUserT> : public dbo_default_traits
    {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char* surrogateIdField() { return 0; }
    };
  }
}

class DbViewT
{
public:
  std::string name;
  std::string path;
  int service_count;
  dbo::collection< dbo::ptr<DbUserT> > users;
  dbo::collection< dbo::ptr<DbQosInfoT> > qosreports;

  template<class Action>
  void persist(Action& a) {
    dbo::id(a, name, "name");
    dbo::field(a, path, "path");
    dbo::field(a, service_count, "service_count");
    dbo::hasMany(a, users, dbo::ManyToMany, "user_view");
    dbo::hasMany(a, qosreports, dbo::ManyToOne, "view");
  }
};


class DbUserT {
public:
  enum RoleT {
    AdmRole = 100,
    OpRole = 101
  };

  std::string username;
  std::string password;
  std::string firstname;
  std::string lastname;
  std::string email;
  int role;
  std::string registrationDate;
  int authsystem; // e.g. LDAP or BuiltIn
  dbo::collection< dbo::ptr<DbViewT> > views;
  dbo::collection< dbo::ptr<DbLoginSession> > sessions;

  template<class Action>
  void persist(Action& a) {
    dbo::id(a, username, "name");
    dbo::field(a, firstname, "firstname");
    dbo::field(a, lastname, "lastname");
    dbo::field(a, email, "email");
    dbo::field(a, role, "role");
    dbo::field(a, registrationDate, "registrationDate");
    dbo::field(a, authsystem, "authsystem");
    dbo::hasMany(a, views, dbo::ManyToMany, "user_view");
    dbo::hasMany(a, sessions, dbo::ManyToMany);
  }

  static std::string role2Text(int role) {
    return role == AdmRole? "Administrator" : "Operator";
  }
  static int role2Int(const std::string& role) {
    return role == "Administrator" ? AdmRole : OpRole;
  }
};

class DbQosInfoT {

public:
  long timestamp;
  int status;
  float normal;
  float minor;
  float major;
  float critical;
  float unknown;
  dbo::ptr<DbViewT> view;
  std::string viewname; // not persisted, duplication for non db mode

  template<class Action>
  void persist(Action& a) {
    dbo::field(a, timestamp, "timestamp");
    dbo::field(a, status, "status");
    dbo::field(a, normal, "normal");
    dbo::field(a, minor, "minor");
    dbo::field(a, major, "major");
    dbo::field(a, critical, "critical");
    dbo::field(a, unknown, "unknown");
    dbo::belongsTo(a, view, "view");
  }

  std::string toString(void) const {
    return QString("%1,%2,%3,%4,%5,%6,%7")
        .arg(viewname.c_str(),
             QString::number(status),
             QString::number(normal),
             QString::number(minor),
             QString::number(major),
             QString::number(critical),
             QString::number(unknown)).toStdString();
  }
};

class DbLoginSession
{
public:
  enum {
    ExpiredCookie = 0,
    ActiveCookie = 1,
    InvalidSession = 2
  };

  std::string username;
  std::string sessionId;
  Wt::WDateTime firstAccess;
  Wt::WDateTime lastAccess;
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


typedef std::set<std::string> UserViewsT;
typedef std::list<DbUserT> DbUsersT;
typedef std::list<DbViewT> DbViewsT;
typedef std::list<DbLoginSession> LoginSessionListT;
typedef std::list<DbQosInfoT> DbQosInfosT;
typedef dbo::collection< dbo::ptr<DbUserT> > UserCollectionT;
typedef dbo::collection< dbo::ptr<DbViewT> > ViewCollectionT;
typedef dbo::collection< dbo::ptr<DbQosInfoT> > QosInfoCollectionT;
typedef dbo::collection< dbo::ptr<DbLoginSession> > LoginSessionCollectionT;

#endif // USER_HPP
