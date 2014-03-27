/*
 * StatsLegend.cpp
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

#include "StatsLegend.hpp"
#include "GuiUtils.hpp"
#include "utilsCore.hpp"
#include <QColor>

StatsLegend::StatsLegend(const QPoint & _left_top_corner, QWidget * _parent)
  : QWidget(_parent)
{
  QFont font("Times", 10, QFont::NoAntialias);
  QFontMetrics font_metrics(font);
  qint32 x_top_corner = _left_top_corner.x();
  qint32 y_top_corner = _left_top_corner.y();
  qint32 x_label = x_top_corner + 15;
  qint32 char_w = font_metrics.charWidth("c", 0);

  normal.addRect(x_top_corner, y_top_corner, 10, 10);
  normal.addText(x_label, y_top_corner + 10, font, QObject::tr("normal"));
  minor.addRect(x_top_corner, y_top_corner + 15 , 10, 10);
  minor.addText(x_label, y_top_corner + 25, font, QObject::tr("minor"));
  major.addRect(x_top_corner, y_top_corner + 30 , 10, 10);
  major.addText(x_label, y_top_corner + 40, font, QObject::tr("major"));
  critical.addRect(x_top_corner, y_top_corner + 45, 10, 10);
  critical.addText(x_label, y_top_corner + 55, font, QObject::tr("critical"));
  unknown.addRect(x_top_corner, y_top_corner + 60, 10, 10);
  unknown.addText(x_label, y_top_corner + 70, font, QObject::tr("unknown"));

  resize(x_label + 10 * char_w, y_top_corner + 80);
  setStyleSheet("background:transparent");
}

void StatsLegend::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  painter.setBrush(ngrt4n::COLOR_NORMAL);
  painter.drawPath(normal);

  painter.setBrush(ngrt4n::COLOR_MINOR);
  painter.drawPath(minor);

  painter.setBrush(ngrt4n::COLOR_MAJOR);
  painter.drawPath(major);

  painter.setBrush(ngrt4n::COLOR_CRITICAL);
  painter.drawPath(critical);

  painter.setBrush(ngrt4n::COLOR_UNKNOWN);
  painter.drawPath(unknown);
}

QSize StatsLegend::minimumSizeHint() const
{
  return QSize(ngrt4n::CHART_WIDTH, ngrt4n::CHART_HEIGHT);
}

QSize StatsLegend::sizeHint() const
{
  return QSize(ngrt4n::CHART_WIDTH, ngrt4n::CHART_HEIGHT);
}
