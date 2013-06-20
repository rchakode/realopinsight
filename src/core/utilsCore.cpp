/*
 * utilsCore.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#	                                                                         #
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

#include "ns.hpp"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <exception>
#include <string>
#include <iostream>
#include <unistd.h>

using namespace std ;

void ngrt4n::initApp()
{
  int ret = mkdir(ngrt4n::APP_HOME.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) ;

  if(ret == -1 && errno != EEXIST) {
      cerr << "You need to set the authentication token first" ;
    }
}


void ngrt4n::checkUser() {
  if( getuid() != 0) {
      cerr << "The program must be run as root" << endl;
      exit(1) ;
    }
}

string ngrt4n::trim(const string& str, const string& enclosingChar)
{
  size_t first = str.find_first_not_of(enclosingChar);

  if (first != string::npos) {
      size_t last = str.find_last_not_of(enclosingChar);

      return str.substr(first, last - first + 1);
    }

  return "";
}

