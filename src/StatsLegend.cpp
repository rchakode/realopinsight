/*
 * StatsLegend.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 13-05-2012												   #
#																		   #
# This file is part of NGRT4N (http://ngrt4n.com).						   #
#																		   #
# NGRT4N is free software: you can redistribute it and/or modify		   #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.									   #
#																		   #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of		   #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	           #
# GNU General Public License for more details.							   #
#																		   #
# You should have received a copy of the GNU General Public License		   #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.		   #
#--------------------------------------------------------------------------#
 */

#include "../include/Stats.hpp"


StatsLegend::StatsLegend(const QPoint & _left_top_corner, QWidget * _parent)
: QWidget(_parent)
{
	QFont font("Times", 10, QFont::NoAntialias);
	QFontMetrics font_metrics(font) ;

	qint32 x_top_corner = _left_top_corner.x() ;
	qint32 y_top_corner = _left_top_corner.y() ;
	qint32 x_label = x_top_corner + 15 ;
	qint32 char_w = font_metrics.charWidth("c", 0) ;

	normal.addRect(x_top_corner, y_top_corner, 10, 10) ;
	normal.addText(x_label, y_top_corner + 10, font, "normal") ;

	warning.addRect(x_top_corner, y_top_corner + 15 , 10, 10) ;
	warning.addText(x_label, y_top_corner + 25, font, "warning") ;

	critical.addRect(x_top_corner, y_top_corner + 30, 10, 10) ;
	critical.addText(x_label, y_top_corner + 40, font, "critical") ;

	unknown.addRect(x_top_corner, y_top_corner + 45, 10, 10) ;
	unknown.addText(x_label, y_top_corner + 55, font, "unknown") ;

	resize(x_label + 10 * char_w, y_top_corner + 65) ;
	setStyleSheet("background:transparent") ;
}

void StatsLegend::paintEvent(QPaintEvent *)
{
	QPainter painter(this) ;
	painter.setRenderHint(QPainter::Antialiasing) ;

	painter.setBrush( OK_COLOR ) ;
	painter.drawPath(normal) ;

	painter.setBrush( WARNING_COLOR ) ;
	painter.drawPath(warning) ;

	painter.setBrush( CRITICAL_COLOR) ;
	painter.drawPath(critical) ;

	painter.setBrush( UNKNOWN_COLOR ) ;
	painter.drawPath(unknown) ;
}

QSize StatsLegend::minimumSizeHint() const
		{
	return QSize(250, 300) ;
		}

 QSize StatsLegend::sizeHint() const
		{
	return QSize(250, 300) ;
		}
