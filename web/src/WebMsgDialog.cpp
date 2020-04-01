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

#include "WebMsgDialog.hpp"
#include "WebUtils.hpp"
#include <Wt/WPushButton.h>

WebMsgDialog::WebMsgDialog(DbSession* dbSession)
  : Wt::WDialog(Q_TR("Manage Notifications"))
{
  setStyleClass("Wt-dialog");
  titleBar()->setStyleClass("titlebar");

  auto notifTable = std::make_unique<NotificationTableView>(dbSession);
  m_notifTableRef = notifTable.get();
  contents()->addWidget(std::move(notifTable));

  auto infoBox = std::make_unique<Wt::WText>();
  m_infoBoxRef = infoBox.get();
  footer()->addWidget(std::move(infoBox));

  auto closeBtn = std::make_unique<Wt::WPushButton>(Q_TR("Close"));
  closeBtn->clicked().connect(this, &Wt::WDialog::accept);
  this->footer()->addWidget(std::move(closeBtn));
}


WebMsgDialog::~WebMsgDialog() {}


void WebMsgDialog::show(void)
{
  if (m_notifTableRef->update() != 0) {
    m_infoBoxRef->setHidden(false);
    m_infoBoxRef->setText(m_notifTableRef->lastError());
    m_infoBoxRef->setStyleClass("text-danger");
  } else {
    m_infoBoxRef->setHidden(true);
    m_infoBoxRef->setStyleClass("text-muted");
  }
  Wt::WDialog::show();
}
