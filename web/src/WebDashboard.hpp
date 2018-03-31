/*
 * WebDashboard.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 23-03-2014                                                 #
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

#ifndef WEBDASHBOOARD_HPP
#define WEBDASHBOOARD_HPP

#include "DashboardBase.hpp"
#include "WebTree.hpp"
#include "WebPieChart.hpp"
#include "WebUtils.hpp"
#include <Wt/WGridLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WMenu>
#include <Wt/WMenuItem>
#include <Wt/WStackedWidget>
#include "WebEventConsole.hpp"
#include "WebMap.hpp"
#include <Wt/WLocalizedStrings>
#include <Wt/WLabel>
#include <QHash>
#include <Wt/WApplication>

#define ROOT_DIV wApp->root()->id()
#define TREEVIEW_DIV m_tree.id()
#define MAP_DIV m_map.id()
#define MAP_SCROLL_AREA_DIV m_map.getWidget()->id()
#define CHART_SCROLL_AREA_DIV m_chart.id()
#define MSG_CONSOLE_DIV m_msgConsole.id()
#define MAP_AREA_HEIGHT_RATIO "0.4"

/**
  This fonction take as parameter the height of the navigation window
  Important : the size of stacked container corresponds to the size of the windows
  minus the size of the navbar (40)
  */
#define JS_AUTO_RESIZING_SCRIPT(computeWindowHeight) \
  computeWindowHeight \
  "var contentHeight = wh - 50;" \
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


class WebDashboard : public DashboardBase, public Wt::WContainerWidget
{
  Q_OBJECT

public:
  WebDashboard(const QString& descriptionFile);
  virtual ~WebDashboard();
  void updateMap(void);
  WebMap* map(void) {return &m_map;}
  void updateThumbnailInfo(void);
  Wt::WImage* thumbnail(void) {return m_map.thumbnailImage();}
  Wt::WLabel* thumbnailTitleBar(void) {return &m_thumbnailTitleBar;}
  Wt::WLabel* thumbnailProblemDetailBar(void) {return &m_thumbnailProblemDetailsBar;}
  std::string thumbnailCssClass(void) {return ngrt4n::thumbnailCssClass(rootNode().sev);}
  virtual void initialize(BaseSettings* preferencePtr);
  std::string tooltip(void) {return m_chart.toStdString();}
  void triggerResizeComponents(void) { doJavaScript(JS_AUTO_RESIZING_SCRIPT("wh=$(window).height();"));}
  void handleShowOnlyTroubleEvents(bool showOnlyTrouble, DbSession* dbSession);
  Wt::WVBoxLayout* eventFeedLayout(void) {return &m_eventFeedLayout;}
  void handleDashboardSelected(std::string viewName) {Q_EMIT dashboardSelected(viewName);}


protected:
  virtual void buildMap(void);
  virtual void updateMap(const NodeT& _node, const QString& _tip);
  virtual void buildTree(void);
  virtual void updateTree(const NodeT& _node, const QString& _tip);
  virtual void updateMsgConsole(const NodeT& _node);
  virtual void updateChart(void);
  virtual void updateEventFeeds(const NodeT& node);

Q_SIGNALS:
  void dashboardSelected(std::string viewName);

private:
  typedef QHash<QString, Wt::WWidget*> EventFeedItemsT;
  WebTree m_tree;
  WebMap m_map;
  WebMsgConsole m_msgConsole;
  WebPieChart m_chart;
  Wt::WLabel m_thumbnailTitleBar;
  Wt::WLabel m_thumbnailProblemDetailsBar;
  Wt::WVBoxLayout m_eventFeedLayout;
  EventFeedItemsT m_eventFeedItems;

  Wt::WHBoxLayout* m_mainLayout;
  Wt::WVBoxLayout m_leftSubMainLayout;
  Wt::WVBoxLayout m_rightSubMainLayout;

  void bindFormWidgets(void);
  void unbindWidgets(void);
  void addJsEventScript(void);
  void addEvents(void);
  Wt::WWidget* createEventFeedTpl(const NodeT& node);
};


#endif /* WEBDASHBOOARD_HPP */
