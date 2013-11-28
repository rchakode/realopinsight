/*
 * WebChart.cpp
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

#include "WebChart.hpp"
#include "utilsClient.hpp"
#include <Wt/WStandardItem>
#include <QString>
#include <QDebug>
#include <Wt/WPainter>
#include <Wt/WPen>

WebChart::WebChart(void)
  : Wt::Chart::WPieChart(),
    m_model(new Wt::WStandardItemModel(this)),
    m_widget(new Wt::WContainerWidget())
{
  // set the widget
  setModel(m_model);     // Set the model.
  m_widget->addWidget(this);

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
  setDisplayLabels(Wt::Chart::TextPercentage); // Configure location and type of labels.
  setPerspectiveEnabled(true, 0.2); // Enable a 3D and shadow effect.
  setShadowEnabled(true);
  resize(200, 250);    // WPaintedWidget must be given an explicit size.
  //setMargin(10, Wt::Top | Wt::Bottom); // Add margin vertically.
  setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right); // Center horizontally
}

WebChart::~WebChart()
{
  delete m_model;
  delete m_widget;
}

//FIXME: do custom painting to avoid black line, 0%legend
//void WebChart::paintEvent(Wt::WPaintDevice* _pdevice)
//{
//  painter = new Wt::WPainter(_pdevice);
//  //painter->setPen(Wt::WPen(Wt::WColor(0,0,0,0)));
//  Wt::Chart::WPieChart::update();
//}

Wt::WColor WebChart::colorFromSeverity(const int& _sev)
{
  QColor qcolor = utils::computeColor(_sev);
  return Wt::WColor(qcolor.red(), qcolor.green(), qcolor.blue(), qcolor.alpha());
}

void WebChart::setSeverityData(int _sev, int _count)
{
  std::string label = utils::severity2Str(_sev).toStdString();
  m_model->setData(_sev, 0, label);
  m_model->setData(_sev, 1, label, Wt::ToolTipRole);
  m_model->setData(_sev, 1, _count);
  setBrush(_sev, Wt::WBrush(colorFromSeverity(_sev)));
}
