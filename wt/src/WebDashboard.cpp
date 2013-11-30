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
  m_widget->resize(Wt::WLength(100, Wt::WLength::Percentage), Wt::WLength(100, Wt::WLength::Percentage));
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
  Wt::WContainerWidget* treeContainer(new Wt::WContainerWidget(m_widget));
  Wt::WContainerWidget* mapMsgContainer(new Wt::WContainerWidget(m_widget));
  Wt::WContainerWidget* mapContainer(new Wt::WContainerWidget(m_widget));
  Wt::WContainerWidget* msgContainer(new Wt::WContainerWidget(m_widget));
  Wt::WHBoxLayout* mainLayout(new Wt::WHBoxLayout(m_widget));
  Wt::WVBoxLayout* leftSubMainLayout(new Wt::WVBoxLayout(treeContainer));
  Wt::WVBoxLayout* rightSubMainLayout(new Wt::WVBoxLayout(msgContainer));
  Wt::WVBoxLayout* mapMsgLayout(new Wt::WVBoxLayout(mapMsgContainer));
  Wt::WVBoxLayout* mapLayout(new Wt::WVBoxLayout(mapContainer));
  Wt::WPanel* treePanel(new Wt::WPanel());
  Wt::WPanel* mapPanel(new Wt::WPanel());
  Wt::WPanel* msgPanel(new Wt::WPanel());

  m_widget->setPadding(Wt::WLength(0), Wt::All);
  m_widget->addStyleClass("dashboard");
  mapMsgContainer->addStyleClass("panel");
  mapContainer->addStyleClass("panel");
  mapPanel->addStyleClass("panel");
  msgContainer->addStyleClass("panel");
  msgPanel->addStyleClass("panel");
  treeContainer->addStyleClass("panel");
  treePanel->addStyleClass("panel");

  mainLayout->setContentsMargins(0, 0, 0, 0);
  leftSubMainLayout->setContentsMargins(0, 0, 0, 0);
  mapMsgLayout->setContentsMargins(0, 0, 0, 0);
  mapLayout->setContentsMargins(0, 0, 0, 0);
  rightSubMainLayout->setContentsMargins(0, 0, 0, 0);

  mainLayout->setSpacing(2);
  leftSubMainLayout->setSpacing(2);
  rightSubMainLayout->setSpacing(0);
  mapMsgLayout->setSpacing(2);
  mapLayout->setSpacing(0);

  treePanel->setCentralWidget(m_tree);
  leftSubMainLayout->addWidget(treePanel);
  leftSubMainLayout->setStretchFactor(treePanel, 3);
  leftSubMainLayout->addWidget(m_chart->get());
  leftSubMainLayout->setStretchFactor(treePanel, 1);
  treePanel->setTitle(QObject::tr("Service Tree").toStdString());

  mapLayout->addWidget(m_map->get());
  mapPanel->setCentralWidget(mapContainer);

  rightSubMainLayout->addWidget(m_msgConsole);
  msgPanel->setCentralWidget(msgContainer);

  mapMsgLayout->addWidget(mapPanel);
  //mapMsgLayout->setStretchFactor(mapPanel, 3);
  mapMsgLayout->addWidget(msgPanel);
  //mapMsgLayout->setStretchFactor(msgPanel, 1);

  mainLayout->addWidget(treeContainer);
  mainLayout->addWidget(mapMsgContainer);

  leftSubMainLayout->setResizable(0);
  mainLayout->setResizable(0);
  mainLayout->setResizable(1);
  mainLayout->setResizable(0);
  mainLayout->setResizable(1);
  mapMsgLayout->setResizable(0);
  mapMsgLayout->setResizable(1);
}
