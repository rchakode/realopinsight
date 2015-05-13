/*
 * Notificator.cpp
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

#include "WebUtils.hpp"
#include "Notificator.hpp"


namespace {
  const QString EMAIL_NOTIFICATION_CONTENT_TEMPLATE =
      "<h2>%1</h2>" // %1 = {PROBLEM, RECOVERY}
      "<hr />"
      "<p>"
      "Last Change: %2  <br />"
      "Current Status:<span style=\"background-color: %3;\"> %4 </span><br />"
      "Last Status: <span style=\"background-color: %5;\"> %6 </span>  <br />"
      "</p>"
      "<h2>Service Details</h2>"
      "<hr />"
      "%7";   // Print root node details\n"
}


Notificator::Notificator(DbSession* dbSession)
  : m_dbSession(dbSession)
{
  m_mailSender.reset(new MailSender(QString::fromStdString(m_preferences.getSmtpServerAddr()),
                                    m_preferences.getSmtpServerPort(),
                                    QString::fromStdString(m_preferences.getSmtpUsername()),
                                    QString::fromStdString(m_preferences.getSmtpPassword()),
                                    m_preferences.getSmtpUseSsl()));
}


void Notificator::sendEmailNotification(const NodeT& node, int lastStatus, const QosDataT& qosData, const QStringList& recipients)
{
  if (m_preferences.getNotificationType() != WebPreferencesBase::EmailNotification) {
    // do nothing and exit
    return;
  }

  QString statusString = Severity(node.sev).toString().toUpper();
  QString statusHtmlColor = QString::fromStdString(ngrt4n::severityHtmlColor(node.sev));
  QString lastStateString = Severity(lastStatus).toString().toUpper();
  QString lastStatusHtmlColor = QString::fromStdString(ngrt4n::severityHtmlColor(lastStatus));

  QString emailSubject;
  if (lastStatus != ngrt4n::Normal && node.sev == ngrt4n::Normal) {
    emailSubject = QString("%1 - Recovered").arg(node.name);
  } else {
    emailSubject = QString("%1 - %2 Problem").arg(node.name, statusString);
  }

  REPORTD_LOG("info", emailSubject);

  QString emailContent = EMAIL_NOTIFICATION_CONTENT_TEMPLATE.arg(
        emailSubject,
        ngrt4n::timet2String(qosData.timestamp).toUTF8().c_str(),
        statusHtmlColor,
        statusString,
        lastStatusHtmlColor,
        lastStateString,
        node.toString().replace("\n", "<br />"));


  int retCode = m_mailSender->send(m_preferences.getSmtpUsername().c_str(),
                                   recipients,
                                   emailSubject,
                                   emailContent);

  std::string logLevel = (retCode == 0) ? "info" : "error";
  REPORTD_LOG(logLevel, QObject::tr("[Notificator] %1").arg(m_mailSender->lastError()));
}

void Notificator::handleNotification(const NodeT& node, const QosDataT& qosData)
{
  std::string viewName = node.name.toStdString();
  QStringList recipients;
  if (m_dbSession->listAssignedUsersEmails(recipients, viewName) <= 0) {
    REPORTD_LOG("info", QString("No notification recipients for view %1").arg(viewName.c_str()));
    return;
  }

  NotificationT lastNotifData;
  m_dbSession->getLastNotificationInfo(lastNotifData, viewName);
  switch (node.sev) {
    case  ngrt4n::Normal:
      if (lastNotifData.view_status != DboNotification::Unset && lastNotifData.view_status != DboNotification::Closed) {
        // check if service just recovered, if yes send recovery notification
        if (lastNotifData.view_status != node.sev && lastNotifData.ack_status != DboNotification::Closed) {
          sendEmailNotification(node, lastNotifData.view_status, qosData, recipients);
        }
      }
      m_dbSession->updateNotificationAckStatusForUser("admin", viewName, DboNotification::Closed);
    break;
  default:
    if (lastNotifData.view_status != node.sev) {
      sendEmailNotification(node, lastNotifData.view_status, qosData, recipients);
      m_dbSession->updateNotificationAckStatusForUser("admin", viewName, DboNotification::Closed);
      m_dbSession->addNotification(viewName, node.sev);
    } else {
      REPORTD_LOG("error", QString("The service %1 is still in %2 state").arg(viewName.c_str(), Severity(node.sev).toString()));
    }
   break;
  }
}

