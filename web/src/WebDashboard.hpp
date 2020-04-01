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
#include "WebEventConsole.hpp"
#include "WebMap.hpp"
#include <QHash>
#include <Wt/WGridLayout.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WPushButton.h>
#include <Wt/WMenu.h>
#include <Wt/WMenuItem.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WLocalizedStrings.h>
#include <Wt/WLabel.h>
#include <Wt/WApplication.h>

/**
  This fonction take as parameter the height of the navigation window
  Important : the size of stacked container corresponds to the size of the windows
  minus the size of the navbar (40)
  */
#define JS_AUTO_RESIZING_SCRIPT(windowHeight) \
  windowHeight \
  "var maxHeight = windowHeight - 50;" \
  "$('#stackcontentarea').height(maxHeight);" \
  "var heightGridRow1 = maxHeight*0.60 - 15;" \
  "var heightGridRow2 = maxHeight - heightGridRow1 - 15;" \
  "$('#wrapper').height(windowHeight);" \
  "$('#maincontainer').height(maxHeight);" \
  "$('#"+wApp->root()->id()+"').height(windowHeight);" \
  "$('#"+m_treeContainerId+"').height(heightGridRow1);" \
  "$('#"+m_mapContainerId+"').height(heightGridRow1);" \
  "$('#"+m_chartContainerId+"').height(heightGridRow2);" \
  "$('#"+m_eventContainerId+"').height(heightGridRow2);"

#define JS_AUTO_RESIZING_FUNCTION \
  "function(self, windowWidth, windowHeight) {" \
  JS_AUTO_RESIZING_SCRIPT("windowHeight=windowHeight;") \
  "var mapHeight = $('#"+m_mapContainerId+"').height();" \
  "var mapWidth = $('#"+m_mapContainerId+"').width();" \
  "Wt.emit("+m_mapRef->id()+", 'containerSizeChanged', mapWidth, mapHeight, windowWidth, windowHeight);" \
  "}"


class WebDashboard : public DashboardBase, public Wt::WContainerWidget
{
  Q_OBJECT

public:
  WebDashboard(DbSession* dbSession);
  virtual ~WebDashboard();
  void updateMap(void);
  void buildMap(void);
  void buildTree(void);
  void updateThumb(void);

  WebMap* mapRef(void) {
    return m_mapRef;
  }
  std::string thumbURL(void) {
    return m_mapRef->thumbURL();
  }
  std::string thumbLink(void) {
    return m_mapRef->thumbLink();
  }
  std::string thumbTitle(void) {
    return m_thumbTitle;
  }
  std::string thumbMsg(void) {
    return m_thumbMsg;
  }
  std::string thumbCss(void) {
    return ngrt4n::thumbCss(rootNode().sev);
  }
  virtual std::pair<int, QString> initialize(const QString& vfile);
  std::string tooltip(void) {
    return m_chartRef->toStdString();
  }
  void doJavascriptAutoResize(void) {
    doJavaScript(JS_AUTO_RESIZING_SCRIPT("windowHeight=$(window).height();"));
  }
  void refreshMsgConsoleOnProblemStates(void);
  std::unique_ptr<Wt::WVBoxLayout> eventItemsContainerLayout(void) {
    return std::move(m_eventItemsContainerLayout);
  }
  void handleDashboardSelected(std::string viewName) {
    Q_EMIT dashboardSelected(viewName);
  }


protected:
  void updateMap(const NodeT& _node, const QString& _tip);
  virtual void updateTree(const NodeT& node, const QString& tooltip);
  virtual void updateMsgConsole(const NodeT& node);
  virtual void updateChart(void);
  virtual void updateEventFeeds(const NodeT& node);

Q_SIGNALS:
  void dashboardSelected(std::string viewName);

private:
  WebTree* m_treeRef;
  WebMap* m_mapRef;
  WebMsgConsole* m_eventConsoleRef;
  WebPieChart* m_chartRef;
  std::string m_thumbTitle;
  std::string m_thumbMsg;
  std::unique_ptr<Wt::WVBoxLayout> m_eventItemsContainerLayout;
  QHash<QString, Wt::WWidget*> m_eventItems;
  Wt::WGridLayout* m_mainLayoutRef;

  std::string m_treeContainerId;
  std::string m_mapContainerId;
  std::string m_chartContainerId;
  std::string m_eventContainerId;

  std::unique_ptr<Wt::WWidget> createEventFeedTpl(const NodeT& node);
  void hanleRenderingAreaSizeChanged(double mapWidth, double mapHeight, double windowWidth, double windowHeight) {}
};


#endif /* WEBDASHBOOARD_HPP */
