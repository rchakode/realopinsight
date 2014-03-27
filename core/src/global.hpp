/*
 * ns.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-03-2014                                                  #
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


#ifndef NS_HPP_
#define NS_HPP_

#include<stdlib.h>
#include <string>
#include <unordered_map>

struct CheckT {
  std::string id;
  std::string host;
  std::string check_command;
  std::string last_state_change;
  std::string alarm_msg;
  int status;
};
typedef std::unordered_map<std::string, CheckT> ChecksT;

namespace ngrt4n {
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
    Unknown = 4
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

  enum {
    AdmUserRole = 100,
    OpUserRole = 101
  };

  enum VisibilityT {
    Hidden = 0x0,
    Visible = 0x1,
    Expanded = 0x2,
    Collapsed = 0xFC
  };
  const std::string AdmUser = "ngrt4n_adm";
  const std::string OpUser = "ngrt4n_op";
  const int MAX_FILE_UPLOAD = 2048; // 2MB
  const std::string CHILD_SEP = ",";
  const std::string TAG_ZABBIX_HOSTNAME = "\\{HOSTNAME\\}";
  const std::string TAG_ZABBIX_HOSTNAME2 = "\\{HOST.NAME\\}";
  const std::string TAG_HOSTNAME = "\\{hostname\\}";
  const std::string TAG_CHECK = "\\{check_name\\}";
  const std::string TAG_THERESHOLD = "\\{threshold\\}";
  const std::string TAG_PLUGIN_OUTPUT = "\\{plugin_output\\}";
  const std::string LINK_HOME ="/home";
  const std::string LINK_LOAD ="/preview-view";
  const std::string LINK_IMPORT ="/upload-view";
  const std::string LINK_LOGIN ="/login";
  const std::string LINK_LOGOUT ="/logout";
  const std::string LINK_ADMIN_HOME ="/adm-console";
  const std::string LINK_OP_HOME ="/op-console";
}

#endif /* NS_HPP_ */
