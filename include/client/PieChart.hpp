/*
 * PieChart.hpp
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

#ifndef PieChart_HPP_
#define PieChart_HPP_

#include "PieChartItem.hpp"
#include "StatsLegend.hpp"

class PieChart : public  QWidget
{
public:

	PieChart(const QRectF &  = QRectF(50, 50, 150, 100), QWidget* = 0 );
	virtual ~PieChart();

	void update(const CheckStatusCountT & _check_status_count, const qint32 _check_count, QString & ) ;

private:
	QRectF boundingRect ;
	QMap<qint32, PieChartItem*> slices ;

	StatsLegend* legend ;
};

#endif /* PieChart_HPP_ */
