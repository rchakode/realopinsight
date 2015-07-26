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

#include "WebBiDashlet.hpp"
#include <ctime>


WebBiDashlet::WebBiDashlet()
  : m_layout(new Wt::WGridLayout())
{
  setLayout(m_layout);
}



WebBiDashlet::~WebBiDashlet()
{
}

void WebBiDashlet::initialize(const DbViewsT& viewList)
{
  //TODO: dont use pointer for chart widgets ?
  int rowIndex = 0;
  for (const auto& view: viewList) {
    m_chartTitleMap.insert(view.name, createTitleWidget(view.name));
    m_csvExportLinkMap.insert(view.name, new WebCsvExportIcon());
    m_itProblemChartMap.insert(view.name, new RawQosTrendsChart(view.name, QosDataList()));
    m_qosChartMap.insert(view.name, new WebPieChart(ChartBase::SLAData));

    m_layout->addWidget(m_chartTitleMap[view.name], rowIndex, 0);
    m_layout->addWidget(m_csvExportLinkMap[view.name], rowIndex, 1, Wt::AlignRight);
    m_layout->addWidget(m_qosChartMap[view.name], ++rowIndex, 0);
    m_layout->addWidget(m_itProblemChartMap[view.name], rowIndex, 1);

    ++rowIndex;
  }
}


Wt::WText* WebBiDashlet::createTitleWidget(const std::string& viewName)
{
  Wt::WString title = Wt::WString("<h5>{1}</h5>").arg(viewName);
  return new Wt::WText(title, Wt::XHTMLText);
}


void WebBiDashlet::update(const QosDataByViewMapT& qosDataMap, const std::string& viewName)
{
  QosDataByViewMapT::ConstIterator iterQosDataSet = qosDataMap.find(viewName);
  if (iterQosDataSet ==  qosDataMap.end())
    return; // stop process

  // update qos chart when applicable
  QMap<std::string, WebPieChart*>::iterator iterQosChart = m_qosChartMap.find(viewName);
  if (iterQosChart == m_qosChartMap.end()) {
    WebBISLAChart slaManager(*iterQosDataSet);
    iterQosChart->setSeverityData(slaManager.normalDuration(),
                                  slaManager.minorDuration(),
                                  slaManager.majorDuration(),
                                  slaManager.criticalDuration(),
                                  slaManager.totalDuration());
    iterQosChart->repaint();
  }

  // update IT problem chart when applicable
  QMap<std::string, RawQosTrendsChart*>::iterator iterItProblemChart = m_itProblemChartMap.find(viewName);
  if (iterItProblemChart != m_itProblemChartMap.end())
    iterItProblemChart->updateData(*iterQosDataSet);

}

