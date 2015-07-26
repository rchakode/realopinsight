/*
 * WebBiSlaChart.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Creation: 26-07-2015                                                     #
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

#ifndef WEBBISLACHART_HPP
#define WEBBISLACHART_HPP


class WebBiSlaChart
{
public:
  WebBiSlaChart(const QosDataList& data);

  double normalDuration(void) const {return m_normalDuration;}
  double minorDuration(void) const {return m_minorDuration;}
  double majorDuration(void) const {return m_majorDuration;}
  double criticalDuration(void) const {return m_criticalDuration;}
  double unknownDuration(void) const {return m_unknownDuration;}
  double totalDuration(void) const {return m_totalDuration;}

private:
  struct TimeStatusT {
    long timestamp;
    int status;
  };
  typedef QList<TimeStatusT> TimeStatusesT;
  TimeStatusesT m_plottingData;
  long m_normalDuration;
  long m_minorDuration;
  long m_majorDuration;
  long m_criticalDuration;
  long m_unknownDuration;
  long m_totalDuration;

  void processData(const QosDataList& data);
  //FIXME: std::string slaText(void) {return QObject::tr("SLA: %1\%").arg(QString::number(m_slaNormal,'f',2)).toStdString();}
};

#endif // WEBBISLACHART_HPP
