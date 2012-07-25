/*
 * ngrt4nd.cpp
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

#include "config.h"
#include "core/ns.hpp"
#include "core/MonitorBroker.hpp"
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <zmq.hpp>
#include <crypt.h>
#include <sstream>
#include <fstream>
#include <zmq.h>

using namespace std;

int numWorkers = 1 ;

string statusFile = "/usr/local/nagios/var/status.dat" ;
string authChain = "" ;
string packageName = PACKAGE_NAME ;

ostringstream help(""
		"SYNOPSIS\n"
		"	" + packageName +" [OPTIONS]\n"
		"\n"
		"OPTIONS\n"
		"	-c FILE\n"
		"	 Specify the path of the status file. Default is " + statusFile + ".\n"
		"	-D\n"
		"	 Run ngrt4nd in foreground mode. \n"
		"	-p\n"
		"	 Set the port of listening. Default is 1983.\n"
		"	-P\n"
		"	 Change the authentication token.\n"
		"	-T\n"
		"	 Print the authentication token.\n"
		"	-v\n"
		"	 Print the version and license information.\n"
		"	-h\n"
		"	 Print this help.\n") ;

void ngrt4n::setPassChain(char* authChain) {

	ofstream ofpass;

	ofpass.open( ngrt4n::AUTH_FILE.c_str() );
	if( ! ofpass.good()) {
		cerr << "Unable to set the password :  perhaps the application's settings file is not well configured." << endl;
		exit(1) ;
	}

	ofpass << crypt(authChain, salt.c_str());
	ofpass.close();
}

string ngrt4n::getPassChain() {

	string authChain ;
	ifstream pfile;

	pfile.open ( ngrt4n::AUTH_FILE.c_str() );
	if( ! pfile.good()) {
		cerr << "Unable to get application's settings" << endl;
		exit(1) ;
	}

	pfile >> authChain ;
	pfile.close();
	return authChain ;
}

int main(int argc, char ** argv)
{
	ostringstream versionMsg;
	versionMsg<< "NGRT4N Broker - " << PACKAGE_NAME <<" "<<PACKAGE_VERSION<< "."<< endl
			<<"This program is part of the NGRT4N Software." << endl
			<<"Copyright (c) 2010-2012 NGRT4N Project <contact@ngrt4n.com>" << "." << endl
			<<"Visit "<<PACKAGE_URL<<" for further details."<< endl ;

	bool foreground = false;
	static const char *shotOpt="DTPhvc:p:n:" ;
	int port = MonitorBroker::DEFAULT_PORT ;
	char opt ;
	while ((opt = getopt(argc, argv, shotOpt)) != -1) {
		switch (opt)
		{
		case 'D':		// daemon mode
			foreground = true ;
			break;

		case 'c':		// alternative location of status.dat
			statusFile = optarg ;
			break;

		case 'p': {	// alternative location of status.dat
			port = atoi(optarg) ;
			if(port <= 0 ) {
				cerr << "ERROR : Bad port number." << endl ;
				exit(1) ;
			}
			break;
		}

		case 'n': {	// alternative location of status.dat
			numWorkers = atoi(optarg) ;
			if(numWorkers <= 0 ) {
				cerr << "ERROR : Bad number of instances." << endl ;
				exit(1) ;
			}
			break;
		}

		case 'P': {
			ngrt4n::checkUser() ;
			ngrt4n::initApp() ;

			char* pass = getpass("Type the passphrase:");
			ngrt4n::setPassChain(pass) ;
			cout << ngrt4n::getPassChain() << endl ;

			exit(0) ;
		}

		case 'T': {
			ngrt4n::checkUser() ;
			cout << ngrt4n::getPassChain() << endl ;

			exit(0) ;
		}

		case 'v': {
			cout << versionMsg.str() ;
			exit(0) ;
		}

		case 'h': {		// alternative location of status.dat
			cout << help.str() << endl ;
			exit(0) ;
		}
		default: {
			cerr << "Unknow option : " << opt << endl;
			cout << help.str() << endl ;
			exit(0) ;
		}
		}
	}

	ngrt4n::checkUser() ;
	ngrt4n::initApp() ;
	authChain = ngrt4n::getPassChain() ;

	if( ! foreground ) {
		pid_t pid = fork();
		if(pid <= -1) {
			cerr << "Can not fork process" << endl;
			exit(1);
		}
		else if(pid > 0) {
			exit (0);
		}
		setsid();
	}

	ostringstream tcpAddr;
	tcpAddr << "tcp://*:" << port ;

	cout << versionMsg.str() ;
	cout << "Starting the program ..." << endl ;
	cout << "Listening address => " << tcpAddr.str() << endl ;
	cout << "Nagios status file => " << statusFile << endl ;

	zmq::context_t ctx(1);
	zmq::socket_t comChannel(ctx, ZMQ_REP);
	comChannel.bind(tcpAddr.str().c_str());

	MonitorBroker* monitor = new MonitorBroker( statusFile ) ;
	while (true) {
		zmq::message_t request;
		request.rebuild() ;
		comChannel.recv(&request) ;
		size_t msize = request.size() ;

		char* msg = (char*)malloc(msize * sizeof(char)) ;
		memcpy(msg, request.data(), msize) ;

		char* _pass = strtok(msg, ":") ;
		string pass = (_pass == NULL) ? "" : _pass ;
		char* _sid = strtok(NULL, ":") ;
		string sid = (_sid == NULL) ? "" : _sid ;

		string result = (pass == authChain )? monitor->getInfOfService(sid) : "-2#Wrong authentication" ;
		msize = result.size() ;
		zmq::message_t reply( MonitorBroker::MAX_MSG) ;
		memset(reply.data(), 0, MonitorBroker::MAX_MSG) ;
		memcpy(reply.data(), result.c_str(), msize);
		comChannel.send(reply);

		free(msg) ;
	}

	comChannel.close() ;

	return 0;
}
