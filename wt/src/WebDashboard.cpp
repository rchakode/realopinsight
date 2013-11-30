/*
 * WebDashboard.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 19-09-2013                                                 #
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

#include <fstream>
#include <iostream>
#include <Wt/WPanel>
#include <Wt/WPointF>
#include <Wt/WText>
#include <Wt/WLink>
#include <Wt/WImage>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "WebDashboard.hpp"
#include "Base.hpp"
#include "utilsClient.hpp"
#include <QDebug>

WebDashboard::WebDashboard(const qint32& _userRole, const QString& _config)
  : DashboardBase(_userRole, _config),
    m_widget(new Wt::WContainerWidget()),
    m_tree(new WebTree(m_cdata)),
    m_map(new WebPieMap(m_cdata)),
    m_msgConsole(new WebMsgConsole()),
    m_chart(new WebPieChart())
{
  setupUI();
  load(_config);
  //  m_widget->setJavaScriptMember(
  //        "wtResize",
  //        "function(self, width, height) { "
  //        "$('maincontainer').height=height;"
  //        "$('REALOPINSIGHT_TREE_BOARD').height=height*0.50;"
  //        "$('REALOPINSIGHT_MAP_BOARD').height=height*0.50;"
  //        "$('REALOPINSIGHT_CHART_BOARD').height=height*0.50;"
  //        "$('REALOPINSIGHT_MSG_BOARD').height=height*0.50;"
  //        "}");
}

WebDashboard::~WebDashboard()
{
  delete m_chart;
  delete m_tree;
  delete m_map;
  delete m_msgConsole;
  delete m_widget;
}

void WebDashboard::buildTree(void)
{
  m_tree->build();
}

void WebDashboard::updateTree(const NodeT& _node, const QString& _tip)
{
  m_tree->updateNodeItem(_node, _tip);
}

void WebDashboard::updateMsgConsole(const NodeT& _node)
{
  if (! m_showOnlyTroubles ||
      (m_showOnlyTroubles && _node.severity != MonitorBroker::Normal))
  {
    m_msgConsole->updateNodeMsg(_node);
  }
}

void WebDashboard::updateChart(void)
{
  for(CheckStatusCountT::ConstIterator it = m_cdata->check_status_count.begin(),
      end = m_cdata->check_status_count.end(); it != end; ++it) {
    m_chart->setSeverityData(it.key(), it.value());
  }
}

void WebDashboard::buildMap(void)
{
  m_map->drawMap();
}


void WebDashboard::updateMap(const NodeT& _node, const QString& _tip)
{
  m_map->updateNode(_node, _tip);
}

void WebDashboard::updateMap(void)
{
  m_map->drawMap();
}

void WebDashboard::setupUI(void)
{
  Wt::WHBoxLayout* mainLayout(new Wt::WHBoxLayout(m_widget));
  Wt::WVBoxLayout* leftSubMainLayout(new Wt::WVBoxLayout());
  Wt::WVBoxLayout* rightSubMainLayout(new Wt::WVBoxLayout());

  m_widget->setId("REALOPINSIGHT_MAIN_DASHBOARD");

  mainLayout->setContentsMargins(0, 0, 0, 0);
  leftSubMainLayout->setContentsMargins(0, 0, 0, 0);
  rightSubMainLayout->setContentsMargins(0, 0, 0, 0);

  mainLayout->setSpacing(2);
  leftSubMainLayout->setSpacing(2);
  rightSubMainLayout->setSpacing(2);

  leftSubMainLayout->addWidget(m_tree);
  leftSubMainLayout->addWidget(m_chart->get());

  rightSubMainLayout->addWidget(m_map->get());
  rightSubMainLayout->addWidget(m_msgConsole);

  m_tree->resize(Wt::WLength::Auto, Wt::WLength(350, Wt::WLength::Pixel));
  m_map->get()->resize(Wt::WLength::Auto, Wt::WLength(350, Wt::WLength::Pixel));
  m_chart->get()->resize(Wt::WLength::Auto, Wt::WLength(200, Wt::WLength::Pixel));
  m_msgConsole->resize(Wt::WLength::Auto, Wt::WLength(200, Wt::WLength::Pixel));

  mainLayout->addLayout(leftSubMainLayout);
  mainLayout->addLayout(rightSubMainLayout);

  leftSubMainLayout->setResizable(0);
  mainLayout->setResizable(0);
  mainLayout->setResizable(1);
  mainLayout->setResizable(0);
  mainLayout->setResizable(1);
  rightSubMainLayout->setResizable(0);
  rightSubMainLayout->setResizable(1);
}
