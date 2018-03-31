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
#include "WebBaseSettings.hpp"
#include "QosCollector.hpp"
#include "WebUtils.hpp"
#include "Applications.hpp"
#include "Notificator.hpp"
#include "utils/smtpclient/MailSender.hpp"
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <QString>
#include <getopt.h>
#include <unistd.h>


void wait_for_interval(int interval)
{
  int remaining = interval;
  while ((remaining = sleep(remaining)) > 0);
}

void runCollector(int period)
{
  ngrt4n::initReportdLogger();

  WebBaseSettings settings;
  DbSession dbSession(settings.getDbType(), settings.getDbConnectionString());
  Notificator notificator(&dbSession);
  while(1) {
    try {
      dbSession.updateUserList();
    } catch(const std::exception& ex) {
      std::cerr << ex.what() <<"\n";
    }

    QosDataList qosDataList;
    NodeListT rootNodes;
    qosDataList.clear();
    rootNodes.clear();
    long now = time(NULL);
    for (const auto& view: dbSession.viewList()) {
      // initialize a collector for the current view.
      // skip the view if the initialization failed
      QosCollector collector(view.path.c_str());
      collector.initialize(&settings);
      if (collector.lastErrorState()) {
        REPORTD_LOG("error", collector.lastErrorMsg());
        continue;
      }
      collector.initSettings(&settings);
      collector.updateAllNodesStatus(&dbSession);
      QosDataT qosData = collector.qosInfo();
      qosData.timestamp = now;
      qosDataList.push_back(qosData);
      rootNodes[qosData.view_name.c_str()] = collector.rootNode();
      try {
        dbSession.addQosData(qosData);
        REPORTD_LOG("notice", dbSession.lastError());
      } catch(const std::exception& ex) {
        REPORTD_LOG("warn", std::string(ex.what()));
      }
    }
    // now handle notifications if applicable
    if (settings.getNotificationType() != WebBaseSettings::NoNotification) {
      for (const auto qosEntry : qosDataList)
        notificator.handleNotification(rootNodes[qosEntry.view_name.c_str()], qosEntry);
    }
    wait_for_interval(period);
  }

  ngrt4n::freeReportdLogger();
}

int main(int argc, char **argv)
{
  RoiQApp qtApp(argc, argv);

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

  REPORTD_LOG("notice", QObject::tr("Reporting collector started"));
  REPORTD_LOG("notice", QObject::tr(" => Interval: %1 second(s)").arg(QString::number(period)));
  runCollector(period); // convert period in seconds

  return qtApp.exec();
}
