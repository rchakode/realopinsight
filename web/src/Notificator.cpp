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
      "Check Time: %2  <br />"
      "State:      %3  <br />"
      "Last State: %4  <br />"
      "</p>"
      "<h2>Service Details</h2>"
      "<hr />"
      "%5";   // Print root node details\n"
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


void Notificator::sendEmailNotification(const NodeT& node, int lastState, const QosDataT& qosData, const QStringList& recipients)
{
  QString stateString = Severity(node.sev).toString();
  QString lastStateString = Severity(lastState).toString();

  QString emailSubject;
  if (lastState != ngrt4n::Normal && node.sev == ngrt4n::Normal) {
    emailSubject = QString("%1 - Recovery").arg(node.name);
  } else {
    emailSubject = QString("%1 - %2 Problem").arg(node.name, stateString);
  }


  QString emailContent = EMAIL_NOTIFICATION_CONTENT_TEMPLATE.arg(
        emailSubject,
        ngrt4n::timet2String(qosData.timestamp).toUTF8().c_str(),
        stateString,
        lastStateString,
        node.toString().replace("\n", "<br />"));

  m_mailSender->send("rodrigue_in@yahoo.fr", recipients, emailSubject,  emailContent);

  m_eventSynchonizer.exec();
}

void Notificator::handleNotification(const NodeT& node, const QosDataT& qosData)
{
  std::string viewName = node.name.toStdString();

  QStringList notificationRecipients;
  if (m_dbSession->fetchAssignedUserEmails(notificationRecipients, viewName) <= 0) {
    REPORTD_LOG("debug", QString("No notification recipients for view: %1").arg(viewName.c_str()).toStdString());
    return;
  }

  NotificationT notificationData;
  int succeeded = m_dbSession->fetchNotificationData(notificationData, viewName);
  if (node.sev != ngrt4n::Normal) { //proble state
    if (succeeded != 0 || notificationData.ack_status == DboNotification::Closed) { // send new notification
      sendEmailNotification(node, ngrt4n::Normal, qosData, notificationRecipients);
      if (succeeded != 0) {
        m_dbSession->addNotification(viewName, node.sev);
      } else  {
        m_dbSession->changeNotificationStatus("admin", viewName, DboNotification::Open);
      }
    } else {
      if (notificationData.view_status != node.sev) { //severity changed
        sendEmailNotification(node, notificationData.view_status, qosData, notificationRecipients);
        m_dbSession->changeNotificationStatus("admin", viewName, DboNotification::Open);
        m_dbSession->addNotification(viewName, node.sev);
      } else {
        if (notificationData.ack_status != DboNotification::Acknowledged) {
          REPORTD_LOG("error", QString("The service %1 is still in %2 state").arg(viewName.c_str(), Severity(node.sev).toString()).toStdString());
          // FIXME: escalate it?
        } else {
          REPORTD_LOG("error", QString("The service %1 is still in %2 state (Acknowledged)").arg(viewName.c_str(), Severity(node.sev).toString()).toStdString());
          //FIXME:
          //REPORTD_LOG("error",
          //            QString("The service %1 is still in %2 state (Acknowledged on %)")
          //            .arg(viewName.c_str(), Severity(serviceInfo.sev).toString(), ngrt4n::timet2String(lastNotificationData.last_change)).toStdString());
          //        }
        }
      }
    }
  } else {  // normal state
    if (succeeded > 0) { // if there were problems
      if (notificationData.view_status != node.sev) { // service recovered
        sendEmailNotification(node, notificationData.view_status, qosData, notificationRecipients);
      }
    }
    m_dbSession->changeNotificationStatus("admin", viewName, DboNotification::Closed);
    m_dbSession->changeNotificationStatus("admin", viewName, DboNotification::Closed);
  }
}

