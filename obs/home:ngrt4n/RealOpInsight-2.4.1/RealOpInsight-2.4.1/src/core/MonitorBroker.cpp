/*
 * MonitorBroker.cpp
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

#include "core/MonitorBroker.hpp"
#include "core/ns.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

const int MonitorBroker::DefaultPort = 1983;
const int MonitorBroker::DefaultUpdateInterval = 300;
const int MonitorBroker::MaxMsg = 512;

MonitorBroker::MonitorBroker(const std::string & _sfile)
  : lastUpdate(0),
    statusFile(_sfile) {}

MonitorBroker::~MonitorBroker() {}

std::string MonitorBroker::getInfOfService(const std::string & _sid)
{
  long curTime = time(NULL);
  if( (curTime - lastUpdate) >= DefaultUpdateInterval) {
      loadNagiosCollectedData(statusFile, services);
      lastUpdate = curTime;
    }
  ChecksT::iterator it = services.find(_sid);
  if (it == services.end() ) {
      return "{\"return_code\":\"-1\",\"message\":\"ERROR: Unknow service '" + _sid + "'\"}";
    }

  std::ostringstream ret;
  ret << "{"
      << "\"return_code\":0,"
      << "\"status\":"<< it->second.status<<","
      << "\"host\":\"" << it->second.host<<"\","
      << "\"lastchange\":\"" << it->second.last_state_change << "\","
      << "\"command\":\"" << it->second.check_command<<"\","
      << "\"message\":\"" << it->second.alarm_msg <<"\""
      << "}";
  return ret.str();
}

bool MonitorBroker::loadNagiosCollectedData(const std::string & _sfile, ChecksT & _checks)
{

  std::ifstream stFileStream;
  stFileStream.open(_sfile.c_str(), std::ios_base::in);
  if (! stFileStream.good() ) {
    std::cerr << "ERROR: Unable to open the file " << _sfile << "\n";
      return false;
    }

  std::string line;
  while (getline(stFileStream, line) , ! stFileStream.eof()) {

      if(line.find("#") != std::string::npos ) continue;

      if( line.find("hoststatus") == std::string::npos &&
          line.find("servicestatus") == std::string::npos ) continue;

      CheckT info;
      info.status = NagiosUnknown;
      while (getline(stFileStream, line), ! stFileStream.eof()) {

          size_t pos = line.find("}"); if( pos != std::string::npos ) break;
          pos = line.find("="); if(pos == std::string::npos) continue;
          std::string param = ngrt4n::trim(line.substr(0, pos));
          std::string value = ngrt4n::trim(line.substr(pos+1, std::string::npos));
          if(param == "host_name") {
              info.host = info.id =
                  ngrt4n::trim(line.substr(pos+1));
            }
          else if(param == "service_description") {
              info.id += "/" + value;
            }
          else if(param == "check_command") {
              info.check_command = value;
            }
          else if(param == "current_state") {
              info.status = atoi(value.c_str());
            }
          else if(param == "last_state_change") {
              info.last_state_change = value;
            }
          else if(param == "plugin_output")
            {
              info.alarm_msg = value;
            }
        }
     //std::transform(info.id.begin(), info.id.end(), info.id.begin(), ::tolower);
      _checks[info.id] = info;
    }
  stFileStream.close();

  return true;
}

