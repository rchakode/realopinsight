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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <QString>
#include <getopt.h>
#include <unistd.h>
#include "DbSession.hpp"
#include "WebPreferences.hpp"
#include "QosCollector.hpp"
#include "WebUtils.hpp"


int main(int argc, char **argv)
{

  int period = 60;
  bool ok;
  int opt;
  if ((opt = getopt(argc, argv, "t:h")) != -1) {
    switch (opt) {
      case 't':
        period = QString(optarg).toInt(&ok);
        if (! ok || period < 60)
          period = 60;
        break;
      case 'h':
        break;
      default:
        break;
    }
  }

  std::unique_ptr<DbSession> dbSession(new DbSession());
  while(1) {

    WebPreferencesBase* preferences = new WebPreferencesBase();
    dbSession->updateUserList();

    long now = time(NULL);
    std::vector<QosCollector*> mycollectors;
    for (auto view: dbSession->viewList()) {
      QosCollector* collector = new QosCollector(view.path.c_str());
      collector->initialize(preferences);
      mycollectors.push_back(collector);
    }

    LOG("notice", Q_TR("Collecting QoS data..."));
    for (auto collector: mycollectors) {
      collector->runMonitor();
      DbQosInfoT qosInfo = collector->qosInfo();
      qosInfo.timestamp = now;
      dbSession->addQosInfo(qosInfo);
    }

    // free up resources
    for (auto collector: mycollectors) {
      delete collector;
    }
  }

  return 0;
}
