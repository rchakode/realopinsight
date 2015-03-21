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

#include "dbo/DbObjects.hpp"
#include "Base.hpp"
#include <QList>
#include <fstream>
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
#include <Wt/WPaintedWidget>
#include <Wt/WStandardItemModel>
#include <Wt/WPainter>
#include <Wt/WSvgImage>

class SLADataManager
{
public:
  SLADataManager(const QosDataList& data);

  double normalDuration(void) const {return m_normalDuration;}
  double minorDuration(void) const {return m_minorDuration;}
  double majorDuration(void) const {return m_majorDuration;}
  double criticalDuration(void) const {return m_criticalDuration;}
  double unknownDuration(void) const {return m_unknownDuration;}
  double totalDuration(void) const {return m_totalDuration;}

private:
  struct TimeStatusT {
    long timestamp;
    int status;
  };
  typedef QList<TimeStatusT> TimeStatusesT;
  TimeStatusesT m_plottingData;
  long m_normalDuration;
  long m_minorDuration;
  long m_majorDuration;
  long m_criticalDuration;
  long m_unknownDuration;
  long m_totalDuration;

  void processData(const QosDataList& data);
  //FIXME: std::string slaText(void) {return QObject::tr("SLA: %1\%").arg(QString::number(m_slaNormal,'f',2)).toStdString();}
};





class RawQosTrendsChart : public Wt::Chart::WCartesianChart
{
public:
  RawQosTrendsChart(const std::string& viewName, const QosDataList& data, Wt::WContainerWidget* parent=0);
  std::string viewName() const {return m_viewName;}
  void updateData(const QosDataList& data);



private:
  std::string m_viewName;
  Wt::WStandardItemModel* m_model;


  Wt::WFont customTitleFont(void);
  void setChartTitle(void);
};


#endif // WEBBIREPORTBUILDER_HPP
