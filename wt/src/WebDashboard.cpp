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

#define ROOT_DIV wApp->root()->id()
#define TREEVIEW_DIV m_tree->id()
#define MAP_DIV m_map->id()
#define MAP_SCROLL_AREA_DIV m_map->get()->id()
#define CHART_SCROLL_AREA_DIV m_chart->get()->id()
#define MSG_CONSOLE_DIV m_msgConsole->id()
#define MAP_AREA_HEIGHT_RATIO "0.4"

/**
  This fonction take as parameter the height of the navigation window
  Important : the size of stacked container corresponds to the size of the windows
  minus the size of the navbar (40)
  */
#define JS_AUTO_RESIZING_SCRIPT(computeWindowHeight) \
  computeWindowHeight \
  "var contentHeight = wh - 40;" \
  "$('#stackcontentarea').height(contentHeight);" \
  "var treeHeight=contentHeight*0.6 - 25;" \
  "var chartAreaHeight=contentHeight - treeHeight - 25;" \
  "var mapAreaHeight=contentHeight*"+std::string(MAP_AREA_HEIGHT_RATIO)+" - 25;" \
  "var msgConsoleHeight=contentHeight - mapAreaHeight - 25;" \
  "$('#wrapper').height(wh);" \
  "$('#maincontainer').height(wh);" \
  "$('#"+ROOT_DIV+"').height(wh);" \
  "$('#"+TREEVIEW_DIV+"').height(treeHeight);" \
  "$('#"+MAP_SCROLL_AREA_DIV+"').height(mapAreaHeight);" \
  "$('#"+CHART_SCROLL_AREA_DIV+"').height(chartAreaHeight);" \
  "$('#"+MSG_CONSOLE_DIV+"').height(msgConsoleHeight);"

#define JS_AUTO_RESIZING_FUNCTION \
  "function(self, width, height) {" \
  JS_AUTO_RESIZING_SCRIPT("wh=height;") \
  "var mapHeight = height*0.45 - 25;" \
  "var mapWidth = $('#"+MAP_SCROLL_AREA_DIV+"').width();" \
  "Wt.emit("+MAP_DIV+", 'containerSizeChanged', mapWidth, mapHeight);" \
  "}"

namespace {
  const IconMapT ICONS = ngrt4n::nodeIcons();
}

WebDashboard::WebDashboard(const QString& descriptionFile, Wt::WVBoxLayout* eventFeedLayout)
  : DashboardBase(descriptionFile),
    m_widget(new Wt::WContainerWidget()),
    m_tree(new WebTree(DashboardBase::cdata())),
    m_map(new WebMap(DashboardBase::cdata())),
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
  } else {
    LOG("error", DashboardBase::lastErrorMsg().toStdString());
    Q_EMIT errorOccurred(DashboardBase::lastErrorMsg());
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
  if (! DashboardBase::showOnlyTroubles()
      || (DashboardBase::showOnlyTroubles() && _node.severity != ngrt4n::Normal))
  {
    m_msgConsole->updateNodeMsg(_node);
  }
}

void WebDashboard::updateChart(void)
{
  for (auto it = std::begin(DashboardBase::cdata()->check_status_count); it != std::end(DashboardBase::cdata()->check_status_count); ++it) {
    m_chart->setSeverityData(it.key(), it.value());
  }
  m_chart->setToolTip(statsTooltip());
}

void WebDashboard::buildMap(void)
{
  //FIXME:  m_map->drawMap(); avoid do this twice at statup
  m_map->drawMap();
}


void WebDashboard::updateMap(const NodeT& _node, const QString& _tip)
{
  m_map->updateNode(_node, _tip);
}

void WebDashboard::updateThumbnail(void)
{
  m_thumbnailTitleBar->setStyleClass(ngrt4n::severityCssClass(rootNode().severity));
}


void WebDashboard::updateMap(void)
{
  m_map->drawMap();
  m_map->setThumbnailTooltip(statsTooltip());
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
  m_widget->doJavaScript(JS_AUTO_RESIZING_SCRIPT("wh=$(window).height();"));
}


std::string WebDashboard::statsTooltip(void)
{
  qint32 totalCount = DashboardBase::cdata()->cnodes.size();
  qint32 criticalCount = DashboardBase::cdata()->check_status_count[ngrt4n::Critical];
  qint32 majorCount = DashboardBase::cdata()->check_status_count[ngrt4n::Major];
  qint32 minorCount = DashboardBase::cdata()->check_status_count[ngrt4n::Minor];
  qint32 normalCount =  DashboardBase::cdata()->check_status_count[ngrt4n::Normal];
  qint32 unknownCount = totalCount - (criticalCount + majorCount + minorCount + normalCount);

  float criticalRatio = (100.0 * criticalCount) / totalCount;
  float majorRatio = (100.0 * majorCount) / totalCount;
  float minorRatio = (100.0 * minorCount) / totalCount;
  float unknownRatio = (100.0 * unknownCount) / totalCount;
  float normalRatio = (100.0 * normalCount) / totalCount;

  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("statistic-tooltip.tpl"));

  tpl->bindString("platform", DashboardBase::rootNode().name.toStdString());
  tpl->bindInt("total-count", totalCount);

  tpl->bindInt("unknown-ratio", unknownRatio);
  tpl->bindInt("unknown-count", unknownCount);

  tpl->bindInt("critical-ratio", criticalRatio);
  tpl->bindInt("critical-count", criticalCount);

  tpl->bindInt("major-ratio", majorRatio);
  tpl->bindInt("major-count", majorCount);

  tpl->bindInt("minor-ratio", minorRatio);
  tpl->bindInt("minor-count", minorCount);

  tpl->bindInt("normal-ratio", normalRatio);
  tpl->bindInt("normal-count", normalCount);

  std::ostringstream oss;
  tpl->renderTemplate(oss);
  delete tpl;
  return oss.str();
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
    if (node.severity != ngrt4n::Normal) {
      Wt::WWidget* widget = createEventFeedItem(node);
      m_eventFeedLayout->insertWidget(0, widget);
      m_eventFeedItems.insert(node.id, widget);
    }
  }
}


Wt::WWidget* WebDashboard::createEventFeedItem(const NodeT& node)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("event-feed.tpl"));
  tpl->bindString("event-feed-id", node.id.toStdString());
  tpl->bindString("severity-css-class", ngrt4n::severityCssClass(node.severity));
  tpl->bindString("event-feed-icon", ngrt4n::getPathFromQtResource(ICONS[node.icon]));
  tpl->bindWidget("event-feed-title", new Wt::WAnchor(Wt::WLink("#"),
                                                      tr("%1 event on %2")
                                                      .arg(ngrt4n::severityText(node.severity), node.name).toStdString()));
  tpl->bindString("event-feed-details", node.check.alarm_msg);
  tpl->bindString("platform", rootNode().name.toStdString());
  tpl->bindString("timestamp", ngrt4n::wTimeToNow(node.check.last_state_change));
  return tpl;
}
