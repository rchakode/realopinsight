/*
 * ngrt4n.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
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


#ifndef MONITORBROKER_HPP_
#define MONITORBROKER_HPP_
#include<string>
#include<iostream>
#include <unordered_map>

struct CheckT{
  std::string id;
  std::string host;
  std::string check_command;
  std::string last_state_change;
  std::string alarm_msg;
  int status;
};
typedef std::unordered_map<std::string, CheckT> ChecksT;

class MonitorBroker {
public:
  enum ApiTypeT {
    Nagios = 0,
    Zabbix = 1,
    Zenoss = 2,
    Auto=99
  };
  enum SeverityT {
    Normal = 0,
    Minor = 1,
    Major = 2,
    Critical = 3,
    Unknown = 100
  };
  enum NagiosStatusT {
    NagiosOk = 0,
    NagiosWarning = 1,
    NagiosCritical = 2,
    NagiosUnknown = 3
  };
  enum ZabbixSeverityT {
    ZabbixClear = 0,
    ZabbixInfo = 1,
    ZabbixWarn = 2,
    ZabbixAverage = 3,
    ZabbixHigh = 4,
    ZabbixDisaster = 5
  };
  enum ZenossSeverityT {
    ZenossClear = 0,
    ZenossDebug = 1,
    ZenossInfo = 2,
    ZenossWarning = 3,
    ZenossError = 4,
    ZenossCritical = 5
  };

  static const int DefaultPort;
  static const int DefaultUpdateInterval;
  static const int MaxMsg;

  MonitorBroker(const std::string& _sfile);
  virtual ~MonitorBroker();
  std::string getInfOfService(const std::string& _sid);
  static bool loadNagiosCollectedData(const std::string& _sfile, ChecksT& _checks);

private:
  int lastUpdate;
  std::string statusFile;
  ChecksT services;
};
#endif /* MONITORBROKER_HPP_ */
