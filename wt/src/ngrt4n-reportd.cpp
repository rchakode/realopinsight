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

#include "DbSession.hpp"
#include "WebPreferences.hpp"
#include "QosCollector.hpp"
#include "WebUtils.hpp"
#include "Applications.hpp"
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <QString>
#include <getopt.h>
#include <unistd.h>

void runCollector(int period, bool foreground)
{
  std::unique_ptr<DbSession> dbSession(new DbSession());
  std::unique_ptr<WebPreferencesBase> preferences(new WebPreferencesBase());
  while(1) {

    try {
      dbSession->updateUserList();
    } catch(const std::exception& ex) {
      std::cerr << ex.what() <<"\n";
    }

    std::vector<QosCollector*> mycollectors;
    for (auto view: dbSession->viewList()) {
      QosCollector* collector = new QosCollector(view.path.c_str());
      collector->initialize(preferences.get());
      collector->initSettings(preferences.get());
      mycollectors.push_back(collector);
    }

    LOG("notice", Q_TR("Collecting QoS data..."));
    long now = time(NULL);
    for (auto collector: mycollectors) {
      collector->runMonitor();
      DbQosDataT qosInfo = collector->qosInfo();
      qosInfo.timestamp = now;
      try {
        dbSession->addQosInfo(qosInfo);
        if (foreground) {
          std::cerr << dbSession->lastError()<<"\n";
        } else {
          LOG("notice", dbSession->lastError());
        }
      } catch(const std::exception& ex) {
        std::cerr << ex.what() <<"\n";
      }
    }

    // clean up data
    for (auto collector: mycollectors) {
      delete collector;
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
  bool foreground = true;
  if ((opt = getopt(argc, argv, "t:dh")) != -1) {
    switch (opt) {
      case 't':
        period = QString(optarg).toInt(&ok);
        if (! ok || period < 1)
          period = 1;
        break;
      case 'd':
        foreground = false;
        break;
      case 'h':
        break;
      default:
        break;
    }
  }

  period *= 60;
  std::string startupMsg = QObject::tr("Reporting collector started. Interval: %1 second(s)").arg(QString::number(period)).toStdString();
  if (! foreground) {
    pid_t pid = fork();

    if (pid < 0) {
      std::cerr << "Fork failed \n";
      exit(EXIT_FAILURE);
    } else if (pid == 0) {
      LOG("notice", startupMsg);
      setsid();
      runCollector(period, foreground); // convert period in seconds
    } else {
      exit(EXIT_SUCCESS);
    }
  } else {
    std::cerr << startupMsg <<"\n";
    runCollector(period, foreground); // convert period in seconds
  }


  return qtApp.exec();
}
