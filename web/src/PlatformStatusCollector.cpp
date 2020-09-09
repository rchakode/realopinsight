/*
 * ReportCollector.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 06-10-2014                                                  #
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

#include "PlatformStatusCollector.hpp"
#include "ctime"

PlatformStatusCollector::PlatformStatusCollector(void)
  : DashboardBase(nullptr)
{
}


void PlatformStatusCollector::updateChart(void)
{
  CheckStatusCountT statsData;
  int statCount = extractStatsData(statsData);
  m_chartBase.updateStatsData(statsData, statCount);

  NodeT rootSrv = rootNode();
  m_info.timestamp = time(nullptr);
  m_info.view_name = rootSrv.name.toStdString();
  m_info.status    = rootSrv.sev;
  m_info.normal    = static_cast<float>(m_chartBase.statusRatio(ngrt4n::Normal));
  m_info.minor     = static_cast<float>(m_chartBase.statusRatio(ngrt4n::Minor));
  m_info.major     = static_cast<float>(m_chartBase.statusRatio(ngrt4n::Major));
  m_info.critical  = static_cast<float>(m_chartBase.statusRatio(ngrt4n::Critical));
  m_info.unknown   = static_cast<float>(m_chartBase.statusRatio(ngrt4n::Unknown));
}
