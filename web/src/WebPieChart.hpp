/*
 * WebPieChart.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
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

#ifndef WEBPIECHART_HPP
#define WEBPIECHART_HPP

#include <Wt/WStandardItemModel>
#include <Wt/Chart/WPieChart>
#include <Wt/WColor>
#include <Wt/WPaintDevice>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include "ChartBase.hpp"

class WebPieChart : public Wt::Chart::WPieChart, public ChartBase
{
public:
  WebPieChart();
  virtual ~WebPieChart();
  Wt::WScrollArea* getContainerArea(void) const  {return m_scrollArea;}
  void repaint();
  void setDataType(int dataType) {ChartBase::setDataType(dataType);}

private:
  Wt::WStandardItemModel* m_model;
  Wt::WScrollArea* m_scrollArea;
  std::map<int, Wt::WText*> m_legendBadges;
  Wt::WTemplate* m_chartLegendBar;
  Wt::WTemplate* createChartLegendBar(void);
  Wt::WTemplate* createChartTemplate(void);
};

#endif // WEBPIECHART_HPP
