/*
 * PieChart.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
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


#include "PieChart.hpp"

PieChart::PieChart(const QRectF& _bounding_rect, QWidget * _parent)
  : QWidget( _parent ), boundingRect( _bounding_rect ),
    legend(new StatsLegend(QPoint(_bounding_rect.width() + 25, 10), this))
{
  resize(legend->size().width() + 10,  boundingRect.topLeft().y() + boundingRect.height());
  setStyleSheet("background:transparent");
}

PieChart::~PieChart()
{
  delete legend;
  slices.clear();
}

void PieChart::update(const CheckStatusCountT& _check_status_count, qint32 _count, QString& toolTip)
{
  qint32 critical_count = _check_status_count[MonitorBroker::Critical];
  qint32 major_count = _check_status_count[MonitorBroker::Major];
  qint32 minor_count = _check_status_count[MonitorBroker::Minor];
  qint32 ok_count =  _check_status_count[MonitorBroker::Normal];
  qint32 unknown_count = _count - (critical_count + major_count + minor_count + ok_count);

  float critical_ratio= (100.0 * critical_count)/_count;
  float major_ratio = (100.0 * major_count)/_count;
  float minor_ratio = (100.0 * minor_count)/_count;
  float unknown_ratio = (100.0 * unknown_count) / _count;
  float ok_ratio = (100.0 * ok_count)/_count;

  slices[MonitorBroker::Critical] =
      new PieChartItem(boundingRect,
                       0,
                       3.6 * critical_ratio,
                       StatsLegend::COLOR_CRITICAL,
                       this);
  slices[MonitorBroker::Major] =
      new PieChartItem(boundingRect,
                       3.6 * critical_ratio,
                       3.6 * major_ratio,
                       StatsLegend::COLOR_MAJOR,
                       this);
  slices[MonitorBroker::Minor] =
      new PieChartItem(boundingRect,
                       3.6 * (critical_ratio + major_ratio),
                       3.6 * minor_ratio,
                       StatsLegend::COLOR_MINOR,
                       this);
  slices[MonitorBroker::Unknown] =
      new PieChartItem(boundingRect,
                       3.6 * (critical_ratio + major_ratio + minor_ratio),
                       3.6 * unknown_ratio,
                       StatsLegend::COLOR_UNKNOWN,
                       this);
  slices[MonitorBroker::Normal] =
      new PieChartItem(boundingRect,
                       3.6 * (critical_ratio + major_ratio + minor_ratio + unknown_ratio),
                       3.6 * ok_ratio,
                       StatsLegend::COLOR_NORMAL,
                       this);
  toolTip = QObject::tr("Normal: ")%QString::number(ok_count)%
      "/"%QString::number(_count)%" ("%QString::number(ok_ratio, 'f', 0)%"%)"
      %"\n"%QObject::tr("Minor: ")%QString::number(minor_count)%
      "/"%QString::number(_count)%" ("%QString::number(minor_ratio, 'f', 0)%"%)"
      %"\n"%QObject::tr("Major: ")%QString::number(major_count)%
      "/"%QString::number(_count)%" ("%QString::number(major_ratio, 'f', 0)%"%)"
      %"\n"%QObject::tr("Critical: ")%QString::number(critical_count)%"/"
      %QString::number(_count)%" ("%QString::number(critical_ratio, 'f', 0) %"%)"
      %"\n"%QObject::tr("Unknown: ")%QString::number(unknown_count)%
      "/"%QString::number(_count)%" ("%QString::number(unknown_ratio, 'f', 0)%"%)";
}
