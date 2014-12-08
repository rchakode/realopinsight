/*
# WebNotificationManager.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 08-12-2014                                                  #
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

#include "WebNotificationManager.hpp"
#include "WebUtils.hpp"
#include <Wt/WPushButton>

WebNotificationManager::WebNotificationManager(DbSession* dbSession, Wt::WContainerWidget* parent)
  : Wt::WDialog(Q_TR("Manage Notifications"), parent)
{
  m_notificationTableView = new NotificationTableView(dbSession, this->contents());
  Wt::WPushButton* closeButton = new Wt::WPushButton(Q_TR("Close"), this->footer());
  closeButton->clicked().connect(this, &Wt::WDialog::accept);

  setStyleClass("Wt-dialog");
  titleBar()->setStyleClass("titlebar");
}


WebNotificationManager::~WebNotificationManager()
{
  delete m_notificationTableView;
}
