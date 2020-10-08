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

#include "WebPlatformStatusAnalyticsCharts.hpp"
#include "WebPlatformStatusRaw.hpp"
#include <ctime>
#include <regex>


WebPlatformStatusAnalyticsCharts::WebPlatformStatusAnalyticsCharts(const DbViewsT& listOfViews, const SourceListT& listOfSources)
  : m_layoutRef(nullptr),
    m_dateFilterRef(nullptr)
{
  auto layout = std::make_unique<Wt::WVBoxLayout>();
  m_layoutRef = layout.get();

  auto dateFilter = std::make_unique<WebPlatformStatusDateFilter>();
  m_dateFilterRef = dateFilter.get();
  m_dateFilterRef->reportPeriodChanged().connect(std::bind([=]{m_reportPeriodChanged.emit(this->startTime(), this->endTime());}));
  layout->addWidget(std::move(dateFilter));

  for (const auto& view : listOfViews) {
    auto row = std::make_unique<Wt::WTemplate>(Wt::WString::tr("thumbnail.tpl"));

    auto title = view.name;
    std::smatch regexMatch;
    if (std::regex_match(view.name, regexMatch, std::regex("(Source[0-9]):(.+)"))) {
      auto sourceId = QString::fromStdString(regexMatch[1].str());
      auto& source = listOfSources[sourceId];
      title = Wt::WString("{1} @ {2}").arg(regexMatch[2].str()).arg(source.mon_url.toStdString()).toUTF8();
    }
    row->bindString("problem-status-percent-title", title);

    auto container = std::make_unique<Wt::WContainerWidget>();
    auto problemReportChart = container->addNew<WebPlatformStatusRaw>(view.name);
    m_problemReportRef.insert(view.name, problemReportChart);
    row->bindWidget("problem-status-percent-chart", std::move(container));

    auto slaReportChart = row->bindNew<WebPieChart>("platform-availability-chart", ChartBase::SLAData);
    m_slaReportsRef.insert(view.name, slaReportChart);

    auto csvExportIcon = row->bindNew<WebCsvExportIcon>("platform-availability-csv-export");
    m_csvLinksRef.insert(view.name, csvExportIcon);

    layout->addWidget(std::move(row));
  }

  setLayout(std::move(layout));
}


WebPlatformStatusAnalyticsCharts::~WebPlatformStatusAnalyticsCharts() {}


void WebPlatformStatusAnalyticsCharts::updateByView(const std::string& vname, const PlatformMappedStatusHistoryT& statusHistory)
{
  PlatformMappedStatusHistoryT::ConstIterator statusHistoryFound = statusHistory.find(vname);
  if (statusHistoryFound == statusHistory.end()) {
    CORE_LOG("warn", QObject::tr("view not found to update chart %1").arg(vname.c_str()).toStdString());
    return;
  }

  // process update since view found
  QMap<std::string, WebPieChart*>::iterator iterSlaPiechart = m_slaReportsRef.find(vname);
  if (iterSlaPiechart != m_slaReportsRef.end()) {
    WebPlatformStatusAnalyticsData statusAnalyticsData(*statusHistoryFound);
    (*iterSlaPiechart)->setSeverityData(statusAnalyticsData.normalDuration(), statusAnalyticsData.minorDuration(), statusAnalyticsData.majorDuration(), statusAnalyticsData.criticalDuration(), statusAnalyticsData.totalDuration());
    (*iterSlaPiechart)->repaint();
  }

  // update IT problem chart when applicable
  QMap<std::string, WebPlatformStatusRaw*>::iterator iterProblemTrendsChart = m_problemReportRef.find(vname);
  if (iterProblemTrendsChart != m_problemReportRef.end()) {
    (*iterProblemTrendsChart)->updateData(*statusHistoryFound);
  }

  // update QoS data for export
  QMap<std::string, WebCsvExportIcon*>::iterator iterCsvExportItem = m_csvLinksRef.find(vname);
  if (iterCsvExportItem != m_csvLinksRef.end()) {
    (*iterCsvExportItem)->updateData(vname, *statusHistoryFound);
  }
}

