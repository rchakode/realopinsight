/*
 * ngrt4n.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */

#ifndef MONITORBROKER_HPP_
#define MONITORBROKER_HPP_
#include<string>
#include<iostream>
#include <unordered_map>

using namespace std ;

class MonitorBroker {
public:

  enum MonirorTypeT {
    NAGIOS = 0,
    ZABBIX = 1,
    ZENOSS = 2
  };

  enum CriticityT {
    CRITICITY_NORMAL = 0,
    CRITICITY_MINOR = 1,
    CRITICITY_MAJOR = 2,
    CRITICITY_HIGH = 3,
    CRITICITY_UNKNOWN = 100
  };

  enum NAGIOS_StatusT {
    NAGIOS_OK = 0,
    NAGIOS_WARNING = 1,
    NAGIOS_CRITICAL = 2,
    NAGIOS_UNKNOWN = 3
  };

  enum ZABBIX_SeverityT {
    ZABBIX_UNCLASSIFIED = 0,
    ZABBIX_INFO = 1,
    ZABBIX_WARN = 2,
    ZABBIX_AVERAGE = 3,
    ZABBIX_HIGH = 4,
    ZABBIX_DISASTER = 5
  };

  enum ZENOSS_SeverityT {
    ZENOSS_CLEAR = 0,
    ZENOSS_DEBUG = 1,
    ZENOSS_INFO = 2,
    ZENOSS_WARNING = 3,
    ZENOSS_ERROR = 4,
    ZENOSS_CRITICAL = 5
  };


  typedef struct _CheckT{
    string id;
    string host ;
    string check_command ;
    string last_state_change ;
    string alarm_msg ;
    int status ;
  }CheckT;
  typedef unordered_map<string, CheckT> ChecksT ;

  MonitorBroker(const string & _sfile);
  virtual ~MonitorBroker();

  string getInfOfService(const string & _sid) ;
  static bool loadNagiosCollectedData(const string & _sfile, ChecksT & _checks) ;

  static const int DEFAULT_PORT ;
  static const int DEFAULT_UPDATE_INTERVAL ;
  static const int MAX_MSG ;

private:
  int lastUpdate ;
  string statusFile ;
  ChecksT services ;
};

#endif /* MONITORBROKER_HPP_ */
