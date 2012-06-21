/*
 * Stats.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */

#include "Stats.hpp"


const qint32 Stats::DefaultWidth=300 ;
const qint32 Stats::DefaultHeight=175 ;

Stats::Stats()
: QWidget(),
pieChart(new PieChart(QRectF(2, 2, 125, 125), this))
{
	resize(pieChart->size()) ;
	setStyleSheet("background:transparent") ;
}

Stats::~Stats()
{
	delete pieChart ;
}

QSize Stats::minimumSizeHint() const
		{
	return QSize(200, 100) ;
		}

 QSize Stats::sizeHint() const
		{
	return QSize(DefaultWidth, DefaultHeight) ;
		}

QString Stats::update(const CheckStatusCountT & _check_status_count, const qint32 & _check_count)
{
	return pieChart->update(_check_status_count, _check_count) ;
}
