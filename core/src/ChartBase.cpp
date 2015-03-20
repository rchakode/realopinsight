/*
 * ChartBase.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 27-04-2014                                                 #
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

#include "ChartBase.hpp"

ChartBase::ChartBase()
{
}

void ChartBase::updateStatsRatio(void)
{
  m_severityRatio.clear();
  m_severityRatio[ngrt4n::Unknown] = qRound((100.0 * m_statsData[ngrt4n::Unknown])/m_dataCount);
  m_severityRatio[ngrt4n::Critical] = qRound((100.0 * m_statsData[ngrt4n::Critical])/m_dataCount);
  m_severityRatio[ngrt4n::Major] = qRound((100.0 * m_statsData[ngrt4n::Major])/m_dataCount);
  m_severityRatio[ngrt4n::Minor] = qRound((100.0 * m_statsData[ngrt4n::Minor])/m_dataCount);
  m_severityRatio[ngrt4n::Normal] = qRound((100.0 * m_statsData[ngrt4n::Normal])/m_dataCount);
}


QString ChartBase::buildTooltipText(void)
{
  return QObject::tr("Normal: ")%QString::number(m_statsData[ngrt4n::Normal])%
      "/"%QString::number(m_dataCount)%" ("%QString::number(m_severityRatio[ngrt4n::Normal], 'f', 0)%"%)"
      %"\n"%QObject::tr("Minor: ")%QString::number(m_statsData[ngrt4n::Minor])%
      "/"%QString::number(m_dataCount)%" ("%QString::number(m_severityRatio[ngrt4n::Minor], 'f', 0)%"%)"
      %"\n"%QObject::tr("Major: ")%QString::number(m_statsData[ngrt4n::Major])%
      "/"%QString::number(m_dataCount)%" ("%QString::number(m_severityRatio[ngrt4n::Major], 'f', 0)%"%)"
      %"\n"%QObject::tr("Critical: ")%QString::number(m_statsData[ngrt4n::Critical])%"/"
      %QString::number(m_dataCount)%" ("%QString::number(m_severityRatio[ngrt4n::Critical], 'f', 0) %"%)"
      %"\n"%QObject::tr("Unknown: ")%QString::number(m_statsData[ngrt4n::Unknown])%
      "/"%QString::number(m_dataCount)%" ("%QString::number(m_severityRatio[ngrt4n::Unknown], 'f', 0)%"%)";
}

void ChartBase::updateStatsData(const CheckStatusCountT& statsData, int count)
{
  m_statsData = statsData;
  m_dataCount = count;
  updateStatsRatio();
}
