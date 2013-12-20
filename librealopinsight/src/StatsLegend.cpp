/*
 * StatsLegend.cpp
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

#include "StatsLegend.hpp"
#include <QColor>

const QColor StatsLegend::COLOR_NORMAL =  Qt::green;
const QColor StatsLegend::COLOR_MINOR = Qt::yellow;
const QColor StatsLegend::COLOR_MAJOR = QColor("#FF8C00");
const QColor StatsLegend::COLOR_CRITICAL = Qt::red;
const QColor StatsLegend::COLOR_UNKNOWN = Qt::lightGray;
const QColor StatsLegend::HIGHLIGHT_COLOR = QColor(255, 255, 200);
const qint32 StatsLegend::CHART_WIDTH=200;
const qint32 StatsLegend::CHART_HEIGHT=150;

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

  painter.setBrush(COLOR_NORMAL);
  painter.drawPath(normal);

  painter.setBrush(COLOR_MINOR);
  painter.drawPath(minor);

  painter.setBrush(COLOR_MAJOR);
  painter.drawPath(major);

  painter.setBrush(COLOR_CRITICAL);
  painter.drawPath(critical);

  painter.setBrush(COLOR_UNKNOWN);
  painter.drawPath(unknown);
}

QSize StatsLegend::minimumSizeHint() const
{
  return QSize(CHART_WIDTH, CHART_HEIGHT);
}

QSize StatsLegend::sizeHint() const
{
  return QSize(CHART_WIDTH, CHART_HEIGHT);
}
