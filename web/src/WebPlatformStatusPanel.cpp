/*
 * WebBiDashlet.cpp
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

#include "WebPlatformStatusPanel.hpp"
#include "WebPlatformStatusRaw.hpp"
#include <ctime>
#include <regex>


WebPlatformStatusPanel::WebPlatformStatusPanel(const DbViewsT& listOfViews)
  : m_layoutRef(nullptr),
    m_dateFilterRef(nullptr)
{
  auto layout = std::make_unique<Wt::WGridLayout>();
  m_layoutRef = layout.get();

  auto dateFilter = std::make_unique<WebPlatformStatusDateFilter>();
  m_dateFilterRef = dateFilter.get();
  m_dateFilterRef->reportPeriodChanged().connect(std::bind([=]{m_reportPeriodChanged.emit(this->startTime(), this->endTime());}));
  layout->addWidget(std::move(dateFilter), 0, 0, 1, 2, Wt::AlignmentFlag::Center);

  int row = 0;
  for (const auto& view : listOfViews) {
    auto dname = view.name;
    std::smatch regexMatch;
    if (std::regex_match(view.name, regexMatch, std::regex("Source[0-9]:(.+)"))) {
      dname = regexMatch[1].str();
      m_viewDashboardAliasNames[dname] = view.name;
    } else {
      m_viewDashboardAliasNames[view.name] = view.name;
    }

    auto slaReportTitle = std::make_unique<Wt::WText>(Wt::WString("<legend>{1}</legend><hr />").arg(dname), Wt::TextFormat::XHTML);
    m_slaReportTitlesRef.insert(dname, slaReportTitle.get());

    auto csvExportIcon = std::make_unique<WebCsvExportIcon>();
    m_csvLinksRef.insert(dname, csvExportIcon.get());

    auto  problemReport = std::make_unique<WebPlatformStatusRaw>(dname);
    m_problemReportsRef.insert(dname, problemReport.get());

    auto slaReport = std::make_unique<WebPieChart>(ChartBase::SLAData);
    m_slaReportsRef.insert(dname, slaReport.get());

    ++row;
    layout->addWidget(std::move(problemReport), row, 0);
    layout->addWidget(std::move(slaReport), row, 1);

    ++row;
    layout->addWidget(std::move(slaReportTitle), row, 0);
    layout->addWidget(std::move(csvExportIcon), row, 1, Wt::AlignmentFlag::Right);
  }

  setLayout(std::move(layout));
}


WebPlatformStatusPanel::~WebPlatformStatusPanel() {}



void WebPlatformStatusPanel::updateByView(const std::string& vname, const PlatformStatusListMapT& qosData)
{
  auto viewDashboardAliasName = m_viewDashboardAliasNames[vname];
  PlatformStatusListMapT::ConstIterator iterQosDataSet = qosData.find(viewDashboardAliasName);
  if (iterQosDataSet ==  qosData.end()) {
    return;
  }

  // process update since view found
  QMap<std::string, WebPieChart*>::iterator iterSlaPiechart = m_slaReportsRef.find(vname);
  if (iterSlaPiechart != m_slaReportsRef.end()) {
    WebQoSAnalytics slaData(*iterQosDataSet);
    (*iterSlaPiechart)->setSeverityData(slaData.normalDuration(), slaData.minorDuration(), slaData.majorDuration(), slaData.criticalDuration(), slaData.totalDuration());
    (*iterSlaPiechart)->repaint();
  }

  // update IT problem chart when applicable
  QMap<std::string, WebPlatformStatusRaw*>::iterator iterProblemTrendsChart = m_problemReportsRef.find(vname);
  if (iterProblemTrendsChart != m_problemReportsRef.end()) {
    (*iterProblemTrendsChart)->updateData(*iterQosDataSet);
  }

  // update QoS data for export
  QMap<std::string, WebCsvExportIcon*>::iterator iterCsvExportItem = m_csvLinksRef.find(vname);
  if (iterCsvExportItem != m_csvLinksRef.end()) {
    (*iterCsvExportItem)->updateData(vname, *iterQosDataSet);
  }
}

