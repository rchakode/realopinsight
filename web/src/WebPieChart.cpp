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
#include <Wt/WStandardItem>
#include <QString>
#include <QDebug>
#include <Wt/WPainter>
#include <Wt/WScrollBar>



WebPieChart::WebPieChart(int dataType)
  : Wt::WTemplate(Wt::WString::tr("chart.tpl")),
    ChartBase()
{
  setDataType(dataType);
  setupChartPalette();
  setupPieChartModel();
  bindFormWidgets();
  setupChartStyle();
}

WebPieChart::WebPieChart(void)
  : WebPieChart(RawData) // delegating construction
{
}


WebPieChart::~WebPieChart()
{
  unbindFormWidgets();
}


void WebPieChart::setupChartPalette(void)
{
  WebChartPalette* palette = new WebChartPalette();
  m_piechart.setPalette(palette); //take the ownership of the palette pointer
}


void WebPieChart::setupPieChartModel(void)
{
  Wt::WStandardItemModel* model = new Wt::WStandardItemModel();

  model->insertColumns(model->columnCount(), 2);
  model->setHeaderData(0, Wt::WString("Severity"));
  model->setHeaderData(1, Wt::WString("Count"));
  model->insertRows(model->rowCount(), 5);

  m_piechart.setModel(model); // take ownership of the pointer
}


void WebPieChart::setupChartStyle(void)
{
  resize(ngrt4n::CHART_WIDTH, ngrt4n::CHART_HEIGHT);  // WPaintedWidget must be given an explicit size.
  setMargin(0, Wt::Top);
  setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);

  m_piechart.setLabelsColumn(0);    // Set the column that holds the labels.
  m_piechart.setDataColumn(1);      // Set the column that holds the data.
  m_piechart.setDisplayLabels(Wt::Chart::NoLabels);
  m_piechart.setPerspectiveEnabled(true, 0.2); // Enable a 3D and shadow effect.
  m_piechart.setShadowEnabled(true);
  m_piechart.setPlotAreaPadding(0, Wt::All);
}

void WebPieChart::unbindFormWidgets(void)
{
  m_chartLegendBarTpl.takeWidget("unknown-count");
  m_chartLegendBarTpl.takeWidget("critical-count");
  m_chartLegendBarTpl.takeWidget("major-count");
  m_chartLegendBarTpl.takeWidget("minor-count");
  m_chartLegendBarTpl.takeWidget("normal-count");
  takeWidget("chart-legend-bar");
  takeWidget("chart-content");
}

void WebPieChart::bindFormWidgets(void)
{
  m_chartLegendBarTpl.setTemplateText(Wt::WString::tr("chart-legend-bar.tpl"));
  m_chartLegendBarTpl.bindWidget("unknown-count", &m_legendBadges[ngrt4n::Unknown]);
  m_chartLegendBarTpl.bindWidget("critical-count", &m_legendBadges[ngrt4n::Critical]);
  m_chartLegendBarTpl.bindWidget("major-count", &m_legendBadges[ngrt4n::Major]);
  m_chartLegendBarTpl.bindWidget("minor-count", &m_legendBadges[ngrt4n::Minor]);
  m_chartLegendBarTpl.bindWidget("normal-count", &m_legendBadges[ngrt4n::Normal]);
  m_chartLegendBarTpl.setHidden(true);

  bindWidget("chart-legend-bar", &m_chartLegendBarTpl);
  bindWidget("chart-content", &m_piechart);
}


void WebPieChart::repaint()
{
  for(auto it = std::begin(m_statsData); it != std::end(m_statsData); ++it) {
    m_piechart.model()->setData(it.key(), 0, Severity(it.key()).toString().toStdString());
    m_piechart.model()->setData(it.key(), 1, it.value());
    m_legendBadges[it.key()].setText(QString::number(it.value()).toStdString());
  }

  if (m_dataType == SLAData) {
    m_chartLegendBarTpl.setHidden(true);
    m_piechart.setTitle(QObject::tr("SLA: %1%")
                        .arg(QString::number(m_severityRatio[ngrt4n::Normal]))
                        .toStdString()
                        );
    if (m_severityRatio[ngrt4n::Normal] > 0
        && m_severityRatio[ngrt4n::Normal] < 100) {
      m_piechart.setExplode(ngrt4n::Normal, 0.3);
    }
    m_piechart.setToolTip(ChartBase::defaultTooltipText());
  } else {
    m_chartLegendBarTpl.setHidden(false);
    m_piechart.setToolTip(ChartBase::defaultTooltipText());
  }
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
      QString("Normal: %1% - %2%9\n"
              "Minor: %3%  - %4%9\n"
              "Major: %5%  - %6%9\n"
              "Critical: %7%  - %8%9\n").arg
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
