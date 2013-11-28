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
#include <QDebug>

WebChart::WebChart(std::map<int, int> _sdata)
  : m_chart(new Wt::Chart::WPieChart(this)),
    m_model(new Wt::WStandardItemModel(this))
{
  m_chart->setModel(m_model);     // Set the model.

  // Configure the header.
  m_model->insertColumns(m_model->columnCount(), 2);
  m_model->setHeaderData(0, Wt::WString("Item"));
  m_model->setHeaderData(1, Wt::WString("Sales"));
  m_model->insertRows(m_model->rowCount(), 5);

  int row = 0;
  m_model->setData(  row, 0, utils::severity2Str(MonitorBroker::Normal).toStdString());
  m_model->setData(  row, 1, utils::severity2Str(MonitorBroker::Normal).toStdString(), Wt::ToolTipRole);
  m_model->setData(  row, 1, _sdata[MonitorBroker::Normal]);

  m_model->setData(++row, 0, utils::severity2Str(MonitorBroker::Minor).toStdString());
  m_model->setData(  row, 1, _sdata[MonitorBroker::Minor]);

  m_model->setData(++row, 0, utils::severity2Str(MonitorBroker::Major).toStdString());
  m_model->setData(  row, 1, _sdata[MonitorBroker::Major]);

  m_model->setData(++row, 0, utils::severity2Str(MonitorBroker::Critical).toStdString());
  m_model->setData(  row, 1, _sdata[MonitorBroker::Critical]);

  m_model->setData(++row, 0, utils::severity2Str(MonitorBroker::Unknown).toStdString());
  m_model->setData(  row, 1, _sdata[MonitorBroker::Unknown]);

  m_chart->setLabelsColumn(0);    // Set the column that holds the labels.
  m_chart->setDataColumn(1);      // Set the column that holds the data.

  // Configure location and type of labels.
  m_chart->setDisplayLabels(Wt::Chart::TextPercentage);
  // Enable a 3D and shadow effect.
  m_chart->setPerspectiveEnabled(true, 0.2);
  m_chart->setShadowEnabled(true);

  m_chart->resize(250, 250);    // WPaintedWidget must be given an explicit size.
  m_chart->setMargin(10, Wt::Top | Wt::Bottom); // Add margin vertically.
  m_chart->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right); // Center horizontally
}

WebChart::~WebChart()
{
  delete m_model;
  delete m_chart;
}

