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

#include <Wt/WStandardItem>
#include "WebChart.hpp"

WebChart::WebChart()
  : Wt::Chart::WPieChart(),
    m_model(new Wt::WStandardItemModel(this))
{
  setModel(m_model);     // Set the model.

  int row = 0;
  m_model->setData(row, 0, Wt::WString("Blueberry"));
  m_model->setData(row, 1, Wt::WString("Blueberry"), Wt::ToolTipRole);
  m_model->setData(row, 1, 120);

  m_model->setData(++row, 0, Wt::WString("Cherry"));
  m_model->setData(row, 1, 30);

  m_model->setData(++row, 0, Wt::WString("Apple"));
  m_model->setData(row, 1, 260);


  setLabelsColumn(0);    // Set the column that holds the labels.
  setDataColumn(1);      // Set the column that holds the data.

  // Configure location and type of labels.
  setDisplayLabels(Wt::Chart::Outside |
                          Wt::Chart::TextLabel |
                          Wt::Chart::TextPercentage);

  // Enable a 3D and shadow effect.
  setPerspectiveEnabled(true, 0.2);
  setShadowEnabled(true);

  setExplode(0, 0.3);  // Explode the first item.
  resize(800, 300);    // WPaintedWidget must be given an explicit size.
  setMargin(10, Wt::Top | Wt::Bottom); // Add margin vertically.
  setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right); // Center horizontally
}

WebChart::~WebChart()
{
  delete m_model;
}

