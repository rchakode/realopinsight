/*
 * ChartBase.hpp
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

#ifndef CHARTBASE_HPP
#define CHARTBASE_HPP

#include "Base.hpp"
#include <QMap>

class ChartBase
{
public:
  enum {
    RawData = 0,
    SLAData = 1
  };
  ChartBase();
  void updateStatsRatio(void);
  std::string defaultTooltipText(void);
  double statusRatio(int status) const { return static_cast<double>(m_severityRatio[status]); }
  std::string toStdString(void) {return defaultTooltipText();}
  void updateStatsData(const CheckStatusCountT& statsData, int count);
  void setSeverityData(double normal, double minor, double major, double critical, double total);
  void setDataType(int dataType) {m_dataType = dataType;}
  QString timeFromSeconds(long seconds);

  int problemCount(void) {
    return m_statsData[ngrt4n::Minor]
        + m_statsData[ngrt4n::Major]
        + m_statsData[ngrt4n::Critical]
        + m_statsData[ngrt4n::Unknown];
  }

  std::string problemsDetailsText(void) {
    auto&& nbProblems = problemCount();
    return QObject::tr("%1 problem%2/%3 probe%4")
        .arg(QString::number(nbProblems))
        .arg(nbProblems > 1? "s": "")
        .arg(QString::number(m_dataCount))
        .arg(m_dataCount > 1? "s": "")
        .toStdString();
  }

protected:
  CheckStatusCountT m_statsData;
  qint32 m_dataCount;
  QMap<int, float> m_severityRatio;
  int m_dataType;
};

#endif // CHARTBASE_HPP
