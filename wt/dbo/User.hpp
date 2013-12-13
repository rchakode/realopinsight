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

#include <Wt/Dbo/Dbo>
#include <string>
#include <list>

namespace dbo = Wt::Dbo;

class User;
namespace Wt {
  namespace Dbo {

    template<>
    struct dbo_traits<User> : public dbo_default_traits
    {
      typedef std::string IdType;
      static IdType invalidId() { return std::string(); }
      static const char *surrogateIdField() { return 0; }
    };
  }
}

class User {
public:

  enum RoleT {
    AdmRole = 100,
    OpRole = 101
  };

  static std::string role2Text(int role) {
    return role == AdmRole? "Administrator" : "Operator";
  }
  std::string username;
  std::string firstname;
  std::string lastname;
  std::string registrationDate;
  int role;

  template<class Action>
  void persist(Action& a)
  {
    dbo::id(a, username, "name");
    dbo::field(a, role, "role");
  }
};

typedef std::list<User> UserListT;

#endif // USER_HPP
