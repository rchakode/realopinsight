/*
 * User.hpp
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

#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <list>
#include <Wt/Dbo/Dbo>
#include <string>
#include <set>

namespace dbo = Wt::Dbo;

class View;
class User;

namespace Wt {
  namespace Dbo {
    template<>
    struct dbo_traits<View> : public dbo_default_traits
    {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char* surrogateIdField() { return 0; }
    };

    template<>
    struct dbo_traits<User> : public dbo_default_traits
    {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char* surrogateIdField() { return 0; }
    };
  }
}

class View
{
public:
  std::string name;
  std::string path;
  int service_count;
  dbo::collection< dbo::ptr<User> > users;

  template<class Action>
  void persist(Action& a) {
    dbo::id(a, name, "name");
    dbo::field(a, path, "path");
    dbo::field(a, service_count, "service_count");
    dbo::hasMany(a, users, dbo::ManyToMany, "user_view");
  }
};


class User {
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
  dbo::collection< dbo::ptr<View> > views;

  template<class Action>
  void persist(Action& a) {
    dbo::id(a, username, "name");
    dbo::field(a, firstname, "firstname");
    dbo::field(a, lastname, "lastname");
    dbo::field(a, email, "email");
    dbo::field(a, role, "role");
    dbo::field(a, registrationDate, "registrationDate");
    dbo::hasMany(a, views, dbo::ManyToMany, "user_view");
  }

  static std::string role2Text(int role) {
    return role == AdmRole? "Administrator" : "Operator";
  }
  static int role2Int(const std::string& role) {
    return role =="Administrator" ? AdmRole : OpRole;
  }
};

typedef std::set<std::string> UserViewListT;
typedef std::list<User> UserListT;
typedef std::list<View> ViewListT;
typedef dbo::collection< dbo::ptr<User> > UserCollectionT;
typedef dbo::collection< dbo::ptr<View> > ViewCollectionT;

#endif // USER_HPP
