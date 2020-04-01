/*
 * realopinsight-setupdb.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2018 Rodrigue Chakode (rodrigue.chakode@gmail.com)         #
#                                                                          #
# This file is part of RealOpInsight Project (http://RealOpInsight.com)    #
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

#include "dbo/src/DbSession.hpp"
#include "WebBaseSettings.hpp"


int main(int, char **)
{
  int rc = ngrt4n::RcGenericFailure;
  DbSession dbSession;
  if (! dbSession.isReady() || dbSession.initDb() != ngrt4n::RcSuccess) {
    std::cerr << Q_TR("Database initialization failed. Please check the core log for more details");
  } else {
    rc = ngrt4n::RcSuccess;
  }

  return rc;
}
