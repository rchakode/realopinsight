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

#include <Wt/WApplication>
#include <Wt/WGridLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WMenu>
#include <Wt/WMenuItem>
#include <Wt/WStackedWidget>
#include "WebMsgConsole.hpp"
#include "WebServiceMap.hpp"
#include <Wt/WLocalizedStrings>
#include "DashboardBase.hpp"

class WebDashboard : public DashboardBase
{
public:
  WebDashboard(const qint32& _userRole, const QString& _config);
  virtual ~WebDashboard();
  void loadConfig();

private:
  TreeNodeItemListT* m_tree;
  WebServiceMap* m_map;
  WebMsgConsole* m_msgConsole;

  Wt::WLayout* createMenuBar(void);
  Wt::WContainerWidget* createMenuBarWidget(void);
  bool buildNavTreeModel();
  bool computeMapCoordinates(CoreDataT& _cdata);
  void updateViews();
  void updateServicesStatuses();
  void updateParentStatus(const NodeT& _service);
  void updateServiceTree(void);
};

#endif /* WEBDASHBOOARD_HPP */
