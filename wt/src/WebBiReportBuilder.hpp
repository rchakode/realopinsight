/*
 * WebBiReportBuilder.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 07-10-2014                                                  #
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

#ifndef WEBBIREPORTBUILDER_HPP
#define WEBBIREPORTBUILDER_HPP

#include <Wt/Chart/WCartesianChart>
#include <Wt/Chart/WDataSeries>
#include <Wt/WAbstractItemModel>
#include <Wt/WAbstractItemView>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WDate>
#include <Wt/WEnvironment>
#include <Wt/WPaintedWidget>
#include <Wt/WItemDelegate>
#include <Wt/WShadow>
#include <Wt/WStandardItemModel>
#include "DbObjects.hpp"


class WebBiReportBuilder : public Wt::Chart::WCartesianChart
{
public:
  WebBiReportBuilder(const std::string& name,
                     const std::list<DbQosDataT>& data,
                     Wt::WContainerWidget* parent=0);

private:
  Wt::WStandardItemModel* m_model;
  int m_row;
};

#endif // WEBBIREPORTBUILDER_HPP
