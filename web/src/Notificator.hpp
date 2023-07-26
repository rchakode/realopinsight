/*
 * Notificator.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 01-12-2014                                                 #
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

#ifndef NOTIFICATOR_H
#define NOTIFICATOR_H
#include "Base.hpp"
#include <QObject>
#include <QString>
#include "dbo/src/DbSession.hpp"
#include "WebNotificationSettings.hpp"
#include "utils/smtpclient/MailSender.hpp"

class Notificator : public QObject
{
  Q_OBJECT

public:
  Notificator(void);
  void sendEmailNotification(const NodeT& node, int lastState, const PlatformStatusT& pfStatus, const QStringList& recipients);
  void handleNotification(const NodeT& node, const PlatformStatusT& pfStatus);


private:
  std::unique_ptr<MailSender> m_mailSender;
  WebBaseSettings m_preferences;
  QEventLoop m_eventSynchronizer;
};
#endif // NOTIFICATOR_H
