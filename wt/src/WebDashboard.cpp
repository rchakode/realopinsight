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
#include <Wt/WScrollArea>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "WebDashboard.hpp"
#include "Base.hpp"
#include "utilsClient.hpp"

WebDashboard::WebDashboard(const qint32& _userRole, const QString& _config)
  : DashboardBase(_userRole, _config),
    m_widget(new Wt::WContainerWidget()),
    m_tree(new TreeNodeItemListT()),
    m_map(new WebMap(m_cdata)),
    m_msgConsole(new WebMsgConsole())
{
  Wt::WContainerWidget* leftContainer = new Wt::WContainerWidget();
  Wt::WContainerWidget* rightContainer = new Wt::WContainerWidget();
  Wt::WContainerWidget* mapContainer = new Wt::WContainerWidget();
  Wt::WContainerWidget* msgContainer = new Wt::WContainerWidget();
  Wt::WVBoxLayout* mainLayout = new Wt::WVBoxLayout();
  Wt::WHBoxLayout* centralLayout = new Wt::WHBoxLayout();
  Wt::WVBoxLayout* leftLayout = new Wt::WVBoxLayout();
  Wt::WVBoxLayout* rightLayout = new Wt::WVBoxLayout();
  Wt::WVBoxLayout* mapLayout = new Wt::WVBoxLayout();
  Wt::WVBoxLayout* msgLayout = new Wt::WVBoxLayout();
  Wt::WScrollArea* mapScArea = new Wt::WScrollArea();
  Wt::WPanel* treePanel = new Wt::WPanel();
  Wt::WPanel* mapPanel = new Wt::WPanel();
  Wt::WPanel* msgPanel = new Wt::WPanel();

  m_widget->setStyleClass("container");
  leftContainer->setStyleClass("container");
  rightContainer->setStyleClass("container");
  mapContainer->setStyleClass("container");
  msgContainer->setStyleClass("container");

  mainLayout->setContentsMargins(0, 0, 0, 0);
  centralLayout->setContentsMargins(0, 0, 0, 0);
  leftLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  mapLayout->setContentsMargins(0, 0, 0, 0);
  msgLayout->setContentsMargins(0, 0, 0, 0);

  centralLayout->setSpacing(1);
  rightLayout->setSpacing(1);

  treePanel->setTitle(QObject::tr("Message Console").toStdString());
  //FIXME: treePanel->setCentralWidget(navTree);
  leftLayout->addWidget(treePanel);
  leftContainer->setLayout(leftLayout);
  leftContainer->setOverflow(Wt::WContainerWidget::OverflowAuto);
  leftContainer->resize(250, m_widget->height());

  mapPanel->setTitle(QObject::tr("TV Explorer").toStdString());
  mapScArea->setWidget(m_map);
  mapLayout->addWidget(mapScArea);
  mapContainer->setLayout(mapLayout);
  mapPanel->setCentralWidget(mapContainer);

  msgPanel->setTitle(QObject::tr("Message Console").toStdString());
  msgLayout->addWidget(m_msgConsole);
  msgContainer->setLayout(msgLayout);
  msgPanel->setCentralWidget(msgContainer);

  //FIXME: connect(m_msgConsole, SIGNAL(sizeChanged(int, int)), m_map, SLOT(msgPanelSizedChanged(int)));
  //connect(m_msgConsole, SIGNAL(sizeChanged(int, int)), m_map, SLOT(msgPanelSizedChanged(int)));

  rightContainer->setLayout(rightLayout);
  rightLayout->addWidget(mapPanel);
  rightLayout->addWidget(msgPanel);
  rightLayout->setStretchFactor(mapPanel, 3);
  rightLayout->setStretchFactor(msgPanel, 2);

  centralLayout->addWidget(leftContainer, 1);
  centralLayout->addWidget(rightContainer, 3);

  mainLayout->addWidget(createMenuBarWidget(), 0);
  mainLayout->addLayout(centralLayout, 2);
  m_widget->setLayout(mainLayout);

  //FIXME: widget alignment
  centralLayout->setResizable(0);
  centralLayout->setResizable(1);
  rightLayout->setResizable(0);
  rightLayout->setResizable(1);

  load(_config);
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

  qDebug() << _file;
  if (!_file.isEmpty()) {
    m_config = utils::getAbsolutePath(_file);
  }

  Parser parser(m_config, m_cdata);
  parser.process(true);
  parser.computeNodeCoordinates();
  //FIXME: m_tree->clear();
  //FIXM: m_tree->addTopLevelItem(m_cdata->tree_items[SvNavigatorTree::RootId]);
  //FIXME: m_map->load(parser.getDotGraphFile(), m_cdata->bpnodes, m_cdata->cnodes);
  m_map->drawMap(m_cdata->map_width, m_cdata->map_height, false);

//  m_root = m_cdata->bpnodes.find(SvNavigatorTree::RootId);
//  if (m_root == m_cdata->bpnodes.end()) {
//    utils::alert(tr("The configuration seems to be invalid, there is not a root service!"));
//    exit(1);
//  }
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

bool WebDashboard::buildNavTreeModel()
{
  //  Ngrt4nViewItemListT nav_tree_items; /* Store the set of items */

  //  /* Create a item for each individual service */
  //  for(ServiceIteratorT service  = data.bpnodes.begin(); service != data.bpnodes.end(); ++service) {
  //    service->second.navt_item = NodeTree::createItem(service->second);
  //    nav_tree_items.insert(pair<string, WStandardItem*>(service->second.id, service->second.navt_item));
  //  }

  //  for(ServiceIteratorT service  = data.cnodes.begin(); service != data.cnodes.end(); ++service) {
  //    service->second.navt_item = NodeTree::createItem(service->second);
  //    nav_tree_items.insert(pair<string, WStandardItem*>(service->second.id, service->second.navt_item));
  //  }

  //  /* Organize an hierarchy according to the mapping of services */
  //  for(ServiceIteratorT service  = data.bpnodes.begin(); service != data.bpnodes.end(); ++service) {
  //    if(service->second.id == "root") {
  //      navTreeRoot = service->second.navt_item;
  //    } else {
  //      Ngrt4nViewItemListT::iterator p_it = nav_tree_items.find(service->second.parent);
  //      p_it->second->appendRow(service->second.navt_item); /* Normally the search would have succeeded, according to the parsing step */
  //    }
  //  }

  //  for(ServiceIteratorT service  = data.cnodes.begin(); service != data.cnodes.end(); ++service) {
  //    Ngrt4nViewItemListT::iterator p_it = nav_tree_items.find(service->second.parent);
  //    p_it->second->appendRow(service->second.navt_item); /* Normally the search would have succeeded, according to the parsing step */
  //  }

  return true;
}


void WebDashboard::updateViews(void)
{
  //  updateServicesStatuses();
  //  msgConsole->update(data.cnodes);
  //  map->update(data.bpnodes, data.cnodes, mapWidth, mapHeight);
  //  navTree->update(navTreeRoot); updateServiceTree();
}

void WebDashboard::updateServicesStatuses()
{
  //  //Fixme: loadConfig
  //  //if (! Ngrt4nConfigParser::loadNagiosCollectedData("examples/status.dat", data.cnodes) ) return;
  //  return; //FIXME:

  //  for(ServiceIteratorT sIt  = data.bpnodes.begin(); sIt != data.bpnodes.end(); ++sIt) {
  //    sIt->status_info.reset();
  //  }

  //  for(NodeListT::Iterator sIt  = data.cnodes.begin(); sIt != data.cnodes.end(); ++sIt) {
  //    if(sIt->id != "root") {
  //      ServiceIteratorT pIt = data.bpnodes.find(sIt->parent);
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

  //  for(ServiceIteratorT sIt  = data.bpnodes.begin(); sIt != data.bpnodes.end(); ++sIt) {
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

  //  for(NodeListT::Iterator sIt  = data.cnodes.begin(); sIt != data.cnodes.end(); ++sIt) {
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
  menuBar->setStyleClass("menubar");
  layout->setSpacing(0);
  menuBar->setPadding(0, Wt::All);
  layout->setContentsMargins(0, 0, 0, 0);

  Wt::WPushButton *b(new Wt::WPushButton(QObject::tr("Refresh").toStdString()));
  b->setIcon("images/built-in/refresh.png");
  b->setStyleClass("button");
  layout->addWidget(b, 0);

  b =  new Wt::WPushButton(QObject::tr("Zoom in").toStdString());
  b->setIcon("images/built-in/zoomin.png");
  b->setStyleClass("button");
  layout->addWidget(b, 0);

  b =  new Wt::WPushButton(QObject::tr("Zoom out").toStdString());
  b->setIcon("images/built-in/zoomout.png");
  b->setStyleClass("button");
  layout->addWidget(b, 0);

  b =  new Wt::WPushButton(QObject::tr("Save map").toStdString());
  b->setIcon("images/built-in/disket.png");
  b->setStyleClass("button");
  layout->addWidget(b, 0);

  b =  new Wt::WPushButton(QObject::tr("Help").toStdString());
  b->setIcon("images/built-in/help.png");
  b->setStyleClass("button");
  layout->addWidget(b, 0);

  b =  new Wt::WPushButton(QObject::tr("Quit").toStdString());
  b->setIcon("images/built-in/logout.png");
  b->setStyleClass("button");
  layout->addWidget(b, 0);

  menuBar->setLayout(layout);
  return menuBar;
}
