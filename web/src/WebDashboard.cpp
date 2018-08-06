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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <fstream>
#include <QDebug>
#include <Wt/WApplication>
#include <Wt/WPanel>
#include <Wt/WPointF>
#include <Wt/WText>
#include <Wt/WLink>
#include <Wt/WImage>
#include <Wt/WTemplate>



WebDashboard::WebDashboard(void)
  : DashboardBase()
{
  m_tree.setCdata(&m_cdata);
  m_map.setCoreData(&m_cdata);
  bindFormWidgets();
  addJsEventScript();
}

WebDashboard::~WebDashboard()
{
  unbindWidgets();
}

std::pair<int, QString> WebDashboard::initialize(BaseSettings* p_settings, const QString& viewFile)
{
  auto outInitilization = DashboardBase::initialize(p_settings, viewFile);
  if (outInitilization.first != ngrt4n::RcSuccess) {
    CORE_LOG("error", outInitilization.second.toStdString());
  } else {
    m_thumbnailTitleBar.setText( rootNode().name.toStdString() );
    m_thumbnailProblemDetailsBar.setText("");
  }
  return outInitilization;
}

void WebDashboard::buildTree(void)
{
  m_tree.build();
  m_tree.expandNodeById(ngrt4n::ROOT_ID);
}


void WebDashboard::updateTree(const NodeT& _node, const QString& _tip)
{
  m_tree.updateItemDecoration(_node, _tip);
}

void WebDashboard::updateMsgConsole(const NodeT& _node)
{
  if (! m_showOnlyTroubles) {
    m_msgConsole.updateNodeMsg(_node);
    return ;
  }

  if (_node.sev != ngrt4n::Normal) {
    m_msgConsole.updateNodeMsg(_node);
  }
}

void WebDashboard::updateChart(void)
{
  CheckStatusCountT statsData;
  qint32 statCount = extractStatsData(statsData);
  m_chart.updateStatsData(statsData, statCount);
  m_chart.repaint();
}

void WebDashboard::buildMap(void)
{
  m_map.drawMap();
}


void WebDashboard::updateMap(const NodeT& _node, const QString& _tip)
{
  m_map.updateNode(_node, _tip);
}

void WebDashboard::updateThumbnailInfo(void)
{
  m_thumbnailProblemDetailsBar.setText(m_chart.problemsDetailsText());
}


void WebDashboard::updateMap(void)
{
  m_map.drawMap();
}

void WebDashboard::bindFormWidgets(void)
{
  setLayout(m_mainLayout = new Wt::WHBoxLayout());

  m_mainLayout->setContentsMargins(0, 0, 0, 0);
  m_leftSubMainLayout.setContentsMargins(0, 0, 0, 0);
  m_rightSubMainLayout.setContentsMargins(0, 0, 0, 0);

  m_mainLayout->setSpacing(2);
  m_leftSubMainLayout.setSpacing(2);
  m_rightSubMainLayout.setSpacing(2);

  m_leftSubMainLayout.addWidget(&m_tree);
  m_leftSubMainLayout.addWidget(&m_chart);

  m_rightSubMainLayout.addWidget(m_map.getWidget());
  m_rightSubMainLayout.addWidget(&m_msgConsole);
  m_mainLayout->addLayout(&m_leftSubMainLayout);
  m_mainLayout->addLayout(&m_rightSubMainLayout);

  m_leftSubMainLayout.setResizable(0);
  m_mainLayout->setResizable(0);
  m_mainLayout->setResizable(1);
  m_leftSubMainLayout.setResizable(0);
  m_leftSubMainLayout.setResizable(1);
  m_rightSubMainLayout.setResizable(0);
  m_rightSubMainLayout.setResizable(1);
}


void WebDashboard::unbindWidgets(void)
{
  m_eventFeedLayout.clear();
  m_leftSubMainLayout.removeWidget(&m_tree);
  m_leftSubMainLayout.removeWidget(&m_chart);
  m_rightSubMainLayout.removeWidget(m_map.getWidget());
  m_rightSubMainLayout.removeWidget(&m_msgConsole);
  m_mainLayout->removeItem(&m_leftSubMainLayout);
  m_mainLayout->removeItem(&m_rightSubMainLayout);
  clear();
}


void WebDashboard::addJsEventScript(void)
{
  setJavaScriptMember("wtResize", JS_AUTO_RESIZING_FUNCTION);
  triggerResizeComponents();
}


void WebDashboard::updateEventFeeds(const NodeT &node)
{
  EventFeedItemsT::Iterator feed = m_eventFeedItems.find(node.id);
  if (feed != m_eventFeedItems.end()) {
    m_eventFeedLayout.removeWidget(*feed);
    delete *feed;
    m_eventFeedItems.erase(feed);
  }
  // FIXME: need optimization to avoid removing and readding the same item
  if (node.sev != ngrt4n::Normal) {
    Wt::WWidget* widget = createEventFeedTpl(node);
    m_eventFeedLayout.insertWidget(0, widget);
    m_eventFeedItems.insert(node.id, widget);
  }
}


Wt::WWidget* WebDashboard::createEventFeedTpl(const NodeT& node)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("event-feed.tpl"));
  Wt::WAnchor* anchor = new Wt::WAnchor(Wt::WLink("#"), node.child_nodes.toStdString());

  std::string viewName = rootNode().name.toStdString();

  anchor->clicked().connect(std::bind(&WebDashboard::handleDashboardSelected, this, viewName));

  //FIXME: clear widget
  tpl->bindWidget("event-feed-title", anchor);
  tpl->bindString("severity-css-class", ngrt4n::severityCssClass(node.sev));
  tpl->bindString("event-feed-icon", ngrt4n::NodeIcons[node.icon]);
  tpl->bindString("event-feed-details", node.check.alarm_msg);
  tpl->bindString("platform", viewName);
  tpl->bindString("timestamp", ngrt4n::wTimeToNow(node.check.last_state_change));
  return tpl;
}


void WebDashboard::handleShowOnlyTroubleEvents(bool showOnlyTrouble, DbSession* dbSession)
{
  m_showOnlyTroubles = showOnlyTrouble;
  setDisabled(true);
  m_msgConsole.clearAll();
  updateAllNodesStatus(dbSession);
  setDisabled(false);
}
