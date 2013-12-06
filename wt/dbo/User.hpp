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
  std::string username;
  std::string password;
  int role;

  template<class Action>
  void persist(Action& a)
  {
    dbo::id(a, username, "username");
    dbo::field(a, password, "password");
    dbo::field(a, role, "role");
  }
};


#endif // USER_HPP
