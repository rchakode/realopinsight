/*
# NotificationManager.cpp
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

#include "NotificationTableView.hpp"
#include "WebUtils.hpp"

namespace {
  const int COLUMN_COUNT = 6;
}
NotificationTableView::NotificationTableView(DbSession* dbSession, Wt::WContainerWidget* parent)
  : Wt::WTableView(parent),
    m_dbSession(dbSession)
{
  setSortingEnabled(true);
  setLayoutSizeAware(true);
  setColumnResizeEnabled(true);
  setSelectable(true);
  setSelectionMode(Wt::SingleSelection);
  setSelectionBehavior(Wt::SelectRows);
  setHeaderHeight(26);

  m_model = new Wt::WStandardItemModel(0, COLUMN_COUNT);
  setModel(m_model);
  setModelHeader();
  addEvent();
}

void NotificationTableView::addEvent()
{
  m_model->itemChanged().connect(this, &NotificationTableView::handleAckStatusChanged);
}


void NotificationTableView::setModelHeader(void)
{
  m_model->insertColumns(0, COLUMN_COUNT);

  m_model->setHeaderData(0, Q_TR("Service Name"));
  m_model->setHeaderData(1, Q_TR("Severity"));
  m_model->setHeaderData(2, Q_TR("Notification"));
  m_model->setHeaderData(3, Q_TR("Last Notification"));
  m_model->setHeaderData(4, Q_TR("Acknowledged"));
  m_model->setHeaderData(5, Q_TR("Last Updated by"));
}


int NotificationTableView::update(void)
{
  int retCode = -1;
  setDisabled(true);
  NotificationMapT notifications;
  int result = m_dbSession->listViewRelatedNotifications(notifications, m_dbSession->loggedUser().username);
  if (result < 0) {
    m_lastError = m_dbSession->lastError();
  } else {
    m_model->clear();
    setModelHeader();
    for (const auto& service : m_services) {
      NotificationMapT::Iterator notificationIter = notifications.find(service.name.toStdString());
      bool found = (notificationIter != notifications.end());
      if (notificationIter != notifications.end()) {
        addServiceEntry(service, found, *notificationIter);
      } else {
        addServiceEntry(service, found, NotificationT());
      }
    }
    retCode = 0;
  }
  setDisabled(false);
  return retCode;
}



void NotificationTableView::addServiceEntry(const NodeT& service,
                                            bool hasNotification,
                                            const NotificationT& notification)
{
  int row = m_model->rowCount();
  std::string serviceName = service.name.toStdString();
  m_model->setItem(row, 0, ngrt4n::createStandardItem(serviceName, serviceName));
  m_model->setItem(row, 1, ngrt4n::createSeverityStandardItem(service));
  if (hasNotification) {
    m_model->setItem(row, 2, ngrt4n::createStandardItem(Q_TR("Enabled"), serviceName) );
    m_model->setItem(row, 3, ngrt4n::createStandardItem(ngrt4n::timet2String(notification.last_change).toUTF8(), serviceName) );
    m_model->setItem(row, 4, ngrt4n::createCheckableStandardItem(serviceName, notification.ack_status == DboNotification::Acknowledged));
    m_model->setItem(row, 5, ngrt4n::createStandardItem(notification.ack_username, serviceName));
  } else {
    m_model->setItem(row, 2, ngrt4n::createStandardItem(Q_TR("Disabled"), serviceName) );
    m_model->setItem(row, 3, ngrt4n::createStandardItem("", serviceName));
    m_model->setItem(row, 4, ngrt4n::createStandardItem("", serviceName));
    m_model->setItem(row, 5, ngrt4n::createStandardItem("", serviceName));
  }

  // Deal with row alternate
  if (row & 1) {
    for (int i:  {0, 2, 3, 4, 5})
      m_model->item(row, i)->setStyleClass(ngrt4n::severityCssClass(-1));
  }
}


void NotificationTableView::handleAckStatusChanged(Wt::WStandardItem* item)
{
  if (item->isCheckable()) {
    std::string viewName = ngrt4n::getItemData(item);
    if (item->checkState() == Wt::Checked) {
      m_dbSession->updateNotificationStatus(m_dbSession->loggedUser().username, viewName, DboNotification::Acknowledged);
      // FIXME: handle error?
    } else {
      m_dbSession->updateNotificationStatus(m_dbSession->loggedUser().username, viewName,  DboNotification::Open);
      // FIXME: handle error?
    }
  }
}
