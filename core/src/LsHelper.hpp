/*
 * LsHelper.hpp
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

#ifndef MKLSHELPER_HPP
#define MKLSHELPER_HPP

#include "Base.hpp"
#include "RawSocket.hpp"

class LsHelper
{
public:
  enum ReqTypeT{
    Host = 0,
    Service = 1
  };

  LsHelper(const QString& host, uint16_t port);
  ~LsHelper();

  int makeRequest(const QByteArray& data, ChecksT& checks);
  int loadChecks(const QString& hostgroupFilter, ChecksT& checks);
  QString lastError(void) const {return m_socketHandler->lastError();}
  int setupSocket(void);

  void parseResult(ChecksT& checks);
  static QByteArray prepareRequestData(ReqTypeT requestType);

private:
  RawSocket* m_socketHandler;
  std::string m_hostOrGroupFilter;
};

#endif // MKLSHELPER_HPP
