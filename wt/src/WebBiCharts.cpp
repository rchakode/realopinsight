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



QosTrendsChart::QosTrendsChart(const std::string& name,
                               const std::list<DbQosDataT>& data,
                               Wt::WContainerWidget* parent)
  : Wt::Chart::WCartesianChart(parent)
{
  setTitle(name);

  filteringPlottingData(data);

  resize(350, 150);
}


void QosTrendsChart::filteringPlottingData(const std::list<DbQosDataT>& data)
{
  std::list<DbQosDataT>::const_iterator qosit = data.begin();
  m_plotData.clear();

  if (! data.empty()) {
    m_plotData.push_back({qosit->timestamp, qosit->status});
    TimeStatusT last = m_plotData.back();
    while (++qosit, qosit != data.end()) {
      if (last.status != qosit->status) {
        last = {qosit->timestamp, qosit->status};
        m_plotData.push_back({qosit->timestamp, qosit->status});
      }
    }
  }
}


void QosTrendsChart::paintEvent(Wt::WPaintDevice * 	paintDevice)
{

}



RawQosTrendsChart::RawQosTrendsChart(const std::string& name,
                                     const std::list<DbQosDataT>& data,
                                     Wt::WContainerWidget* parent)
  : Wt::Chart::WCartesianChart(parent),
    m_model(new Wt::WStandardItemModel(data.size(), 7, this))
{
  setTitle(name);
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

  setBackground(Wt::WColor(248, 254, 252));
  setXSeriesColumn(0);
  setLegendEnabled(true);
  setType(Wt::Chart::ScatterPlot);
  axis(Wt::Chart::XAxis).setScale(Wt::Chart::DateTimeScale);
  setPlotAreaPadding(40, Wt::Left | Wt::Top | Wt::Bottom);
  setPlotAreaPadding(120, Wt::Right);

  for (int i = 6; i>=2; --i) {
    Wt::Chart::WDataSeries serie(i, Wt::Chart::CurveSeries);
    Wt::WColor color = ngrt4n::severityWColor(i - 2);
    serie.setPen(color);
    serie.setBrush(color);
    serie.setStacked(true);
    serie.setFillRange(Wt::Chart::MinimumValueFill);
    addSeries(serie);
  }
  resize(350, 150);
}
