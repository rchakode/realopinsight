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
  : m_dataType(RawData)
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


std::string ChartBase::tooltipText(void)
{
  QString normalText = "";
  QString minorText = "";
  QString majorText = "";
  QString criticalText = "";
  QString unknownText = "";
  QString totalText = "";
  if (m_dataType == SLAData) {
    normalText   = timeFromSeconds(m_statsData[ngrt4n::Normal]);
    minorText    = timeFromSeconds(m_statsData[ngrt4n::Minor]);
    majorText    = timeFromSeconds(m_statsData[ngrt4n::Major]);
    criticalText = timeFromSeconds(m_statsData[ngrt4n::Critical]);
    unknownText  = timeFromSeconds(m_statsData[ngrt4n::Unknown]);
  } else {
    normalText   = QString::number(m_statsData[ngrt4n::Normal]);
    minorText    = QString::number(m_statsData[ngrt4n::Minor]);
    majorText    = QString::number(m_statsData[ngrt4n::Major]);
    criticalText = QString::number(m_statsData[ngrt4n::Critical]);
    unknownText  = QString::number(m_statsData[ngrt4n::Unknown]);
    totalText    = "/"+QString::number(m_dataCount);
  }
  QString tooltip =
      QString("Normal: %1% - %2%9\n"
              "Minor: %3%  - %4%9\n"
              "Major: %5%  - %6%9\n"
              "Critical: %7%  - %8%9\n").arg
      (QString::number(m_severityRatio[ngrt4n::Normal],'f',0),
      normalText,
      QString::number(m_severityRatio[ngrt4n::Minor],'f',0),
      minorText,
      QString::number(m_severityRatio[ngrt4n::Major],'f',0),
      majorText,
      QString::number(m_severityRatio[ngrt4n::Critical],'f',0),
      criticalText,
      totalText);

  return tooltip.append("Unknown: %1% - %2%3").arg(
        QString::number(m_severityRatio[ngrt4n::Unknown],'f',0),
      unknownText, totalText).toStdString();
}

void ChartBase::updateStatsData(const CheckStatusCountT& statsData, int count)
{
  m_statsData = statsData;
  m_dataCount = count;
  updateStatsRatio();
}


void ChartBase::setSeverityData(double normal, double minor, double major, double critical, double total)
{
  m_dataCount = total;
  m_statsData[ngrt4n::Normal]   = normal;
  m_statsData[ngrt4n::Minor]    = minor;
  m_statsData[ngrt4n::Major]    = major;
  m_statsData[ngrt4n::Critical] = critical;
  m_statsData[ngrt4n::Unknown]  = total  - (normal + minor + major + critical);
  updateStatsRatio();
}

QString ChartBase::timeFromSeconds(long seconds)
{
  QTime time;
  time.addSecs(seconds);
  QString hour = QString::number(time.hour());
  QString min = QString::number(time.minute());
  QString sec = QString::number(time.second());
  return QString("%1:%2:%3").arg((hour.size() < 2 ? "0"+hour : hour),
                                 (min.size() < 2 ? "0"+min : min),
                                 (sec.size() < 2 ? "0"+sec : sec));
}
