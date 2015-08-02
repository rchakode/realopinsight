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

WebBiRawChart::WebBiRawChart(void)
  : Wt::Chart::WCartesianChart(0),
    m_viewName(""),
    m_model(NULL)
{
  setStyleClass("bi-chart");
  setLegendEnabled(false);
  setPlotAreaPadding(BI_CHART_AREA_MARGIN, Wt::Left | Wt::Top | Wt::Bottom | Wt::Right);
  setType(Wt::Chart::ScatterPlot);
  axis(Wt::Chart::XAxis).setScale(Wt::Chart::DateTimeScale);
}


void WebBiRawChart::initialize(const std::string& viewName, const QosDataList& data)
{
  setViewName(viewName);
  updateData(data);
}

Wt::WFont WebBiRawChart::customTitleFont(void)
{
  Wt::WFont ft;
  return ft;
}

void WebBiRawChart::setChartTitle(void)
{
  setTitle(Q_TR("IT Problem Trends"));
  setTitleFont(customTitleFont());
}

void WebBiRawChart::updateData(const QosDataList& data)
{
  Wt::WStandardItemModel* model = new Wt::WStandardItemModel(data.size(), 7, this);

  model->setHeaderData(0, Q_TR("Date/time"));
  model->setHeaderData(1, Q_TR("Status"));
  model->setHeaderData(2, Q_TR("% Normal"));
  model->setHeaderData(3, Q_TR("% Minor"));
  model->setHeaderData(4, Q_TR("% Major"));
  model->setHeaderData(5, Q_TR("% Critical"));
  model->setHeaderData(6, Q_TR("% Unknown"));

  int row = 0;
  for (const auto& entry : data) {
    Wt::WDateTime date;
    date.setTime_t(entry.timestamp);
    model->setData(row, 0, date);

    model->setData(row, 1, entry.status);

    float cum = entry.normal;
    model->setData(row, 2, cum);

    cum += entry.minor;
    model->setData(row, 3, cum);

    cum += entry.major;
    model->setData(row, 4, cum);

    cum += entry.critical;
    model->setData(row, 5, cum);

    cum += entry.unknown;
    model->setData(row, 6, cum);
    ++row;
  }

  setModel(model);
  if (m_model)
    delete m_model;
  m_model = model;

  setXSeriesColumn(0);
  for (int i = 6; i>=2; --i) {
    Wt::Chart::WDataSeries serie(i, Wt::Chart::LineSeries);
    Wt::WColor color = ngrt4n::severityWColor(i - 2);
    serie.setPen(color);
    serie.setBrush(color);
    serie.setStacked(true);
    serie.setFillRange(Wt::Chart::MinimumValueFill);
    addSeries(serie);
  }

  setChartTitle();
  resize(BI_RAW_CHART_AREA_WIDTH, BI_RAW_CHART_AREA_HEIGHT);
}
