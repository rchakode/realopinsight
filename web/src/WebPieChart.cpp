/*
 * WebPieChart.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
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
#include "WebPieChart.hpp"
#include "utilsCore.hpp"
#include <Wt/WStandardItem.h>
#include <QString>
#include <Wt/WPainter.h>



WebPieChart::WebPieChart(int dataType)
  : ChartBase(),
    Wt::Chart::WPieChart()

{
  setDataType(dataType);
  setupChartPalette();
  setupPieChartModel();
  setupChartStyle();
  setTitleFont(ngrt4n::chartTitleFont());
}

WebPieChart::WebPieChart(void)
  : WebPieChart(RawData)
{
}


WebPieChart::~WebPieChart()
{
}


void WebPieChart::setupChartPalette(void)
{
  std::shared_ptr<WebChartPalette> palette = std::make_shared<WebChartPalette>();
  setPalette(palette);
}


void WebPieChart::setupPieChartModel(void)
{
  auto model = std::make_unique<Wt::WStandardItemModel>();
  m_modelRef = model.get();
  model->insertColumns(m_modelRef->columnCount(), 2);
  model->setHeaderData(0, Wt::WString("Severity"));
  model->setHeaderData(1, Wt::WString("Count"));
  model->insertRows(m_modelRef->rowCount(), 5);
  setModel(std::move(model));
}


void WebPieChart::setupChartStyle(void)
{
  resize(ngrt4n::CHART_WIDTH, ngrt4n::CHART_HEIGHT);  // WPaintedWidget must be given an explicit size.
  setMargin(0, Wt::Side::Top);
  setPlotAreaPadding(0, Wt::AllSides);
  setDisplayLabels(Wt::Chart::LabelOption::None);
  setPerspectiveEnabled(true, 0.2); // Enable a 3D and shadow effect.
  setShadowEnabled(true);
  setLabelsColumn(0);    // Set the column that holds the labels.
  setDataColumn(1);      // Set the column that holds the data.
}


void WebPieChart::repaint()
{
  for(auto it = std::begin(m_statsData); it != std::end(m_statsData); ++it) {
    m_modelRef->setData(it.key(), 0, Severity(it.key()).toString().toStdString());
    m_modelRef->setData(it.key(), 1, it.value());
  }

  if (m_dataType == SLAData) {
    setTitle(QObject::tr("SLA: %1%")
             .arg(QString::number(m_severityRatio[ngrt4n::Normal]))
             .toStdString()
             );
    if (m_severityRatio[ngrt4n::Normal] > 0
        && m_severityRatio[ngrt4n::Normal] < 100) {
      setExplode(ngrt4n::Normal, 0.3);
    }
    setToolTip(ChartBase::defaultTooltipText());
  } else {
    setToolTip(ChartBase::defaultTooltipText());
  }
  refresh();
}


std::string WebPieChart::defaultTooltipText(void)
{
  QString normalText = "";
  QString minorText = "";
  QString majorText = "";
  QString criticalText = "";
  QString unknownText = "";
  QString totalText = "";
  if (m_dataType == SLAData) {
    normalText   = timeFromSeconds(m_statsData[ngrt4n::Normal]);
    minorText    = timeFromSeconds(m_statsData[ngrt4n::Minor]);
    majorText    = timeFromSeconds(m_statsData[ngrt4n::Major]);
    criticalText = timeFromSeconds(m_statsData[ngrt4n::Critical]);
    unknownText  = timeFromSeconds(m_statsData[ngrt4n::Unknown]);
  } else {
    normalText   = QString::number(m_statsData[ngrt4n::Normal]);
    minorText    = QString::number(m_statsData[ngrt4n::Minor]);
    majorText    = QString::number(m_statsData[ngrt4n::Major]);
    criticalText = QString::number(m_statsData[ngrt4n::Critical]);
    unknownText  = QString::number(m_statsData[ngrt4n::Unknown]);
    totalText    = "/"+QString::number(m_dataCount);
  }
  QString tooltip =
      QString("Normal: %1% (%2%9)\n"
              "Minor: %3% (%4%9)\n"
              "Major: %5% (%6%9)\n"
              "Critical: %7% (%8%9)\n").arg
      (QString::number(m_severityRatio[ngrt4n::Normal],'f',0),
      normalText,
       QString::number(m_severityRatio[ngrt4n::Minor],'f',0),
       minorText,
       QString::number(m_severityRatio[ngrt4n::Major],'f',0),
       majorText,
       QString::number(m_severityRatio[ngrt4n::Critical],'f',0),
       criticalText,
       totalText);

  return tooltip.append("Unknown: %1% - %2%3").arg(
        QString::number(m_severityRatio[ngrt4n::Unknown],'f',0),
        unknownText, totalText).toStdString();
}
