/*
 * Stats.cpp
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

#include "Chart.hpp"


const qint32 Chart::DefaultWidth=300;
const qint32 Chart::DefaultHeight=175;

Chart::Chart()
  : QWidget(),
    m_pieChart(new PieChart(QRectF(2, 2, 125, 125), this))
{
  resize(m_pieChart->size());
}

Chart::~Chart()
{
  delete m_pieChart;
}

QSize Chart::minimumSizeHint() const
{
  return QSize(200, 100);
}

QSize Chart::sizeHint() const
{
  return QSize(DefaultWidth, DefaultHeight);
}

void Chart::update(const CheckStatusCountT& _stCount, qint32 _count)
{
  QString toolTip;
  m_pieChart->update(_stCount, _count, toolTip);
  setToolTip(toolTip);
}
