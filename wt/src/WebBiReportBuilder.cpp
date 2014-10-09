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

#include "WebBiReportBuilder.hpp"
#include "WebUtils.hpp"
#include <QDebug>

WebBiReportBuilder::WebBiReportBuilder(const std::list<DbQosDataT>& data, Wt::WContainerWidget* parent)
  : Wt::Chart::WCartesianChart(parent),
    m_model(new Wt::WStandardItemModel(data.size(), 7, this))
{
  m_model->setHeaderData(0, Q_TR("Date/time"));
  m_model->setHeaderData(1, Q_TR("Status"));
  m_model->setHeaderData(2, Q_TR("% Normal"));
  m_model->setHeaderData(3, Q_TR("% Minor"));
  m_model->setHeaderData(4, Q_TR("% Major"));
  m_model->setHeaderData(5, Q_TR("% Critical"));
  m_model->setHeaderData(6, Q_TR("% Unknown"));

  m_row = 0;
  for (const auto& entry : data) {
    Wt::WDateTime date;
    date.setTime_t(entry.timestamp);
    m_model->setData(m_row, 0, date.date());
    m_model->setData(m_row, 1, entry.status);
    m_model->setData(m_row, 2, entry.normal);
    m_model->setData(m_row, 3, entry.minor);
    m_model->setData(m_row, 4, entry.major);
    m_model->setData(m_row, 5, entry.critical);
    m_model->setData(m_row, 6, entry.unknown);
    ++m_row;
  }

  // setBackground(Wt::WColor(220, 220, 220));
  setModel(m_model);
  setXSeriesColumn(0);
  setLegendEnabled(true);
  setType(Wt::Chart::ScatterPlot);
  axis(Wt::Chart::XAxis).setScale(Wt::Chart::DateScale);
  setPlotAreaPadding(40, Wt::Left | Wt::Top | Wt::Bottom);
  setPlotAreaPadding(120, Wt::Right);

  Wt::Chart::WDataSeries series(6, Wt::Chart::LineSeries);
  series.setShadow(Wt::WShadow(3, 3, Wt::WColor(0, 0, 0, 127), 3));
  addSeries(series);
  resize(350, 200);
}
