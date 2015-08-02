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

#include "ChartBase.hpp"
#include "WebUtils.hpp"
#include <Wt/WStandardItemModel>
#include <Wt/Chart/WPieChart>
#include <Wt/WColor>
#include <Wt/Chart/WChartPalette>
#include <Wt/WScrollArea>
#include <Wt/WText>
#include <Wt/WTemplate>
#include <Wt/WPen>


class WebChartPalette : public Wt::Chart::WChartPalette
{
public:
  WebChartPalette(void) { }
  virtual Wt::WBrush brush (int index) const { return Wt::WBrush(ngrt4n::severityWColor(index));}
  virtual Wt::WPen borderPen (int index) const { return Wt::WPen(Wt::WColor(255, 255, 255, 0)); }
  Wt::WPen strokePen (int) const { /* TODO: check value first */ return Wt::WPen(Wt::WColor(255, 255, 255, 1)); }
  Wt::WColor fontColor (int index) const { /* TOTO: check value first */ return Wt::WColor(255, 255, 255, 0);}
  virtual Wt::WColor color (int index) const { return Wt::WColor(255, 255, 255, 0); }
};


class WebPieChart : public Wt::WTemplate, public ChartBase
{
public:
  WebPieChart(int dataType);
  WebPieChart(void);
  virtual ~WebPieChart();
  void repaint();
  void setDataType(int dataType) {ChartBase::setDataType(dataType);}
  std::string defaultTooltipText(void);

private:
  Wt::Chart::WPieChart m_piechart;
  std::map<int, Wt::WText> m_legendBadges;
  Wt::WTemplate m_mainChartTpl;
  Wt::WTemplate m_chartLegendBarTpl;
  void bindFormWidgets(void);
  void unbindFormWidgets(void);

  void setupChartPalette(void);
  void setupChartStyle(void);
  void setupPieChartModel(void);
};

#endif // WEBPIECHART_HPP
