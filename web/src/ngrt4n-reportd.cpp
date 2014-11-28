/*
 * ngrt4n-reportd.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
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

#include "dbo/DbSession.hpp"
#include "WebPreferences.hpp"
#include "QosCollector.hpp"
#include "WebUtils.hpp"
#include "Applications.hpp"
#include "utils/smtpclient/MailSender.hpp"
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <QString>
#include <getopt.h>
#include <unistd.h>

namespace {
  const QString EMAIL_NOTIFICATION_SUBJECT_TEMPLATE = "%1: The State of Service %2 is %3";
  const QString EMAIL_NOTIFICATION_CONTENT_TEMPLATE = "%1: The State of Service %2 is %3" // %1 = {PROBLEM, RECOVERY}
      "-----------------------------"
      "Check Time: %4"
      "State: %5"
      "Last State: %6"
      "\n"
      "Service Details:"
      "--------------------"
      "%7" // Print root node details
      "\n"
      "Additional Information"
      "-----------------------------"
      "%8" // Print stats related to IT services (QoS entry)
      "\n";
}

void sendEmailNotification(const NodeT& serviceInfo,
                           int lastState,
                           const QosDataT& qosData,
                           const QStringList& recipients,
                           const WebPreferencesBase& preferences)
{

  QString typeNotification = (lastState != ngrt4n::Normal && serviceInfo.sev == ngrt4n::Normal) ? "RECOVERY": "PROBLEM";
  QString stateString = Severity(serviceInfo.sev).toString();
  QString lastStateString = Severity(lastState).toString();

  QString emailSubject = EMAIL_NOTIFICATION_SUBJECT_TEMPLATE.arg(
        typeNotification,
        serviceInfo.name,
        stateString);

  QString emailContent = EMAIL_NOTIFICATION_CONTENT_TEMPLATE.arg(
        typeNotification,
        serviceInfo.name,
        stateString,
        ngrt4n::timet2String(qosData.timestamp).toUTF8().c_str(),
        stateString,
        lastStateString,
        serviceInfo.toString(),
        qosData.toString().c_str() //FIXME: create human-readable string
        );

  MailSender mailSender(QString::fromStdString( preferences.getSmtpServerAddr() ),
                        preferences.getSmtpServerPort(),
                        QString::fromStdString( preferences.getSmtpUsername() ),
                        QString::fromStdString( preferences.getSmtpPassword() ),
                        preferences.getSmtpUseSsl());

  mailSender.send("sender",
                  recipients,
                  emailSubject,
                  emailContent);
}

void handleNotification(const NodeT& serviceInfo,
                        const QosDataT& qosData,
                        DbSession* dbSession,
                        const WebPreferencesBase& preferences)
{

  std::string viewName = serviceInfo.name.toStdString();

  QStringList notificationRecipients;
  if (dbSession->fetchAssignedUserEmails(notificationRecipients, viewName) <= 0)
    return;

  NotificationListT activeNotifications;
  int count = dbSession->fetchActiveNotifications(activeNotifications, viewName);

  if (serviceInfo.sev != ngrt4n::Normal) {
    if (count <= 0) { // send new notif
      sendEmailNotification(serviceInfo, ngrt4n::Normal, qosData, notificationRecipients, preferences);
      dbSession->addNotification(viewName, serviceInfo.sev);
    } else { //
      NotificationT lastNotification = activeNotifications.front();
      if (lastNotification.view_status != serviceInfo.sev) { //severity changed
        sendEmailNotification(serviceInfo, lastNotification.view_status, qosData, notificationRecipients, preferences);
        dbSession->acknowledgeAllActiveNotifications("admin", viewName);
        dbSession->addNotification(viewName, serviceInfo.sev);
      } else {
        // FIXME: repeating notification: escalate it?
      }
    }
  } else {
    if (count > 0) { // if there were problems
      NotificationT lastNotification = activeNotifications.front();
      if (lastNotification.view_status != serviceInfo.sev) { // service recovered
        sendEmailNotification(serviceInfo, lastNotification.view_status, qosData, notificationRecipients, preferences);
        dbSession->acknowledgeAllActiveNotifications("admin", viewName);
      }
    }
  }

}

void runCollector(int period)
{
  DbSession dbSession;
  WebPreferencesBase preferences;
  while(1) {
    try {
      dbSession.updateUserList();
    } catch(const std::exception& ex) {
      std::cerr << ex.what() <<"\n";
    }

    REPORTD_LOG("notice", Q_TR("Collecting QoS data..."));
    long now = time(NULL);
    for (const auto& view: dbSession.viewList()) {

      QosCollector collector(view.path.c_str());
      collector.initialize(&preferences);

      // skip the view if initialization failed
      if (collector.lastErrorState()) {
        LOG("error", collector.lastErrorMsg().toStdString());
        continue;
      }

      collector.initSettings(&preferences);
      collector.runMonitor();

      QosDataT qosInfo = collector.qosInfo();
      qosInfo.timestamp = now;
      try {
        dbSession.addQosData(qosInfo);
        handleNotification(collector.rootNode(), qosInfo, &dbSession, preferences);
        REPORTD_LOG("notice", dbSession.lastError());
      } catch(const std::exception& ex) {
        REPORTD_LOG("warn", ex.what());
      }
    }

    sleep(period);
  }
}

int main(int argc, char **argv)
{
  RealOpInsightQApp qtApp (argc, argv);
  int period = 5;
  bool ok;
  int opt;
  if ((opt = getopt(argc, argv, "t:dh")) != -1) {
    switch (opt) {
      case 't':
        period = QString(optarg).toInt(&ok);
        if (! ok || period < 1)
          period = 1;
        break;
      case 'h':
        break;
      default:
        break;
    }
  }

  period *= 60;

  REPORTD_LOG("notice", QObject::tr("Reporting collector started. Interval: %1 second(s)").arg(QString::number(period)).toStdString());
  runCollector(period); // convert period in seconds

  return qtApp.exec();
}
