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

#ifndef WEBDASHBOOARD_HPP
#define WEBDASHBOOARD_HPP

#include "DashboardBase.hpp"
#include "WebTree.hpp"
#include "WebPieChart.hpp"
#include <Wt/WGridLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WMenu>
#include <Wt/WMenuItem>
#include <Wt/WStackedWidget>
#include "WebMsgConsole.hpp"
#include "WebMap.hpp"
#include <Wt/WLocalizedStrings>
#include <Wt/WLabel>
#include <QHash>


class WebDashboard : public DashboardBase
{
public:
  WebDashboard(const QString& descriptionFile, std::shared_ptr<Wt::WVBoxLayout> eventFeedLayout=NULL);
  virtual ~WebDashboard();
  Wt::WWidget* get(void) const {return m_widget;}
  void updateMap(void);
  WebMap* map(void) const {return m_map;}
  void updateThumbnail(void);
  Wt::WImage* thumbnail(void) const {return m_map->thumbnail();}
  Wt::WLabel* thumbnailTitleBar(void) {return m_thumbnailTitleBar;}
  void setEventFeedLayout(Wt::WVBoxLayout* layout) {m_eventFeedLayout.reset(layout);}
  virtual void initialize(Preferences* preferencePtr);

protected:
  virtual void buildMap(void);
  virtual void updateMap(const NodeT& _node, const QString& _tip);
  virtual void buildTree(void);
  virtual void updateTree(const NodeT& _node, const QString& _tip);
  virtual void updateMsgConsole(const NodeT& _node);
  virtual void updateChart(void);
  virtual void updateEventFeeds(const NodeT& node);

private:
  typedef QHash<QString, Wt::WWidget*> EventFeedItemsT;
  Wt::WContainerWidget* m_widget;
  WebTree* m_tree;
  WebMap* m_map;
  WebMsgConsole* m_msgConsole;
  WebPieChart* m_chart;
  Wt::WLabel* m_thumbnailTitleBar;
  std::shared_ptr<Wt::WVBoxLayout> m_eventFeedLayout;
  EventFeedItemsT m_eventFeedItems;

  void setupUI(void);
  void addJsEventScript(void);
  void addEvents(void);
  std::string statsTooltip(void);
  Wt::WWidget* createEventFeedItem(const NodeT& node);
};


#endif /* WEBDASHBOOARD_HPP */
