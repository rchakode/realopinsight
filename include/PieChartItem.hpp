/*
 * ChartItem.hpp
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

#ifndef PieChartItem_HPP_
#define PieChartItem_HPP_

#include "Base.hpp"


class PieChartItem : public QWidget
{
	Q_OBJECT

public:
	PieChartItem(const QRectF & _bounding_rect = QRectF(10.0, 10.0, 100.0, 80.0),
			const qreal & _start_angle = 0, const qreal & _proportion = 0,
			const QColor & _color = Qt::green, QWidget * _parent = 0);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

public slots:
void setFillRule(Qt::FillRule rule);
void setFillGradient(const QColor &color1, const QColor &color2);

protected:
void paintEvent(QPaintEvent *event);

private:
QPainterPath path;
QRectF pieChartBoundingRect ;
qreal startAngle ;
qreal proportion ;
QColor fillColor1;
QColor fillColor2;

void setPath(void) ;

};

#endif /* PieChartItem_HPP_ */
