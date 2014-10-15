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
  const double BI_CHART_AREA_WIDTH = 300;
  const double BI_CHART_AREA_HEIGHT = 150;
  const double BI_CHART_AREA_MARGIN = 25;
  const double BI_CHART_WIDTH = BI_CHART_AREA_WIDTH;
  const double BI_CHART_TREND_HEIGHT = 50;
  const double AREA_TOP_CORNER_Y = BI_CHART_AREA_HEIGHT - BI_CHART_AREA_MARGIN - BI_CHART_TREND_HEIGHT;
  const double TEXT_TOP_CORNER_Y = AREA_TOP_CORNER_Y - 5;
  const Wt::WColor LEGEND_TEXT_COLOR = Wt::WColor(0, 0, 0); // black
  const Wt::WColor TRANSPARENT_COLOR = Wt::WColor(0, 0, 0, 0);
}


/**
 * @brief QosTrendsChart::QosTrendsChart
 * @param viewName
 * @param data
 * @param parent
 */
QosTrendsChart::QosTrendsChart(const std::string& viewName,
                               const std::list<DbQosDataT>& data,
                               Wt::WContainerWidget* parent)
  : Wt::WPaintedWidget(parent),
    m_viewName(viewName)
{
  setStyleClass("bi-chart");
  setMargin(5, Wt::Top | Wt::Bottom);
  setMargin(BI_CHART_AREA_MARGIN, Wt::Left | Wt::Right);
  setLayoutSizeAware(true);
  updateData(data);
}

void QosTrendsChart::updateData(const std::list<DbQosDataT>& data)
{
  processPlottingData(data);
  update();
}

void QosTrendsChart::processPlottingData(const std::list<DbQosDataT>& data)
{
  std::list<DbQosDataT>::const_iterator qosit = data.begin();
  m_plottingData.clear();
  m_normalTimeCount = 0;

  if (! data.empty()) {
    TimeStatusT last = {qosit->timestamp, qosit->status};
    m_plottingData.push_back(last);
    while (++qosit, qosit != data.end()) {
      TimeStatusT current = {qosit->timestamp, qosit->status};
      m_plottingData.push_back(current);
      if (last.status == ngrt4n::Normal) {
        m_normalTimeCount += current.timestamp - last.timestamp;
      }
      last = m_plottingData.back();
    }
    long diff = (m_plottingData.back().timestamp - m_plottingData.front().timestamp);
    if (diff > 0)
      m_sla = 100 * ((double)m_normalTimeCount / diff);
    else
      m_sla = (m_plottingData.back().status == ngrt4n::Normal) ? 100 : 0;
  }
}


void QosTrendsChart::paintEvent(Wt::WPaintDevice* paintDevice)
{
  if (! m_plottingData.empty()) {

    TimeStatusT firstPoint = m_plottingData.front();
    TimeStatusT lastPoint = m_plottingData.last();
    const double SCALING_FACTOR = BI_CHART_WIDTH / (double)(lastPoint.timestamp - firstPoint.timestamp);

    TimeStatusesT::ConstIterator currentIt = m_plottingData.begin();
    TimeStatusesT::ConstIterator previousIt = m_plottingData.begin();
    double x1 = 0;
    double x2 = 0;
    Wt::WPainter painter(paintDevice);
    painter.setPen(TRANSPARENT_COLOR); // invisible
    drawRotatedLegendText(painter,
                          ngrt4n::timet2String(previousIt->timestamp),
                          x1, TEXT_TOP_CORNER_Y, -90);
    while (++currentIt, currentIt != m_plottingData.end()) {
      painter.setBrush(ngrt4n::severityWColor(currentIt->status));

      x1 = SCALING_FACTOR * (previousIt->timestamp - firstPoint.timestamp);
      x2 = SCALING_FACTOR * (currentIt->timestamp - firstPoint.timestamp);

      painter.drawRect(x1, AREA_TOP_CORNER_Y, x2, BI_CHART_TREND_HEIGHT);
      if (previousIt->status != currentIt->status)
        drawRotatedLegendText(painter,
                              ngrt4n::timet2String(previousIt->timestamp),
                              x1, TEXT_TOP_CORNER_Y, -90);

      previousIt = currentIt;
    }
    drawRotatedLegendText(painter, ngrt4n::timet2String(lastPoint.timestamp),
                          BI_CHART_WIDTH, TEXT_TOP_CORNER_Y, -90, -15);

    //FIXME:

    painter.setPen(LEGEND_TEXT_COLOR); // invisible
    painter.drawText(BI_CHART_AREA_WIDTH / 2, BI_CHART_AREA_HEIGHT - BI_CHART_AREA_MARGIN + 5,
                     Wt::WLength::Auto.toPixels(), Wt::WLength::Auto.toPixels(),
                     Wt::AlignCenter,
                     slaText());
  }
  resize(BI_CHART_AREA_WIDTH, BI_CHART_AREA_HEIGHT);
}

void QosTrendsChart::drawRotatedLegendText(Wt::WPainter& painter,
                                           const Wt::WString& text,
                                           double x,
                                           double y,
                                           double angle,
                                           double shiftLegendXPos)
{
  painter.save();
  painter.setPen(LEGEND_TEXT_COLOR);
  painter.drawLine(x, y, x, y + BI_CHART_TREND_HEIGHT + 10);
  painter.translate(x + shiftLegendXPos, y);
  painter.rotate(angle);
  painter.drawText(0, 0,
                   Wt::WLength::Auto.toPixels(), Wt::WLength::Auto.toPixels(),
                   Wt::AlignLeft,
                   text);
  painter.restore();
}




/**
 * @brief RawQosTrendsChart::RawQosTrendsChart
 * @param viewName
 * @param data
 * @param parent
 */
RawQosTrendsChart::RawQosTrendsChart(const std::string& viewName,
                                     const std::list<DbQosDataT>& data,
                                     Wt::WContainerWidget* parent)
  : Wt::Chart::WCartesianChart(parent),
    m_viewName(viewName),
    m_model(NULL)
{
  setStyleClass("bi-chart");
  setLegendEnabled(false);
  setPlotAreaPadding(BI_CHART_AREA_MARGIN, Wt::Left | Wt::Top | Wt::Bottom | Wt::Right);
  setType(Wt::Chart::ScatterPlot);
  axis(Wt::Chart::XAxis).setScale(Wt::Chart::DateTimeScale);
  updateData(data);
}


Wt::WFont RawQosTrendsChart::customTitleFont(void)
{
  Wt::WFont ft;
  return ft;
}

void RawQosTrendsChart::setChartTitle(void)
{
  setTitle(Q_TR("IT Problem Trends"));
  setTitleFont(customTitleFont());
}

void RawQosTrendsChart::updateData(const std::list<DbQosDataT>& data)
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
  resize(BI_CHART_AREA_WIDTH, BI_CHART_AREA_HEIGHT);
}
