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



namespace {
  const IconMapT ICONS = ngrt4n::nodeIcons();
}

WebDashboard::WebDashboard(const QString& descriptionFile, Wt::WVBoxLayout* eventFeedLayout)
  : DashboardBase(descriptionFile),
    m_widget(new Wt::WContainerWidget()),
    m_tree(new WebTree(m_cdata)),
    m_map(new WebMap(m_cdata)),
    m_msgConsole(new WebMsgConsole()),
    m_chart(new WebPieChart()),
    m_eventFeedLayout(eventFeedLayout)
{
  setupUI();
  addJsEventScript();
}

WebDashboard::~WebDashboard()
{
  delete m_widget; // Other widget are deleted through the hierarchy
}

void WebDashboard::initialize(Preferences* preferencePtr)
{
  DashboardBase::initialize(preferencePtr);
  if (! DashboardBase::lastErrorState()) {
    m_thumbnailTitleBar = new Wt::WLabel(rootNode().name.toStdString(), m_widget);
    m_thumbnailProblemDetailsBar = new Wt::WLabel("", m_widget);
  } else {
    LOG("error", m_lastErrorMsg.toStdString());
    Q_EMIT errorOccurred(m_lastErrorMsg);
  }
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
  if (! m_showOnlyTroubles
      || (m_showOnlyTroubles && _node.sev != ngrt4n::Normal))
  {
    m_msgConsole->updateNodeMsg(_node);
  }
}

void WebDashboard::updateChart(void)
{
  qint32 statCount;
  CheckStatusCountT statsData;
  if (m_cdata) {
    extractStatsData(statsData, statCount);
    m_chart->updateStatsData(statsData, statCount);
    m_chart->repaint();
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

void WebDashboard::updateThumbnailInfo(void)
{
  m_thumbnailProblemDetailsBar->setText(m_chart->problemsDetailsText());
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

void WebDashboard::addJsEventScript(void)
{
  m_widget->setJavaScriptMember("wtResize", JS_AUTO_RESIZING_FUNCTION);
  triggerResizeComponents();
}

void WebDashboard::updateEventFeeds(const NodeT &node)
{
  if (m_eventFeedLayout) {
    EventFeedItemsT::Iterator feed = m_eventFeedItems.find(node.id);
    if (feed != m_eventFeedItems.end()) {
      m_eventFeedLayout->removeWidget(*feed);
      delete *feed;
      m_eventFeedItems.erase(feed);
    }
    // FIXME: need optimization to avoid removing and readding the same item
    if (node.sev != ngrt4n::Normal) {
      Wt::WWidget* widget = createEventFeedItem(node);
      m_eventFeedLayout->insertWidget(0, widget);
      m_eventFeedItems.insert(node.id, widget);
    }
  }
}


Wt::WWidget* WebDashboard::createEventFeedItem(const NodeT& node)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("event-feed.tpl"));

  Wt::WAnchor* anchor = new Wt::WAnchor(Wt::WLink("#"), tr("%1 event on %2").arg(Severity(node.sev).toString(),
                                                                                 node.child_nodes).toStdString());
  anchor->clicked().connect(std::bind([&](){Q_EMIT dashboardSelected(m_widget);}));

  tpl->bindWidget("event-feed-title", anchor);
  tpl->bindString("severity-css-class", ngrt4n::severityCssClass(node.sev));
  tpl->bindString("event-feed-icon", ngrt4n::getPathFromQtResource(ICONS[node.icon]));
  tpl->bindString("event-feed-details", node.check.alarm_msg);
  tpl->bindString("platform", rootNode().name.toStdString());
  tpl->bindString("timestamp", ngrt4n::wTimeToNow(node.check.last_state_change));
  return tpl;
}


void WebDashboard::handleShowOnlyTroubleEvents(bool showOnlyTrouble)
{
  m_showOnlyTroubles = showOnlyTrouble;
  m_widget->setDisabled(true);
  m_msgConsole->clearAll();
  runMonitor();
  m_widget->setDisabled(false);
}
