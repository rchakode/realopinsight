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

PieChart::PieChart(const QRectF& chartArea, QWidget * _parent)
  : QWidget( _parent ), m_boundingRect(chartArea),
    m_legend(new StatsLegend(QPoint(chartArea.width() + 25, 10), this))
{
  resize(m_legend->size().width() + 10,  m_boundingRect.topLeft().y() + m_boundingRect.height());
  setStyleSheet("background:#ffffff;");
}

PieChart::~PieChart()
{
  delete m_legend;
}

void PieChart::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  qint32 critical_count = m_statsData[ngrt4n::Critical];
  qint32 major_count = m_statsData[ngrt4n::Major];
  qint32 minor_count = m_statsData[ngrt4n::Minor];
  qint32 ok_count =  m_statsData[ngrt4n::Normal];
  qint32 unknown_count = m_nbStatsEntries - (critical_count + major_count + minor_count + ok_count);

  int critical_ratio= (100.0 * critical_count)/m_nbStatsEntries;
  int major_ratio = (100.0 * major_count)/m_nbStatsEntries;
  int minor_ratio = (100.0 * minor_count)/m_nbStatsEntries;
  int unknown_ratio = (100.0 * unknown_count)/m_nbStatsEntries;
  int ok_ratio = (100.0 * ok_count)/m_nbStatsEntries;

  painter.setPen(Qt::transparent);
  painter.setBrush(ngrt4n::COLOR_CRITICAL);
  painter.drawPie(m_boundingRect, 0, 16 * 3.6 * critical_ratio);
  painter.setBrush(ngrt4n::COLOR_MAJOR);
  painter.drawPie(m_boundingRect, 3.6 * critical_ratio, 16 * 3.6 * major_ratio);
  painter.setBrush(ngrt4n::COLOR_MINOR);
  painter.drawPie(m_boundingRect, 3.6 * (critical_ratio + major_ratio), 16 * 3.6 * minor_ratio);
  painter.setBrush(ngrt4n::COLOR_UNKNOWN);
  painter.drawPie(m_boundingRect, 3.6 * (critical_ratio + major_ratio + minor_ratio), 16 * 3.6 * unknown_ratio);
  painter.setBrush(ngrt4n::COLOR_NORMAL);
  painter.drawPie(m_boundingRect, 3.6 * (critical_ratio + major_ratio + minor_ratio + unknown_ratio), 16 * 3.6 * ok_ratio);

  QString toolTip = QObject::tr("Normal: ")%QString::number(ok_count)%
      "/"%QString::number(m_nbStatsEntries)%" ("%QString::number(ok_ratio, 'f', 0)%"%)"
      %"\n"%QObject::tr("Minor: ")%QString::number(minor_count)%
      "/"%QString::number(m_nbStatsEntries)%" ("%QString::number(minor_ratio, 'f', 0)%"%)"
      %"\n"%QObject::tr("Major: ")%QString::number(major_count)%
      "/"%QString::number(m_nbStatsEntries)%" ("%QString::number(major_ratio, 'f', 0)%"%)"
      %"\n"%QObject::tr("Critical: ")%QString::number(critical_count)%"/"
      %QString::number(m_nbStatsEntries)%" ("%QString::number(critical_ratio, 'f', 0) %"%)"
      %"\n"%QObject::tr("Unknown: ")%QString::number(unknown_count)%
      "/"%QString::number(m_nbStatsEntries)%" ("%QString::number(unknown_ratio, 'f', 0)%"%)";
  setToolTip(toolTip);
}
