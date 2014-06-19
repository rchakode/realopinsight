/*
 * PieChart.hpp
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

#ifndef PieChart_HPP_
#define PieChart_HPP_

#include "StatsLegend.hpp"
#include "GuiUtils.hpp"
#include "ChartBase.hpp"

class PieChart : public QWidget, public ChartBase
{
public:
  PieChart(const QRectF &  = QRectF(50, 50, 150, 100), QWidget* = 0 );
  virtual ~PieChart();
  QRectF getBoundingRect() {return m_boundingRect ; }

protected:
  virtual void paintEvent(QPaintEvent*);

private:
  QRectF m_boundingRect;
  StatsLegend* m_legend;
};

#endif /* PieChart_HPP_ */
