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

#include "WebBiSlaChart.hpp"
#include "WebBiRawChart.hpp"
#include "WebPieChart.hpp"
#include "WebBiDateFilter.hpp"
#include "WebCsvReportResource.hpp"
#include <Wt/WContainerWidget>
#include <Wt/WHBoxLayout>
#include <Wt/WDatePicker>



class WebBiDashlet : public Wt::WContainerWidget
{
  WebBiDashlet();
  ~WebBiDashlet();
  void initialize(const DbViewsT& viewList);
  void update(const QosDataByViewMapT& qosDataMap, const std::string& viewName);

private:
  Wt::WGridLayout* m_layout;
  WebBiDateFilter m_filter;
  QMap<std::string, Wt::WText*> m_chartTitleMap;
  QMap<std::string, WebPieChart*> m_qosChartMap;
  QMap<std::string, WebBiRawChart*> m_itProblemChartMap;
  QMap<std::string, WebCsvExportIcon*> m_csvExportLinkMap;

  Wt::WText* createTitleWidget(const std::string& viewName);
};

#endif // WEBBIDASHLET_HPP
