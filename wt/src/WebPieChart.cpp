/*
 * WebPieChart.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 27-11-2013                                                 #
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

#include "WebPieChart.hpp"
#include "utilsClient.hpp"
#include "StatsLegend.hpp"
#include <Wt/WStandardItem>
#include <QString>
#include <QDebug>
#include <Wt/WPainter>
#include <Wt/WPen>
#include <Wt/WScrollArea>
#include <Wt/Chart/WChartPalette>

class WebCharPalette : public Wt::Chart::WChartPalette
{
public:
  WebCharPalette(Wt::WStandardItemModel* model):
    m_model(model)
  {

  }

  virtual Wt::WBrush brush (int index) const
  {
    return Wt::WBrush(WebPieChart::colorFromSeverity(index));
  }

  virtual Wt::WPen borderPen (int index) const
  {
    return Wt::WPen(Wt::WColor(255, 255, 255, 0));
  }

  Wt::WPen strokePen (int) const
  {
    // Check value first
    return Wt::WPen(Wt::WColor(255, 255, 255, 0));
  }

  Wt::WColor fontColor (int index) const
  {
    // Check value first
    return Wt::WColor(255, 255, 255, 0);
  }

  virtual Wt::WColor color (int index) const
  {
    return Wt::WColor(255, 255, 255, 0);
  }

private:
  Wt::WStandardItemModel* m_model;
};

WebPieChart::WebPieChart(void)
  : Wt::Chart::WPieChart(),
    m_model(new Wt::WStandardItemModel(this)),
    m_scrollArea(new Wt::WScrollArea())
{
  setModel(m_model);

  resize(StatsLegend::CHART_WIDTH, StatsLegend::CHART_HEIGHT);  // WPaintedWidget must be given an explicit size.
  setMargin(0, Wt::Top);
  setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);

  m_scrollArea->setWidget(this);
  m_scrollArea->setMargin(0, Wt::Top| Wt::Bottom);

  // Configure the header.
  m_model->insertColumns(m_model->columnCount(), 2);
  m_model->setHeaderData(0, Wt::WString("Item"));
  m_model->setHeaderData(1, Wt::WString("Sales"));
  m_model->insertRows(m_model->rowCount(), 5);
  // Initialize data
  setSeverityData(MonitorBroker::Normal, 0);
  setSeverityData(MonitorBroker::Minor, 0);
  setSeverityData(MonitorBroker::Major, 0);
  setSeverityData(MonitorBroker::Critical, 0);
  setSeverityData(MonitorBroker::Unknown, 0);
  // Draw the chart
  setLabelsColumn(0);    // Set the column that holds the labels.
  setDataColumn(1);      // Set the column that holds the data.
  setDisplayLabels(Wt::Chart::Inside|Wt::Chart::TextPercentage); // Configure location and type of labels.
  setPerspectiveEnabled(true, 0.2); // Enable a 3D and shadow effect.
  setShadowEnabled(true);

  setPlotAreaPadding(0, Wt::All);
  setPalette(new WebCharPalette(m_model));
}

WebPieChart::~WebPieChart()
{
  // m_scrollArea is deleted by the layout manager
  delete m_model;
}

Wt::WColor WebPieChart::colorFromSeverity(const int& _sev)
{
  QColor qcolor = utils::severityColor(_sev);
  return Wt::WColor(qcolor.red(), qcolor.green(), qcolor.blue(), qcolor.alpha());
}

void WebPieChart::setSeverityData(int _sev, int _count)
{
  std::string label = utils::severityText(_sev).toStdString();
  m_model->setData(_sev, 0, label);
 // m_model->setData(_sev, 1, label, Wt::ToolTipRole);
  m_model->setData(_sev, 1, _count);
  //setBrush(_sev, Wt::WBrush(colorFromSeverity(_sev)));
}
