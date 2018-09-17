/*
 * WebBiRawChart.cpp
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

#include "WebBiRawChart.hpp"
#include <Wt/WStandardItemModel>
#include <Wt/WFont>


WebBiRawChart::WebBiRawChart(const std::string& viewName)
  : Wt::Chart::WCartesianChart(nullptr),
    m_viewName(viewName),
    m_dataModel(nullptr)
{
  setStyleClass("bi-chart");
  setLegendEnabled(false);
  setType(Wt::Chart::ScatterPlot);
  axis(Wt::Chart::XAxis).setScale(Wt::Chart::DateTimeScale);
  setTitleFont(ngrt4n::chartTitleFont());
}


void WebBiRawChart::setChartTitle(void)
{
  setTitle(Q_TR("Trends - % Problems"));
}

void WebBiRawChart::updateData(const QosDataList& data)
{
  Wt::WStandardItemModel* model = new Wt::WStandardItemModel(static_cast<int>(data.size()), 9, this);
  model->setHeaderData(0, Q_TR("Date/time"));
  model->setHeaderData(1, Q_TR("Status"));
  model->setHeaderData(2, Q_TR("% Normal"));
  model->setHeaderData(3, Q_TR("% Minor"));
  model->setHeaderData(4, Q_TR("% Major"));
  model->setHeaderData(5, Q_TR("% Critical"));
  model->setHeaderData(6, Q_TR("% Unknown"));
  model->setHeaderData(7, Q_TR("placeholder for 0% value"));
  model->setHeaderData(8, Q_TR("placeholder for 100% value"));

  int row = 0;
  for (const auto& entry : data) {
    Wt::WDateTime date;
    date.setTime_t(entry.timestamp);

    model->setData(row, 0, date);
    model->setData(row, 1, entry.status);

    float sev = entry.normal;
    model->setData(row, 2, sev);

    sev += entry.minor;
    model->setData(row, 3, sev);

    sev += entry.major;
    model->setData(row, 4, sev);

    sev += entry.critical;
    model->setData(row, 5, sev);

    sev += entry.unknown;
    model->setData(row, 6, sev);

    // placeholders
    model->setData(row, 7, 0.0);
    model->setData(row, 8, 100.0);
    ++row;
  }

  resetDataModel(model);

  setXSeriesColumn(0);
  for (int i = 8; i >= 2; --i) {
    Wt::Chart::WDataSeries serie(i, Wt::Chart::LineSeries);
    Wt::WColor color = ngrt4n::severityWColor(i - 2);
    serie.setPen(color);
    serie.setBrush(color);
    serie.setStacked(true);
    serie.setFillRange(Wt::Chart::MinimumValueFill);
    addSeries(serie);
  }
  setChartTitle();
}


void WebBiRawChart::resetDataModel(Wt::WStandardItemModel* model)
{
  setModel(model);
  if (m_dataModel) {
    delete m_dataModel;
  }
  m_dataModel = model;
}
