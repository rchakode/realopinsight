/*
# Stats.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-03-2014                                                  #
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


#ifndef SNAVSTATS_HPP_
#define SNAVSTATS_HPP_

#include "PieChart.hpp"


class Chart : public QWidget
{
public:
  static const qint32 DefaultWidth ;
  static const qint32 DefaultHeight ;

  Chart();
  virtual ~Chart();
  void update(const CheckStatusCountT& _stCount, qint32 _count) ;
  QSize minimumSizeHint() const;
  QSize sizeHint() const;

protected:
  void paintEvent(QPaintEvent * event)
  {
    QPainter p;
    p.scale(m_scale, m_scale);
    QWidget::paintEvent(event);
  }
  void wheelEvent(QWheelEvent * event)
  {
    m_scale+=(event->delta()/120); //FIXME: use the scaling factors
  }

private:
  PieChart* m_pieChart;
  double m_scale;
};

#endif /* SNAVSTATS_HPP_ */
