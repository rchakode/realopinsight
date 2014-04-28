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

  updateSeverityInfo();

  int unknown_angle = 16 * m_severityRatio[ngrt4n::Unknown];
  int critical_angle= 16 * m_severityRatio[ngrt4n::Critical];
  int major_angle = 16 * m_severityRatio[ngrt4n::Major];
  int minor_angle = 16 * m_severityRatio[ngrt4n::Minor];
  int ok_angle = 16 * m_severityRatio[ngrt4n::Normal];

  painter.setPen(Qt::transparent);
  painter.setBrush(ngrt4n::COLOR_CRITICAL);
  painter.drawPie(m_boundingRect, 0, 3.6 * critical_angle);
  painter.setBrush(ngrt4n::COLOR_MAJOR);
  painter.drawPie(m_boundingRect, 3.6 * critical_angle, 3.6 * major_angle);
  painter.setBrush(ngrt4n::COLOR_MINOR);
  painter.drawPie(m_boundingRect, 3.6 * (critical_angle + major_angle), 16 * 3.6 * minor_angle);
  painter.setBrush(ngrt4n::COLOR_UNKNOWN);
  painter.drawPie(m_boundingRect, 3.6 * (critical_angle + major_angle + minor_angle), 3.6 * unknown_angle);
  painter.setBrush(ngrt4n::COLOR_NORMAL);
  painter.drawPie(m_boundingRect, 3.6 * (critical_angle + major_angle + minor_angle + unknown_angle), 3.6 * ok_angle);

  setToolTip(ChartBase::buildTooltipText());
}
