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

class QosTrendsChart : public  Wt::WPaintedWidget
{
public:
  QosTrendsChart(const std::string& viewName, const QosDataList& data, Wt::WContainerWidget* parent=0);
  std::string viewName() const {return m_viewName;}
  void updateData(const QosDataList& data);

protected:
  virtual void paintEvent(Wt::WPaintDevice * 	paintDevice);

private:
  struct TimeStatusT {
    long timestamp;
    int status;
  };
  typedef QList<TimeStatusT> TimeStatusesT;
  TimeStatusesT m_plottingData;
  std::string m_viewName;
  double m_slaNormal;
  double m_slaMinor;
  double m_slaMajor;
  double m_slaCritical;
  double m_slaUnknown;
  long m_timeNormal;
  long m_timeMinor;
  long m_timeMajor;
  long m_timeCritical;
  long m_timeUnknown;
  double m_xScalingFactor;
  TimeStatusT m_firstPoint;


  void processPlottingData(const QosDataList& data);
  void addRangeToolTip(double x1, double x2, long t1, long t2);
  std::string slaText(void) {return QObject::tr("QoS Trend - SLA: %1\%").arg(QString::number(m_slaNormal,'f',2)).toStdString();}
  double computeXAxis(long timestamp) {return timestamp - m_firstPoint.timestamp;}
  void paintSlaBar(Wt::WPaintDevice* paintDevice);
  void paintSlaChord(Wt::WPaintDevice* paintDevice);
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
