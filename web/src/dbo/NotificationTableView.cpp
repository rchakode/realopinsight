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
    m_ackStatuschanged(this),
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

/**
 * @brief Add signa/slot event handling
 */
void NotificationTableView::addEvent()
{
  m_model->itemChanged().connect(this, &NotificationTableView::handleAckStatusChanged);
}


/**
 * @brief Set the table view header
 */
void NotificationTableView::setModelHeader(void)
{
  m_model->setHeaderData(0, Q_TR("Last Change"));
  m_model->setHeaderData(1, Q_TR("Service Name"));
  m_model->setHeaderData(2, Q_TR("Severity"));
  m_model->setHeaderData(3, Q_TR("Ack Status"));
  m_model->setHeaderData(4, Q_TR("Ack User"));
}

void NotificationTableView::handleAckStatusChanged(Wt::WStandardItem* item)
{
  if (item->isCheckable()) {
    //TODO
  }
}
