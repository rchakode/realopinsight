/*
# NotificationManager.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 07-12-2014                                                  #
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

#ifndef NOTIFICATIONMANAGER_HPP
#define NOTIFICATIONMANAGER_HPP

#include "Base.hpp"
#include "dbo/src/DbSession.hpp"
#include "dbo/src/DbObjects.hpp"
#include <Wt/WTableView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>

class NotificationTableView : public Wt::WTableView
{
public:
  NotificationTableView(DbSession* dbSession, Wt::WContainerWidget* parent = nullptr);
  int update(void);
  std::string lastError(void) const {return m_lastError;}
  void clearAllServicesData(void) { m_services.clear(); }
  void updateServiceData(const NodeT& node) { m_services[node.name] = node; }

private:
  /** other members **/
  std::string m_lastError;
  Wt::WStandardItemModel* m_model;
  DbSession* m_dbSession;
  NodeListT m_services;

  void addEvent(void);
  void setModelHeader(void);
  void handleAckStatusChanged(Wt::WStandardItem* item);
  void addServiceEntry(const NodeT& service, bool hasNotification, const NotificationT& notification);
};

#endif // NOTIFICATIONMANAGER_HPP
