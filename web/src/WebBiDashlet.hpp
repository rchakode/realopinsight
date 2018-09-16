/*
 * WebBiDashlet.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Creation: 17-07-2015                                                     #
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

#ifndef WEBBIDASHLET_HPP
#define WEBBIDASHLET_HPP

#include "WebBiSlaDataAggregator.hpp"
#include "WebBiRawChart.hpp"
#include "WebPieChart.hpp"
#include "WebBiDateFilter.hpp"
#include "WebCsvReportResource.hpp"
#include <Wt/WContainerWidget>
#include <Wt/WHBoxLayout>
#include <Wt/WDatePicker>



class WebBiDashlet : public QObject, public Wt::WContainerWidget
{
  Q_OBJECT

public:
  WebBiDashlet();
  ~WebBiDashlet();
  void initialize(const DbViewsT& viewList);
  void updateChartsByViewName (const std::string& viewName, const QosDataListMapT& qosDataMap);
  long startTime(void) {return m_filterHeader.epochStartTime();}
  long endTime(void) {return m_filterHeader.epochEndTime();}

public Q_SLOTS:
  void handleReportPeriodChanged(long start, long end) { Q_EMIT reportPeriodChanged(start, end);}

Q_SIGNALS:
  void reportPeriodChanged(long start, long end);


private:
  Wt::WGridLayout* m_layout;
  WebBiDateFilter m_filterHeader;
  QMap<std::string, Wt::WText*> m_slaChartTitles;
  QMap<std::string, WebPieChart*> m_slaPieCharts;
  QMap<std::string, WebBiRawChart*> m_itProblemCharts;
  QMap<std::string, WebCsvExportIcon*> m_csvExportLinks;
  QMap<std::string, std::string> m_viewDashboardAliasNames;

  void addEvent(void);
  Wt::WText* createTitleWidget(const std::string& viewName);
};

#endif // WEBBIDASHLET_HPP
