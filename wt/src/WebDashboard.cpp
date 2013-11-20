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

WebDashboard::WebDashboard(const qint32& _userRole, const QString& _config)
  : DashboardBase(_userRole, _config),
    m_widget(new Wt::WContainerWidget()),
    m_tree(new WebTree(m_cdata)),
    m_map(new WebMap(m_cdata)),
    m_msgConsole(new WebMsgConsole())
{
  Wt::WContainerWidget* treeContainer = new Wt::WContainerWidget();
  Wt::WContainerWidget* mapMsgContainer = new Wt::WContainerWidget();
  Wt::WContainerWidget* mapContainer = new Wt::WContainerWidget();
  Wt::WContainerWidget* msgContainer = new Wt::WContainerWidget();
  Wt::WVBoxLayout* mainLayout = new Wt::WVBoxLayout();
  Wt::WHBoxLayout* centralLayout = new Wt::WHBoxLayout();
  Wt::WVBoxLayout* treeLayout = new Wt::WVBoxLayout();
  Wt::WVBoxLayout* rightVLayout = new Wt::WVBoxLayout();
  Wt::WVBoxLayout* mapLayout = new Wt::WVBoxLayout();
  Wt::WVBoxLayout* msgLayout = new Wt::WVBoxLayout();
  Wt::WPanel* treePanel = new Wt::WPanel();
  Wt::WPanel* mapPanel = new Wt::WPanel();
  Wt::WPanel* msgPanel = new Wt::WPanel();

  m_widget->setStyleClass("maincontainer");
  treeContainer->setStyleClass("panel");
  mapMsgContainer->setStyleClass("panel");
  mapContainer->setStyleClass("panel");
  msgContainer->setStyleClass("panel");

  mainLayout->setContentsMargins(0, 0, 0, 0);
  centralLayout->setContentsMargins(0, 0, 0, 0);
  treeLayout->setContentsMargins(0, 0, 0, 0);
  rightVLayout->setContentsMargins(0, 0, 0, 0);
  mapLayout->setContentsMargins(0, 0, 0, 0);
  msgLayout->setContentsMargins(0, 0, 0, 0);

  mainLayout->setSpacing(0);
  centralLayout->setSpacing(0);
  treeLayout->setSpacing(0);
  rightVLayout->setSpacing(0);
  mapLayout->setSpacing(0);
  msgLayout->setSpacing(0);

  treePanel->setCentralWidget(m_tree);
  treeLayout->addWidget(treePanel);
  treeContainer->setLayout(treeLayout);
  treePanel->setTitle(QObject::tr("TV Explorer").toStdString());

  mapLayout->addWidget(m_map->get());
  mapContainer->setLayout(mapLayout);
  mapPanel->setCentralWidget(mapContainer);
 //mapPanel->setTitle(QObject::tr("Service Map").toStdString());

  msgLayout->addWidget(m_msgConsole);
  msgContainer->setLayout(msgLayout);
  msgPanel->setCentralWidget(msgContainer);
  //msgPanel->setTitle(QObject::tr("Message Console").toStdString());

  //FIXME: connect(m_msgConsole, SIGNAL(sizeChanged(int, int)), m_map, SLOT(msgPanelSizedChanged(int)));
  //connect(m_msgConsole, SIGNAL(sizeChanged(int, int)), m_map, SLOT(msgPanelSizedChanged(int)));

  mapMsgContainer->setLayout(rightVLayout);
  rightVLayout->addWidget(mapPanel);
  rightVLayout->addWidget(msgPanel);
  rightVLayout->setStretchFactor(mapPanel, 3);
  rightVLayout->setStretchFactor(msgPanel, 2);

  centralLayout->addWidget(treeContainer, 1);
  centralLayout->addWidget(mapMsgContainer, 3);

  mainLayout->addWidget(createMenuBarWidget(), 0);
  mainLayout->addLayout(centralLayout, 2);
  m_widget->setLayout(mainLayout);

  //FIXME: widget alignment
  centralLayout->setResizable(0);
  centralLayout->setResizable(1);
  rightVLayout->setResizable(0);
  rightVLayout->setResizable(1);

  load(_config);
//  mapPanel->resize(m_cdata->map_width,
//                   m_cdata->map_height);
}

WebDashboard::~WebDashboard()
{
  delete m_tree;
  delete m_map;
  delete m_msgConsole;
  delete m_widget;
}


void WebDashboard::load(const QString& _file)
{
  if (!_file.isEmpty()) {
    m_config = utils::getAbsolutePath(_file);
  }

  Parser parser(m_config, m_cdata);
  parser.process(true);
  parser.computeNodeCoordinates(1);
  m_tree->build();
  m_map->drawMap(false);
  updateViews();
}


void WebDashboard::updateDashboard(const NodeT& _node)
{
  qDebug() << _node.name;
}

void WebDashboard::finalizeUpdate(const SourceT& src)
{
  qDebug() << src.id;
}
void WebDashboard::updateNavTreeItemStatus(const NodeT& _node, const QString& _tip)
{
  qDebug() << _node.name << _tip;
}


void WebDashboard::updateMap(const NodeListT::iterator& _node, const QString& _tip)
{
  qDebug() << _node->name << _tip;
}



void WebDashboard::updateViews(void)
{
  //FIXME: updateViews(void)
  //  updateServicesStatuses();
  m_msgConsole->update(m_cdata->cnodes);
  //  map->update(m_cdata->bpnodes, m_cdata->cnodes, mapWidth, mapHeight);
  //  navTree->update(navTreeRoot); updateServiceTree();
}

void WebDashboard::updateServicesStatuses(void)
{
  //  //Fixme: loadConfig
  //  //if (! Ngrt4nConfigParser::loadNagiosCollectedData("examples/status.dat", m_cdata->cnodes) ) return;
  //  return; //FIXME:

  //  for(NodeListT sIt  = m_cdata->bpnodes.begin(); sIt != m_cdata->bpnodes.end(); ++sIt) {
  //    sIt->status_info.reset();
  //  }

  //  for(NodeListT::Iterator sIt  = m_cdata->cnodes.begin(); sIt != m_cdata->cnodes.end(); ++sIt) {
  //    if(sIt->id != "root") {
  //      NodeListT pIt = m_cdata->bpnodes.find(sIt->parent);
  //      pIt->status_info |= sIt->status_info;
  //      updateParentStatus(*pIt);
  //    }
  //  }
}

void WebDashboard::updateParentStatus(const NodeT& _service)
{
  NodeListT::Iterator pIt = m_cdata->bpnodes.find(_service.parent);
  //pIt->status_info |= _service.status_info;
  if(pIt->id != "root") updateParentStatus(*pIt);
}

void WebDashboard::updateServiceTree(void)
{
  //  string icon;

  //  for(NodeListT sIt  = m_cdata->bpnodes.begin(); sIt != m_cdata->bpnodes.end(); ++sIt) {
  //    icon = "icons/built-in/unknown.png";
  //    if( sIt->status_info[MonitorBroker::NagiosCritical] ){
  //      icon = "icons/built-in/critical.png";
  //    } else if(  sIt->status_info[MonitorBroker::NagiosWarning] ){
  //      icon = "icons/built-in/warning.png";
  //    } else if(  sIt->status_info[MonitorBroker::NagiosOk]){
  //      icon = "icons/built-in/normal.png";
  //    }
  //    sIt->navt_item->setIcon(icon);
  //  }

  //  for(NodeListT::Iterator sIt  = m_cdata->cnodes.begin(); sIt != m_cdata->cnodes.end(); ++sIt) {
  //    icon = "icons/built-in/unknown.png";
  //    if(  sIt->status_info[MonitorBroker::NagiosCritical] ){
  //      icon = "icons/built-in/critical.png";
  //    } else if(  sIt->status_info[MonitorBroker::NagiosWarning] ){
  //      icon = "icons/built-in/warning.png";
  //    } else if(  sIt->status_info[MonitorBroker::NagiosOk]){
  //      icon = "icons/built-in/normal.png";
  //    }
  //    sIt->navt_item->setIcon(icon);
  //  }
}


Wt::WContainerWidget* WebDashboard::createMenuBarWidget(void)
{
  Wt::WContainerWidget* menuBar(new Wt::WContainerWidget());
  Wt::WHBoxLayout* layout(new Wt::WHBoxLayout());
  layout->setSpacing(0);
  //  menuBar->setStyleClass("menubar");
  layout->setSpacing(0);
  menuBar->setPadding(0, Wt::All);
  layout->setContentsMargins(0, 0, 0, 0);

  Wt::WPushButton *b(new Wt::WPushButton(QObject::tr("Refresh").toStdString()));
  b->setIcon("images/built-in/refresh.png");
  //  b->setStyleClass("button");
  layout->addWidget(b, 0);

  b =  new Wt::WPushButton(QObject::tr("Zoom in").toStdString());
  b->setIcon("images/built-in/zoomin.png");
  //  b->setStyleClass("button");
  layout->addWidget(b, 0);

  b =  new Wt::WPushButton(QObject::tr("Zoom out").toStdString());
  b->setIcon("images/built-in/zoomout.png");
  //  b->setStyleClass("button");
  layout->addWidget(b, 0);

  b =  new Wt::WPushButton(QObject::tr("Save map").toStdString());
  b->setIcon("images/built-in/disket.png");
  //  b->setStyleClass("button");
  layout->addWidget(b, 0);

  b =  new Wt::WPushButton(QObject::tr("Help").toStdString());
  b->setIcon("images/built-in/help.png");
  //  b->setStyleClass("button");
  layout->addWidget(b, 0);

  b =  new Wt::WPushButton(QObject::tr("Quit").toStdString());
  b->setIcon("images/built-in/logout.png");
  //  b->setStyleClass("button");
  layout->addWidget(b, 0);

  menuBar->setLayout(layout);
  return menuBar;
}
