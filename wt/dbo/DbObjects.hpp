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

namespace dbo = Wt::Dbo;

class RoiDboView;
class RoiDboUser;
class RoiDboLoginSession;

namespace Wt {
  namespace Dbo {
    template<>
    struct dbo_traits<RoiDboView> : public dbo_default_traits
    {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char* surrogateIdField() { return 0; }
    };

    template<>
    struct dbo_traits<RoiDboUser> : public dbo_default_traits
    {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char* surrogateIdField() { return 0; }
    };
  }
}

class RoiDboView
{
public:
  std::string name;
  std::string path;
  int service_count;
  dbo::collection< dbo::ptr<RoiDboUser> > users;

  template<class Action>
  void persist(Action& a) {
    dbo::id(a, name, "name");
    dbo::field(a, path, "path");
    dbo::field(a, service_count, "service_count");
    dbo::hasMany(a, users, dbo::ManyToMany, "user_view");
  }
};


class RoiDboUser {
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
  //int authSystem; // e.g. LDAP or BuiltIn
  dbo::collection< dbo::ptr<RoiDboView> > views;
  dbo::collection< dbo::ptr<RoiDboLoginSession> > sessions;

  template<class Action>
  void persist(Action& a) {
    dbo::id(a, username, "name");
    dbo::field(a, firstname, "firstname");
    dbo::field(a, lastname, "lastname");
    dbo::field(a, email, "email");
    dbo::field(a, role, "role");
    dbo::field(a, registrationDate, "registrationDate");
    //dbo::id(a, authSystem, "authSystem");
    dbo::hasMany(a, views, dbo::ManyToMany, "user_view");
    dbo::hasMany(a, sessions, dbo::ManyToMany);
  }

  static std::string role2Text(int role) {
    return role == AdmRole? "Administrator" : "Operator";
  }
  static int role2Int(const std::string& role) {
    return role =="Administrator" ? AdmRole : OpRole;
  }
};


class RoiDboLoginSession
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


typedef std::set<std::string> RoiUserViewsT;
typedef std::list<RoiDboUser> RoiDboUsersT;
typedef std::list<RoiDboView> RoiDboViewsT;
typedef std::list<RoiDboLoginSession> LoginSessionListT;
typedef dbo::collection< dbo::ptr<RoiDboUser> > UserCollectionT;
typedef dbo::collection< dbo::ptr<RoiDboView> > ViewCollectionT;
typedef dbo::collection< dbo::ptr<RoiDboLoginSession> > LoginSessionCollectionT;

#endif // USER_HPP
