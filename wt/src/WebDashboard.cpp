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
#include "WebChart.hpp"

WebDashboard::WebDashboard(const qint32& _userRole, const QString& _config)
  : DashboardBase(_userRole, _config),
    m_widget(new Wt::WContainerWidget()),
    m_tree(new WebTree(m_cdata)),
    m_map(new WebMap(m_cdata)),
    m_msgConsole(new WebMsgConsole())
{
  Wt::WContainerWidget* treeContainer(new Wt::WContainerWidget());
  Wt::WContainerWidget* mapMsgContainer(new Wt::WContainerWidget());
  Wt::WContainerWidget* mapContainer(new Wt::WContainerWidget());
  Wt::WContainerWidget* msgContainer(new Wt::WContainerWidget());
  Wt::WHBoxLayout* mainLayout(new Wt::WHBoxLayout(m_widget));
  Wt::WVBoxLayout* treeLayout(new Wt::WVBoxLayout(treeContainer));
  Wt::WVBoxLayout* mapMsgLayout(new Wt::WVBoxLayout(mapMsgContainer));
  Wt::WVBoxLayout* mapLayout(new Wt::WVBoxLayout(mapContainer));
  Wt::WVBoxLayout* msgLayout(new Wt::WVBoxLayout(msgContainer));
  Wt::WPanel* treePanel = new Wt::WPanel();
  Wt::WPanel* mapPanel = new Wt::WPanel();
  Wt::WPanel* msgPanel = new Wt::WPanel();

  m_widget->setPadding(Wt::WLength(0), Wt::All);
  m_widget->setStyleClass("maincontainer");
  mapMsgContainer->setStyleClass("panel");
  mapContainer->setStyleClass("panel");
  mapPanel->setStyleClass("panel");
  msgContainer->setStyleClass("panel");
  msgPanel->setStyleClass("panel");
  treeContainer->setStyleClass("panel");
  treePanel->setStyleClass("panel");

  mainLayout->setContentsMargins(0, 0, 0, 0);
  treeLayout->setContentsMargins(0, 0, 0, 0);
  mapMsgLayout->setContentsMargins(0, 0, 0, 0);
  mapLayout->setContentsMargins(0, 0, 0, 0);
  msgLayout->setContentsMargins(0, 0, 0, 0);

  mainLayout->setSpacing(2);
  treeLayout->setSpacing(0);
  mapMsgLayout->setSpacing(2);
  mapLayout->setSpacing(0);
  msgLayout->setSpacing(0);

  treePanel->setCentralWidget(m_tree);
  treeLayout->addWidget(treePanel);
  treeLayout->addWidget(new WebChart());
  treePanel->setTitle(QObject::tr("Service Tree").toStdString());

  mapLayout->addWidget(m_map->get());
  mapPanel->setCentralWidget(mapContainer);

  msgLayout->addWidget(m_msgConsole);
  msgPanel->setCentralWidget(msgContainer);

  mapMsgLayout->addWidget(mapPanel);
  mapMsgLayout->addWidget(msgPanel);
  //  rightVLayout->setStretchFactor(mapPanel, 3);
  //  rightVLayout->setStretchFactor(msgPanel, 1);

  mainLayout->addWidget(treeContainer);
  mainLayout->addWidget(mapMsgContainer);

  treeLayout->setResizable(0);
  mainLayout->setResizable(0);
  mainLayout->setResizable(1);
  mainLayout->setResizable(0);
  mainLayout->setResizable(1);
  mapMsgLayout->setResizable(0);
  mapMsgLayout->setResizable(1);

  load(_config);
}

WebDashboard::~WebDashboard()
{
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
  if (!m_showOnlyTroubles
      || (m_showOnlyTroubles && _node.severity != MonitorBroker::Normal))
  {
    m_msgConsole->updateNodeMsg(_node);
  }
}

void WebDashboard::updateChart(void)
{
  //FIXME: to be implemented
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
