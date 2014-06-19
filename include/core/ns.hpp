/*
 * ns.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */


#ifndef NS_HPP_
#define NS_HPP_

#include<stdlib.h>
#include <string>

namespace ngrt4n {
  const std::string APP_NAME = "ngrt4n" ;
  const std::string APP_HOME = std::string(getenv("HOME")) + "/." + APP_NAME ;
  const std::string SETTINGS_FILE =  APP_HOME + "/db" ;
  const std::string AUTH_FILE =  APP_HOME + "/auth" ;
  const std::string salt = "$1$" + APP_NAME + "$";

  void initApp() ;
  void checkUser() ;
  void setPassChain(char* authChain) ;
  std::string getPassChain() ;
  std::string trim(const std::string& str, const std::string& enclosingChar=" \t");
}

#endif /* NS_HPP_ */
