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


class WebDashboard : public DashboardBase, public Wt::WContainerWidget
{
  Q_OBJECT

public:
  WebDashboard(DbSession* dbSession);
  virtual ~WebDashboard();
  void updateMap(void);
  void buildMap(void);
  void buildTree(void);

  WebMap* mapRef(void) {
    return m_mapRef;
  }
  std::string thumbURL(void) {
    return m_mapRef->thumbURL();
  }
  std::string thumbTitle(void) {
    return m_thumbTitle;
  }
  std::string thumbMsg(void) {
    return m_chartRef->problemsMsg();
  }
  std::string thumbCss(void) {
    return ngrt4n::thumbCss(rootNode().sev);
  }
  virtual std::pair<int, QString> initialize(const QString& vfile);
  std::string tooltip(void) {
    return m_chartRef->toStdString();
  }
  void setEventFeedLayout(Wt::WVBoxLayout* eventFeedLayout) {
    m_eventFeedLayout = eventFeedLayout;
  }
  void handleDashboardSelected(std::string viewName) {
    Q_EMIT dashboardSelected(viewName);
  }
  void refreshMsgConsoleOnProblemStates(void);


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
  Wt::WVBoxLayout* m_eventFeedLayout;
  QHash<QString, Wt::WWidget*> m_eventItems;

  std::string m_treeContainerId;
  std::string m_mapContainerId;
  std::string m_chartContainerId;
  std::string m_eventContainerId;

  std::unique_ptr<Wt::WWidget> createEventFeedTpl(const NodeT& node);
  void handleRenderingAreaSizeChanged(double mapWidth, double mapHeight, double windowWidth, double windowHeight) {}
};


#endif /* WEBDASHBOOARD_HPP */
