/*
 * WebBiReportBuilder.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 07-10-2014                                                  #
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

#include "WebBiCharts.hpp"
#include "WebUtils.hpp"
#include <QDebug>


namespace {
  const double BI_CHART_WIDTH = 300;
  const double BI_CHART_HEIGHT = 150;
  const double BI_CHART_MARGIN = 25;
  const double BI_CHART_TREND_HEIGHT = 50;
}

QosTrendsChart::QosTrendsChart(const std::string& viewName,
                               const std::list<DbQosDataT>& data,
                               Wt::WContainerWidget* parent)
  : Wt::WPaintedWidget(parent),
    m_viewName(viewName)
{
  setStyleClass("bi-chart");
  setMargin(5, Wt::Top | Wt::Bottom);
  setMargin(BI_CHART_MARGIN, Wt::Left | Wt::Right);
  setLayoutSizeAware(false);
  resize(BI_CHART_WIDTH, BI_CHART_HEIGHT);
  filteringPlottingData(data);
}


void QosTrendsChart::filteringPlottingData(const std::list<DbQosDataT>& data)
{
  std::list<DbQosDataT>::const_iterator qosit = data.begin();
  m_plottingData.clear();

  if (! data.empty()) {
    m_plottingData.push_back({qosit->timestamp, qosit->status});
    TimeStatusT last = m_plottingData.back();
    while (++qosit, qosit != data.end()) {
      if (last.status != qosit->status) {
        last = {qosit->timestamp, qosit->status};
        m_plottingData.push_back(last);
      }
    }
    // always insert the last point, could be duplicated...
    --qosit;
    m_plottingData.push_back({qosit->timestamp, qosit->status});
  }
}


void QosTrendsChart::paintEvent(Wt::WPaintDevice* paintDevice)
{
  const Wt::WColor TRANSPARENT_COLOR = Wt::WColor(0, 0, 0, 0);
  const double AREA_TOP_CORNER_Y = BI_CHART_HEIGHT - BI_CHART_MARGIN - BI_CHART_TREND_HEIGHT;
  const double TEXT_TOP_CORNER_Y = AREA_TOP_CORNER_Y - 5;

  if (! m_plottingData.empty()) {
    TimeStatusT firstPoint = m_plottingData.front();
    TimeStatusesT::ConstIterator currentIt = m_plottingData.begin();
    TimeStatusesT::ConstIterator previousIt = m_plottingData.begin();
    double x1 = 0;
    double x2 = 0;
    Wt::WString textLegend = "";
    Wt::WPainter painter(paintDevice);
    painter.setPen(TRANSPARENT_COLOR); // invisible
    while (++currentIt, currentIt != m_plottingData.end()) {
      painter.setBrush(ngrt4n::severityWColor(currentIt->status));

      x1 = previousIt->timestamp - firstPoint.timestamp;
      x2 = currentIt->timestamp - firstPoint.timestamp;
      textLegend = ngrt4n::timet2String(previousIt->timestamp, "dd/MM-hh:mm");

      painter.drawRect(x1, AREA_TOP_CORNER_Y, x2, BI_CHART_TREND_HEIGHT);
      drawRotatedLegendText(painter, textLegend, x1, TEXT_TOP_CORNER_Y, -80);

      previousIt = currentIt;
    }
    textLegend = ngrt4n::timet2String(previousIt->timestamp, "dd/MM-hh:mm");
    drawRotatedLegendText(painter, textLegend, x2 - 100, TEXT_TOP_CORNER_Y, -80);
  }
}


void QosTrendsChart::drawRotatedLegendText(Wt::WPainter& painter,
                                           const Wt::WString& text,
                                           double x, double y, double angle)
{
  const Wt::WColor LEGEND_TEXT_COLOR = Wt::WColor(0, 0, 0); // black

  painter.save();
  painter.setPen(LEGEND_TEXT_COLOR);
  painter.translate(x, y);
  painter.rotate(angle);
  painter.drawText(0, 0,
                   Wt::WLength::Auto.toPixels(), Wt::WLength::Auto.toPixels(),
                   Wt::AlignLeft | Wt::AlignLeft,
                   text);
  painter.restore();
}


RawQosTrendsChart::RawQosTrendsChart(const std::string& viewName,
                                     const std::list<DbQosDataT>& data,
                                     Wt::WContainerWidget* parent)
  : Wt::Chart::WCartesianChart(parent),
    m_model(new Wt::WStandardItemModel(data.size(), 7, this)),
    m_viewName(viewName)
{
  setStyleClass("bi-chart");
  setLegendEnabled(false);
  setPlotAreaPadding(BI_CHART_MARGIN, Wt::Left | Wt::Top | Wt::Bottom | Wt::Right);
  setType(Wt::Chart::ScatterPlot);
  axis(Wt::Chart::XAxis).setScale(Wt::Chart::DateTimeScale);
  setModel(m_model);

  m_model->setHeaderData(0, Q_TR("Date/time"));
  m_model->setHeaderData(1, Q_TR("Status"));
  m_model->setHeaderData(2, Q_TR("% Normal"));
  m_model->setHeaderData(3, Q_TR("% Minor"));
  m_model->setHeaderData(4, Q_TR("% Major"));
  m_model->setHeaderData(5, Q_TR("% Critical"));
  m_model->setHeaderData(6, Q_TR("% Unknown"));

  int row = 0;
  for (const auto& entry : data) {
    Wt::WDateTime date;
    date.setTime_t(entry.timestamp);
    m_model->setData(row, 0, date);

    m_model->setData(row, 1, entry.status);

    float cum = entry.normal;
    m_model->setData(row, 2, cum);

    cum += entry.minor;
    m_model->setData(row, 3, cum);

    cum += entry.major;
    m_model->setData(row, 4, cum);

    cum += entry.critical;
    m_model->setData(row, 5, cum);

    cum += entry.unknown;
    m_model->setData(row, 6, cum);
    ++row;
  }

  setXSeriesColumn(0);
  for (int i = 6; i>=2; --i) {
    Wt::Chart::WDataSeries serie(i, Wt::Chart::CurveSeries);
    Wt::WColor color = ngrt4n::severityWColor(i - 2);
    serie.setPen(color);
    serie.setBrush(color);
    serie.setStacked(true);
    serie.setFillRange(Wt::Chart::MinimumValueFill);
    addSeries(serie);
  }

  resize(BI_CHART_WIDTH, BI_CHART_HEIGHT);
}
