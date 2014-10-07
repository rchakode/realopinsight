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
  ChartBase();
  void setCoreData(CoreDataT* cdata) {m_cdata = cdata;}
  void updateSeverityInfo(void);
  QString buildTooltipText(void);
  double statusRatio(int status) const { return m_severityRatio[status]; }

protected:
  CoreDataT* m_cdata;
  CheckStatusCountT m_statsData;
  qint32 m_nbStatsEntries;
  QMap<int, float> m_severityRatio;
};

#endif // CHARTBASE_HPP
