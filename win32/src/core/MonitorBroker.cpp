/*
 * MonitorBroker.cpp
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

#include "core/MonitorBroker.hpp"
#include "core/ns.hpp"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <time.h>


const int MonitorBroker::DEFAULT_PORT = 1983 ;
const int MonitorBroker::DEFAULT_UPDATE_INTERVAL = 300 ;
const int MonitorBroker::MAX_MSG = 512 ;

MonitorBroker::MonitorBroker(const string & _sfile)
    : lastUpdate(0),
      statusFile(_sfile) {}

MonitorBroker::~MonitorBroker() {}

string MonitorBroker::getInfOfService(const string & _sid)
{
    long curTime = time(NULL) ;
    if( (curTime - lastUpdate) >= DEFAULT_UPDATE_INTERVAL) {
        loadNagiosCollectedData(statusFile, services) ;
        lastUpdate = curTime ;
    }

    NagiosChecksT::iterator it = services.find(_sid) ;

    if (it == services.end() ) {
        return "{\"return_code\":\"-1\",\"message\":\"ERROR: Unknow service '" + _sid + "'\"}" ;
    }

    ostringstream ret ;
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

bool MonitorBroker::loadNagiosCollectedData(const string & _sfile, NagiosChecksT & _checks)
{

    ifstream stFileStream ;
    stFileStream.open(_sfile.c_str(), std::ios_base::in) ;
    if (! stFileStream.good() ) {
        cerr << "ERROR: Unable to open the file " << _sfile << endl ;
        return false ;
    }

    string line;
    while (getline(stFileStream, line) , ! stFileStream.eof()) {

        if(line.find("#") != string::npos ) continue ;

        if( line.find("hoststatus") == string::npos &&
                line.find("servicestatus") == string::npos ) continue ;

        NagiosCheckT info;
        info.status = UNSET_STATUS ;
        while (getline(stFileStream, line), ! stFileStream.eof()) {

            size_t pos = line.find("}") ; if( pos != string::npos ) break ;
            pos = line.find("=") ; if(pos == string::npos) continue ;
            string param = ngrt4n::trim(line.substr(0, pos));
            string value = ngrt4n::trim(line.substr(pos+1, string::npos)) ;
            if(param == "host_name") {
                info.host = info.id =
                        ngrt4n::trim(line.substr(pos+1)) ;
            }
            else if(param == "service_description") {
                info.id += "/" + value ;
            }
            else if(param == "check_command") {
                info.check_command = value ;
            }
            else if(param == "current_state") {
                info.status = atoi(value.c_str()) ;
            }
            else if(param == "last_state_change") {
                info.last_state_change = value ;
            }
            else if(param == "plugin_output")
            {
                info.alarm_msg = value;
            }
        }
        _checks[info.id] = info;

    }
    stFileStream.close() ;

    return true;
}

