/*
 * WebBiSlaChart.cpp
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


#include "WebBiSlaChart.hpp"
#include "WebUtils.hpp"
#include <QDebug>
#include <Wt/WRectArea>


namespace {
  const double BI_RAW_CHART_AREA_WIDTH = 400;
  const double BI_RAW_CHART_AREA_HEIGHT = 150;
  const double BI_QOS_CHART_AREA_WIDTH = 250;
  const double BI_QOS_CHART_AREA_HEIGHT = 150;
  const double BI_CHART_AREA_MARGIN = 25;
  const double BI_CHART_WIDTH = BI_QOS_CHART_AREA_WIDTH;
  const double BI_CHART_TREND_HEIGHT = 50;
  const double AREA_TOP_CORNER_Y = BI_QOS_CHART_AREA_HEIGHT - BI_CHART_AREA_MARGIN - BI_CHART_TREND_HEIGHT;
  const double TEXT_TOP_CORNER_Y = AREA_TOP_CORNER_Y - 5;
  const Wt::WColor LEGEND_TEXT_COLOR = Wt::WColor(0, 0, 0); // black
}



WebBiSlaChart::WebBiSlaChart(const std::list<QosDataT>& data)
  : m_normalDuration(0),
    m_minorDuration(0),
    m_majorDuration(0),
    m_criticalDuration(0),
    m_unknownDuration(0)
{
  processData(data);
}

void WebBiSlaChart::processData(const QosDataList& data)
{
  QosDataList::const_iterator qosit = data.begin();
  m_plottingData.clear();
  m_normalDuration   = 0;
  m_minorDuration    = 0;
  m_majorDuration    = 0;
  m_criticalDuration = 0;
  m_unknownDuration  = 0;
  m_totalDuration    = 1;

  if (! data.empty()) {
    TimeStatusT last = {qosit->timestamp, qosit->status};
    m_plottingData.push_back(last);
    while (++qosit, qosit != data.end()) {
      TimeStatusT current = {qosit->timestamp, qosit->status};
      m_plottingData.push_back(current);
      switch(last.status) {
        case ngrt4n::Normal:
          m_normalDuration += current.timestamp - last.timestamp;
          break;
        case ngrt4n::Minor:
          m_minorDuration += current.timestamp - last.timestamp;
          break;
        case ngrt4n::Major:
          m_majorDuration += current.timestamp - last.timestamp;
          break;
        case ngrt4n::Critical:
          m_criticalDuration += current.timestamp - last.timestamp;
          break;
        case ngrt4n::Unknown:
          m_unknownDuration += current.timestamp - last.timestamp;
        default:
          break;
      }
      last = m_plottingData.back();
    }
    m_totalDuration = (m_plottingData.back().timestamp - m_plottingData.front().timestamp);
  }
}
