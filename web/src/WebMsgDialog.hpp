/*
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 17-12-2017                                                  #
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

#ifndef WEBMSGDIALOG_HPP
#define WEBMSGDIALOG_HPP

#include "Base.hpp"
#include "dbo/src/DbSession.hpp"
#include "dbo/src/NotificationTableView.hpp"
#include <QMap>
#include <Wt/WDialog.h>

class WebMsgDialog : public Wt::WDialog
{
public:
  WebMsgDialog(DbSession* dbSession);
  ~WebMsgDialog();

  Wt::Signal<int, std::string>& operationCompleted(void) {return m_operationCompleted;}
  void show(void);
  void clearAllServicesData(void) {m_notifTableRef->clearAllServicesData(); }
  void updateServiceData(const NodeT& node) { m_notifTableRef->updateServiceData(node); }

private:
  Wt::Signal<int, std::string> m_operationCompleted;
  Wt::WText* m_infoBoxRef;
  NotificationTableView* m_notifTableRef;
};

#endif // WEBMSGDIALOG_HPP
