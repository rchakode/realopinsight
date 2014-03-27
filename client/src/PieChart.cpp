/*
 * PieChart.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
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
#include "GuiUtils.hpp"

PieChart::PieChart(const QRectF& _bounding_rect, QWidget * _parent)
  : QWidget( _parent ), m_boundingRect( _bounding_rect ),
    m_legend(new StatsLegend(QPoint(_bounding_rect.width() + 25, 10), this))
{
  resize(m_legend->size().width() + 10,  m_boundingRect.topLeft().y() + m_boundingRect.height());
  setStyleSheet("background:transparent");
}

PieChart::~PieChart()
{
  delete m_legend;
  m_slices.clear();
}

void PieChart::update(const CheckStatusCountT& _check_status_count, qint32 _count, QString& toolTip)
{
  qint32 critical_count = _check_status_count[ngrt4n::Critical];
  qint32 major_count = _check_status_count[ngrt4n::Major];
  qint32 minor_count = _check_status_count[ngrt4n::Minor];
  qint32 ok_count =  _check_status_count[ngrt4n::Normal];
  qint32 unknown_count = _count - (critical_count + major_count + minor_count + ok_count);

  float critical_ratio= (100.0 * critical_count)/_count;
  float major_ratio = (100.0 * major_count)/_count;
  float minor_ratio = (100.0 * minor_count)/_count;
  float unknown_ratio = (100.0 * unknown_count) / _count;
  float ok_ratio = (100.0 * ok_count)/_count;

  m_slices[ngrt4n::Critical] =
      new PieChartItem(m_boundingRect,
                       0,
                       3.6 * critical_ratio,
                       ngrt4n::COLOR_CRITICAL,
                       this);
  m_slices[ngrt4n::Major] =
      new PieChartItem(m_boundingRect,
                       3.6 * critical_ratio,
                       3.6 * major_ratio,
                       ngrt4n::COLOR_MAJOR,
                       this);
  m_slices[ngrt4n::Minor] =
      new PieChartItem(m_boundingRect,
                       3.6 * (critical_ratio + major_ratio),
                       3.6 * minor_ratio,
                       ngrt4n::COLOR_MINOR,
                       this);
  m_slices[ngrt4n::Unknown] =
      new PieChartItem(m_boundingRect,
                       3.6 * (critical_ratio + major_ratio + minor_ratio),
                       3.6 * unknown_ratio,
                       ngrt4n::COLOR_UNKNOWN,
                       this);
  m_slices[ngrt4n::Normal] =
      new PieChartItem(m_boundingRect,
                       3.6 * (critical_ratio + major_ratio + minor_ratio + unknown_ratio),
                       3.6 * ok_ratio,
                       ngrt4n::COLOR_NORMAL,
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
