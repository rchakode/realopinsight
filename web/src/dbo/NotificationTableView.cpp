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

NotificationTableView::NotificationTableView(DbSession* dbSession, Wt::WContainerWidget* parent)
  : Wt::WTableView(parent),
    m_ackStatusChanged(this),
    m_model(new Wt::WStandardItemModel(0, 5, this)),
    m_dbSession(dbSession)
{
  setSortingEnabled(true);
  setLayoutSizeAware(true);
  setColumnResizeEnabled(true);
  setSelectable(true);
  setSelectionMode(Wt::SingleSelection);
  setSelectionBehavior(Wt::SelectRows);
  setHeaderHeight(26);
  setAlternatingRowColors(true);

  setModelHeader();
  setModel(m_model);
  addEvent();
}

void NotificationTableView::addEvent()
{
  m_model->itemChanged().connect(this, &NotificationTableView::handleAckStatusChanged);
}


void NotificationTableView::setModelHeader(void)
{
  m_model->setHeaderData(0, Q_TR("Service Name"));
  m_model->setHeaderData(1, Q_TR("Severity"));
  m_model->setHeaderData(2, Q_TR("Last Change"));
  m_model->setHeaderData(3, Q_TR("Ack Status"));
  m_model->setHeaderData(4, Q_TR("Ack User"));
}


void NotificationTableView::updateEntries(void)
{
  setDisabled(true);
  NotificationListT notifications;
  m_model->clear();
  int count = m_dbSession->fetchUserRelatedNotifications(notifications, m_dbSession->loggedUser().username);
  if (count > 0) {
    for (const auto& entry : notifications)
      addNotificationEntry(entry);
  } else if (count <= 0) {
    // FIXME: handle error
  }
  setDisabled(false);
}

void NotificationTableView::addNotificationEntry(const NotificationT& notifData)
{
  int row = m_model->rowCount();
  m_model->setItem(row, 0, ngrt4n::createStandardItem(notifData.view_name, notifData.view_name));
  m_model->setItem(row, 1, ngrt4n::createStandardItem(Severity(notifData.view_status).toString().toStdString(), notifData.view_name));
  m_model->setItem(row, 2, ngrt4n::createStandardItem(ngrt4n::timet2String(notifData.last_change).toUTF8(), notifData.view_name) );
  m_model->setItem(row, 3, ngrt4n::createCheckableStandardIItem(notifData.view_name, notifData.ack_status == DboNotification::Acknowledged));
  m_model->setItem(row, 3, ngrt4n::createStandardItem(notifData.ack_username, notifData.view_name));
}

void NotificationTableView::handleAckStatusChanged(Wt::WStandardItem* item)
{
  if (item->isCheckable()) {
    std::string viewName = ngrt4n::getItemData(item);
    if (item->checkState() == Wt::Checked) {
      m_dbSession->changeNotificationStatus(m_dbSession->loggedUser().username,
                                            viewName,
                                            DboNotification::Acknowledged);
      // FIXME: handle error?
    } else {
      m_dbSession->changeNotificationStatus(m_dbSession->loggedUser().username,
                                            viewName,
                                            DboNotification::Open);
      // FIXME: handle error?
    }
  }
}
