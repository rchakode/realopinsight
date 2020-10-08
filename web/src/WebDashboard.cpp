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

const double MAP_DEFAULT_HEIGHT = 600.0;


WebDashboard::WebDashboard(DbSession* dbSession)
  : DashboardBase(dbSession)
{
  m_eventItemsContainerLayout = std::make_unique<Wt::WVBoxLayout>();
  auto dashboardTpl = std::make_unique<Wt::WTemplate>(Wt::WString::tr("dashboard-item.tpl"));
  m_treeRef = dashboardTpl->bindNew<WebTree>("dashboard-tree", &m_cdata);
  m_mapRef = dashboardTpl->bindNew<WebMap>("dashboard-map", &m_cdata);
  m_chartRef = dashboardTpl->bindNew<WebPieChart>("dashboard-piechart");
  m_eventConsoleRef = dashboardTpl->bindNew<WebMsgConsole>("dashboard-msg-console");
  addWidget(std::move(dashboardTpl));
}

WebDashboard::~WebDashboard(){ }

std::pair<int, QString> WebDashboard::initialize(const QString& vfile)
{
  auto initResult = DashboardBase::initialize(vfile);
  if (initResult.first != ngrt4n::RcSuccess) {
    CORE_LOG("error", initResult.second.toStdString());
  } else {
    m_thumbTitle = rootNode().name.toStdString();
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
  auto scaleFactor = std::min(1.0, MAP_DEFAULT_HEIGHT / m_cdata.map_height);
  m_mapRef->setScaleFactor(scaleFactor, scaleFactor);
  m_mapRef->drawMap();
}


void WebDashboard::updateMap(const NodeT& _node, const QString& _tip)
{
  m_mapRef->updateNode(_node, _tip);
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
