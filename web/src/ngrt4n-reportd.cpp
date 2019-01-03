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

#include "dbo/src/DbSession.hpp"
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
#include <regex>


void wait_for_interval(int interval)
{
  unsigned int remaining = static_cast<unsigned int>(interval);
  while ((remaining = sleep(remaining)) > 0);
}

void runCollector(int period)
{
  ngrt4n::initReportdLogger();

  WebBaseSettings settings;
  DbSession dbSession(settings.getDbType(), settings.getDbConnectionString());
  Notificator notificator(&dbSession);
  while(1) {
    QosDataList qosDataList;
    NodeListT rootNodes;
    qosDataList.clear();
    rootNodes.clear();
    DbViewsT vlist;
    try {
      vlist = dbSession.listViews();
    } catch(const std::exception& ex) {
      std::cerr << ex.what() <<"\n";
    }

    for (const auto& view: vlist) {
      QosCollector collector;
      collector.setDbSession(&dbSession);

      auto initilizeOut = collector.initialize(&settings, view.path.c_str());
      if (initilizeOut.first != ngrt4n::RcSuccess) {
        REPORTD_LOG("error", QObject::tr("%1: %2").arg(view.name.c_str(), initilizeOut.second).toStdString());
        continue;
      }

      collector.loadDataSources();
      auto updateOut = collector.updateAllNodesStatus();
      if (updateOut.first != ngrt4n::RcSuccess) {
        REPORTD_LOG("error", updateOut.second.toStdString());
        continue;
      }

      QosDataT qosData = collector.qosInfo();
      if (qosData.view_name != view.name && std::regex_match(view.name, std::regex("Source[0-9]:.+"))) {
        qosData.view_name = view.name;
      }
      qosData.timestamp = time(nullptr); // now
      qosDataList.push_back(qosData);
      rootNodes[qosData.view_name.c_str()] = collector.rootNode();
      try {
        dbSession.addQosData(qosData);
      } catch(const std::exception& ex) {
        REPORTD_LOG("error", std::string(ex.what()));
      }
    }

    // handle notifications if applicable
    if (settings.getNotificationType() != WebBaseSettings::NoNotification) {
      for (const auto& qosEntry : qosDataList) {
        notificator.handleNotification(rootNodes[qosEntry.view_name.c_str()], qosEntry);
      }
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
