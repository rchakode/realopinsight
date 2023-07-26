/*
 * realopinsight-reportd.cpp
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

#include "WebBaseSettings.hpp"
#include "PlatformStatusCollector.hpp"
#include "WebUtils.hpp"
#include "WebApplication.hpp"
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
#include <prometheus/gauge.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

void wait_for_interval(int interval)
{
  unsigned int remaining = static_cast<unsigned int>(interval);
  while ((remaining = sleep(remaining)) > 0);
}

void runCollector(int period)
{
  ngrt4n::initReportdLogger();

  prometheus::Exposer promExposer{"0.0.0.0:4584"};

  auto registry = std::make_shared<prometheus::Registry>();
  auto& promMetrics = prometheus::BuildGauge()
      .Name("realopinsight_probes_status_percent")
      .Help("Status of monitored platforms and related components")
      .Register(*registry);
  promExposer.RegisterCollectable(registry);


  while(1) {
    DbSession dbSession;
    WebBaseSettings settings;
    Notificator notificator;
    ListofPlatformStatusT platformStatusList;
    NodeListT rootNodes;
    DbViewsT vlist;

    platformStatusList.clear();
    rootNodes.clear();
    try {
      vlist = dbSession.listViews();
    } catch(const std::exception& ex) {
      std::cerr << ex.what() <<"\n";
    }

    for (const auto& view: vlist) {
      PlatformStatusCollector collector;
      collector.setDbSession(&dbSession);

      auto& promStatusOverall = promMetrics.Add({{"scope", view.name}, {"status", "summary"}});
      auto& promStatusCritical = promMetrics.Add({{"scope", view.name}, {"status", "critical"}});
      auto& promStatusUnknown = promMetrics.Add({{"scope", view.name}, {"status", "unknown"}});
      auto& promStatusMajor = promMetrics.Add({{"scope", view.name}, {"status", "major"}});
      auto& promStatusMinor = promMetrics.Add({{"scope", view.name}, {"status", "minor"}});
      auto& promStatusNormal = promMetrics.Add({{"scope", view.name}, {"status", "normal"}});

      auto initializeOut = collector.initialize(view.path.c_str());
      if (initializeOut.first != ngrt4n::RcSuccess) {
        REPORTD_LOG("error", QObject::tr("%1: %2").arg(view.name.c_str(), initializeOut.second).toStdString());
        promStatusOverall.Set(ngrt4n::Unknown);
        promStatusCritical.Set(-1);
        promStatusUnknown.Set(-1);
        promStatusMajor.Set(-1);
        promStatusMinor.Set(-1);
        promStatusNormal.Set(-1);
        continue;
      }

      collector.loadDataSources();
      auto updateOut = collector.updateAllNodesStatus();
      if (updateOut.first != ngrt4n::RcSuccess) {
        REPORTD_LOG("error", updateOut.second.toStdString());
        promStatusOverall.Set(ngrt4n::Unknown);
        promStatusCritical.Set(-1);
        promStatusUnknown.Set(-1);
        promStatusMajor.Set(-1);
        promStatusMinor.Set(-1);
        promStatusNormal.Set(-1);
        continue;
      }

      PlatformStatusT platformStatus = collector.info();
      if (platformStatus.view_name != view.name && std::regex_match(view.name, std::regex("Source[0-9]:.+"))) {
        platformStatus.view_name = view.name;
      }
      platformStatus.timestamp = time(nullptr); // now
      platformStatusList.push_back(platformStatus);
      rootNodes[platformStatus.view_name.c_str()] = collector.rootNode();
      promStatusOverall.Set(platformStatus.status);
      promStatusCritical.Set(platformStatus.critical);
      promStatusUnknown.Set(platformStatus.unknown);
      promStatusMajor.Set(platformStatus.major);
      promStatusMinor.Set(platformStatus.minor);
      promStatusNormal.Set(platformStatus.normal);

      try {
        dbSession.addPlatformStatus(platformStatus);
      } catch(const std::exception& ex) {
        REPORTD_LOG("error", std::string(ex.what()));
      }
    }

    // handle notifications if applicable
    if (settings.getNotificationType() != WebBaseSettings::NoNotification) {
      for (const auto& pfs : platformStatusList) {
        notificator.handleNotification(rootNodes[pfs.view_name.c_str()], pfs);
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
