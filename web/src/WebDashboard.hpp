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
#define MAP_SCROLL_AREA_DIV m_map.renderingScrollArea()->id()
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
  "var contentHeight = winH - 50;" \
  "$('#stackcontentarea').height(contentHeight);" \
  "var treeHeight=contentHeight*0.6 - 25;" \
  "var chartAreaHeight=contentHeight - treeHeight - 25;" \
  "$('#wrapper').height(winH);" \
  "$('#maincontainer').height(winH);" \
  "$('#"+ROOT_DIV+"').height(winH);" \
  "$('#"+TREEVIEW_DIV+"').height(treeHeight);" \
  "$('#"+CHART_SCROLL_AREA_DIV+"').height(chartAreaHeight);"

#define JS_AUTO_RESIZING_FUNCTION \
  "function(self, winW, winH) {" \
  JS_AUTO_RESIZING_SCRIPT("winH=winH;") \
  "var mapH = winH*0.45 - 25;" \
  "var mapW = $('#"+MAP_SCROLL_AREA_DIV+"').width();" \
  "Wt.emit("+MAP_DIV+", 'containerSizeChanged', mapW, mapH, winW, winH);" \
  "}"


class WebDashboard : public DashboardBase, public Wt::WContainerWidget
{
  Q_OBJECT

public:
  WebDashboard(DbSession* dbSession);
  virtual ~WebDashboard();
  void updateMap(void);
  WebMap* map(void) {return &m_map;}
  void updateThumbnailInfo(void);
  Wt::WImage* thumbnail(void) {return m_map.thumbnailImage();}
  Wt::WLabel* thumbnailTitleBar(void) {return &m_thumbnailTitleBar;}
  Wt::WLabel* thumbnailProblemDetailBar(void) {return &m_thumbnailProblemDetailsBar;}
  std::string thumbnailCssClass(void) {return ngrt4n::thumbnailCssClass(rootNode().sev);}
  virtual std::pair<int, QString> initialize(BaseSettings* p_settings, const QString& descriptionFile);
  std::string tooltip(void) {return m_chart.toStdString();}
  void doJavascriptAutoResize(void) { doJavaScript(JS_AUTO_RESIZING_SCRIPT("winH=$(window).height();"));}
  void refreshMsgConsoleOnProblemStates(void);
  Wt::WVBoxLayout* eventFeedLayout(void) {return &m_eventFeedLayout;}
  void handleDashboardSelected(std::string viewName) {Q_EMIT dashboardSelected(viewName);}


protected:
  virtual void buildMap(void);
  virtual void updateMap(const NodeT& _node, const QString& _tip);
  virtual void buildTree(void);
  virtual void updateTree(const NodeT& _node, const QString& _tip);
  virtual void updateMsgConsole(const NodeT& node);
  virtual void updateChart(void);
  virtual void updateEventFeeds(const NodeT& node);

Q_SIGNALS:
  void dashboardSelected(std::string viewName);

private:
  typedef QHash<QString, Wt::WWidget*> EventFeedItemsT;
  DbSession* m_dbSession;
  WebTree m_tree;
  WebMap m_map;
  WebMsgConsole m_msgConsole;
  WebPieChart m_chart;
  Wt::WLabel m_thumbnailTitleBar;
  Wt::WLabel m_thumbnailProblemDetailsBar;
  Wt::WVBoxLayout m_eventFeedLayout;
  EventFeedItemsT m_eventFeedItems;

  Wt::WHBoxLayout* m_mainLayout;
  Wt::WVBoxLayout m_leftVBoxLayout;
  Wt::WVBoxLayout m_rightVBoxLayout;

  void setCData(CoreDataT* cdata) {m_tree.setCdata(cdata);  m_map.setCoreData(cdata);}
  void bindFormWidgets(void);
  void unbindWidgets(void);
  void addJsEventScript(void);
  void addEvents(void);
  Wt::WWidget* createEventFeedTpl(const NodeT& node);
  void hanleRenderingAreaSizeChanged(double mapW, double mapH, double winW, double winH) {}
};


#endif /* WEBDASHBOOARD_HPP */
