/*
 * WebDashboard.cpp
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

#include "WebUtils.hpp"
#include "WebDashboard.hpp"
#include "Base.hpp"
#include "utilsCore.hpp"
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <Wt/WApplication.h>
#include <Wt/WPanel.h>
#include <Wt/WPointF.h>
#include <Wt/WText.h>
#include <Wt/WLink.h>
#include <Wt/WImage.h>
#include <Wt/WTemplate.h>



WebDashboard::WebDashboard(DbSession* dbSession)
  : DashboardBase(dbSession)
{
  m_eventItemsContainerLayout = std::make_unique<Wt::WVBoxLayout>();
  auto mainLayout = std::make_unique<Wt::WGridLayout>();
  m_mainLayoutRef = mainLayout.get();
  mainLayout->setColumnResizable(0);
  mainLayout->setColumnResizable(1);
  mainLayout->setRowResizable(0);
  mainLayout->setRowResizable(1);

  auto tree = std::make_unique<WebTree>(&m_cdata);
  m_treeRef = tree.get();
  auto treeContainer = std::make_unique<Wt::WContainerWidget>();
  m_treeContainerId = treeContainer->id();
  treeContainer->setOverflow(Wt::Overflow::Auto);
  treeContainer->addWidget(std::move(tree));
  mainLayout->addWidget(std::move(treeContainer), 0, 0);

  auto map = std::make_unique<WebMap>(&m_cdata);
  m_mapRef = map.get();
  map->containerSizeChanged().connect(this, & WebDashboard::hanleRenderingAreaSizeChanged);
  auto mapContainer = std::make_unique<Wt::WContainerWidget>();
  m_mapContainerId = mapContainer->id();
  mapContainer->setOverflow(Wt::Overflow::Auto);
  mapContainer->addWidget(std::move(map));
  mainLayout->addWidget(std::move(mapContainer), 0, 1);

  auto chart = std::make_unique<WebPieChart>();
  m_chartRef = chart.get();
  auto chartContainer = std::make_unique<Wt::WContainerWidget>();
  m_chartContainerId = chartContainer->id();
  chartContainer->setOverflow(Wt::Overflow::Auto);
  chartContainer->addWidget(std::move(chart));
  mainLayout->addWidget(std::move(chartContainer), 1, 0);

  auto eventConsole = std::make_unique<WebMsgConsole>();
  m_eventConsoleRef = eventConsole.get();
  auto eventContainer = std::make_unique<Wt::WContainerWidget>();
  m_eventContainerId = m_eventConsoleRef->id();
  eventContainer->setOverflow(Wt::Overflow::Auto);
  eventContainer->addWidget(std::move(eventConsole));
  mainLayout->addWidget(std::move(eventContainer), 1, 1);

  setJavaScriptMember("wtResize", JS_AUTO_RESIZING_FUNCTION);
  doJavascriptAutoResize();
  setLayout(std::move(mainLayout));
}

WebDashboard::~WebDashboard(){ }

std::pair<int, QString> WebDashboard::initialize(const QString& vfile)
{
  auto initResult = DashboardBase::initialize(vfile);
  if (initResult.first != ngrt4n::RcSuccess) {
    CORE_LOG("error", initResult.second.toStdString());
  } else {
    m_thumbTitle = rootNode().name.toStdString();
    m_thumbMsg = "";
  }
  return initResult;
}

void WebDashboard::buildTree(void)
{
  m_treeRef->build();
  m_treeRef->expandNodeById(ngrt4n::ROOT_ID);
}


void WebDashboard::updateTree(const NodeT& node, const QString& tooltip)
{
  m_treeRef->updateItemDecoration(node, tooltip);
}

void WebDashboard::updateMsgConsole(const NodeT& node)
{
  if (node.sev != ngrt4n::Normal) {
    m_eventConsoleRef->updateNodeMsg(node);
  } else {
    if (! m_showOnlyProblemMsgsState) {
      m_eventConsoleRef->updateNodeMsg(node);
    }
  }
}

void WebDashboard::updateChart(void)
{
  CheckStatusCountT statsData;
  qint32 statCount = extractStatsData(statsData);
  m_chartRef->updateStatsData(statsData, statCount);
  m_chartRef->repaint();
}

void WebDashboard::buildMap(void)
{
  m_mapRef->drawMap();
}


void WebDashboard::updateMap(const NodeT& _node, const QString& _tip)
{
  m_mapRef->updateNode(_node, _tip);
}

void WebDashboard::updateThumb(void)
{
  m_thumbMsg = m_chartRef->problemsMsg();
}


void WebDashboard::updateMap(void)
{
  m_mapRef->drawMap();
}


void WebDashboard::updateEventFeeds(const NodeT &node)
{
  auto oldItem = m_eventItems.find(node.id);
  if (oldItem != m_eventItems.end()) {
    auto itemPtr = m_eventItemsContainerLayout->removeWidget(*oldItem);
    itemPtr.reset(nullptr);
    m_eventItems.erase(oldItem);
  }
  if (node.sev != ngrt4n::Normal) {
    auto newEventItem = createEventFeedTpl(node);
    m_eventItems.insert(node.id, newEventItem.get());
    m_eventItemsContainerLayout->insertWidget(0, std::move(newEventItem));
  }
}


std::unique_ptr<Wt::WWidget> WebDashboard::createEventFeedTpl(const NodeT& node)
{
  std::string vname = rootNode().name.toStdString();
  auto anchor = std::make_unique<Wt::WAnchor>(Wt::WLink("#"), node.child_nodes.toStdString());
  anchor->clicked().connect(std::bind(&WebDashboard::handleDashboardSelected, this, vname));

  auto page = std::make_unique<Wt::WTemplate>(Wt::WString::tr("event-feed.tpl"));
  page->bindWidget("event-feed-title", std::move(anchor));
  page->bindString("severity-css-class", ngrt4n::severityCssClass(node.sev));
  page->bindString("event-feed-icon", ngrt4n::NodeIcons[node.icon]);
  page->bindString("event-feed-details", node.check.alarm_msg);
  page->bindString("platform", vname);
  page->bindString("timestamp", ngrt4n::wTimeToNow(node.check.last_state_change));

  return std::move(page);
}


void WebDashboard::refreshMsgConsoleOnProblemStates(void)
{
  setDisabled(true);
  m_eventConsoleRef->clearAll();
  for(const auto& node: m_cdata.cnodes) {
    updateMsgConsole(node);
  }
  setDisabled(false);
}
