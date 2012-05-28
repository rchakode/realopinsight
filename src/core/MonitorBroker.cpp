/*
 * ngrt4n.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 2-05-2012												   #
#																		   #
# This file is part of NGRT4N (http://ngrt4n.com).						   #
#																		   #
# NGRT4N is free software: you can redistribute it and/or modify		   #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.									   #
#																		   #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of		   #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	           #
# GNU General Public License for more details.							   #
#																		   #
# You should have received a copy of the GNU General Public License		   #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.		   #
#--------------------------------------------------------------------------#
 */

#include "core/MonitorBroker.hpp"
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <sstream>
#include <stdlib.h>


const int MonitorBroker::DEFAULT_PORT = 1983 ;
const int MonitorBroker::DEFAULT_UPDATE_INTERVAL = 300 ;

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
		return "-1<==>Unknow service : " + _sid ;
	}

	ostringstream ret ;
	ret << _sid
			<<"<==>" << it->second.status
			<< "<==>" << it->second.host
			<< "<==>" << it->second.last_state_change
			<< "<==>" << it->second.alarm_msg
			<< "<==>" << it->second.check_command;
	return ret.str() ;
}

bool MonitorBroker::loadNagiosCollectedData(const string & _sfile, NagiosChecksT & _checks)
{
	string line, check_type;
	NagiosCheckT info;

	/* First make a snapshot of the status file before treat it ; */
	string snapshot = "/tmp/status.dat.snap" ;
	FILE* stFile = fopen(_sfile.c_str(), "rt") ;
	FILE* fSnapshot =  fopen(snapshot.c_str(), "wt") ;

	if( stFile == NULL || fSnapshot == NULL ){
		cerr << "Unable to access to check the status file : " << _sfile << endl;
		return false ;
	}

	fseek(stFile, 0, SEEK_END) ; size_t size = ftell(stFile) ; rewind(stFile) ;

	char* buffer = (char*)malloc(size * sizeof(char)) ;

	size_t nbRead = fread(buffer, 1, size, stFile);
	if(nbRead != size){
		cerr << "Error while reading the status file : " << _sfile << endl;
		return false ;
	}

	size_t nbWrite = fwrite(buffer, 1, size, fSnapshot);

	if(nbWrite != size){
		cerr << "Error while creating a snapshot of the status file : " << endl;
		return false ;
	}

	fcloseall() ; // End of the copy

	/* Now start parsing */
	ifstream stFileStream ;
	stFileStream.open(snapshot.c_str(), std::ios_base::in) ;
	if (! stFileStream.good() ) {
		cerr << "Unable to access the snapshot of the the status file " << endl ;
		return false ;
	}

	while (getline(stFileStream, line) , ! stFileStream.eof()) {

		if(line.find("#") != string::npos ) continue ;

		if( line.find("hoststatus") == string::npos &&
				line.find("servicestatus") == string::npos ) continue ;

		info.status = UNSET_STATUS ;
		while (getline(stFileStream, line), ! stFileStream.eof()) {

			size_t pos = line.find("}") ; if( pos != string::npos ) break ;
			pos = line.find("=") ; if(pos == string::npos) continue ;

			string param = boost::trim_copy(line.substr(0, pos));
			string value = boost::trim_copy(line.substr(pos+1, string::npos)) ;

			if(param == "host_name") {
				info.host =
						info.id =
								boost::trim_copy(line.substr(pos+1)) ;
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

