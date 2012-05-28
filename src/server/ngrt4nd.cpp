/*
 * ngrt4n.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012												   #
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
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include <crypt.h>
#include <fstream>

using namespace std;


class GetServiceInfo : public xmlrpc_c::method {
public:
	GetServiceInfo(const string& _sfile, const string& _authchain)
	: monitor(new MonitorBroker(_sfile)),
	  authChain (_authchain) {
		this->_signature = "s:ss";
		this->_help = "This method gets the status of a given service through a string";
	}
	void
	execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP) {

		string const sid(paramList.getString(0));
		string const auth(paramList.getString(1));
		paramList.verifyEnd(2);

		if(auth == authChain) {
			*retvalP = xmlrpc_c::value_string( monitor->getInfOfService(sid) );
		}else {
			*retvalP = xmlrpc_c::value_string( "-2<==>Unauthenticated request" );
			cerr << "Unauthenticated request " << endl ;
		}
	}
	~GetServiceInfo(){ delete monitor ;}

private:
	MonitorBroker* monitor ;
	string authChain ;
};



int
main(int argc, char ** argv) {

	if( getuid() != 0) {
		cerr << "The program must be run as root" << endl;
		exit(1) ;
	}

	string passwordFile = string(getenv("HOME")) + "/.ngrt4nd_auth" ;
	string salt = "$1$$";
	string statusFile = "/usr/local/nagios/var/status.dat" ;
	bool foreground = false;
	static const char *shotOpt="dpc:l:" ;
	int port = 0 ;
	char opt ;
	while ((opt = getopt(argc, argv, shotOpt)) != -1) {
		switch (opt) {
		case 'd':		// daemon mode
			foreground = true;
			break;
		case 'c':		// alternative location of status.dat
			statusFile = optarg ;
			break;
		case 'l':		// alternative location of status.dat
			port = atoi(optarg) ;
			break;
		case 'p': {		// force to remove existing semaphore
			char* password = getpass("Ngrt4n Password:");
			char* rePassword = getpass("Confirm the Password:");

			if( static_cast<string>(password) != static_cast<string>(rePassword) ) {
				cerr << "The two password are different. Retry." << endl ;
				exit(1) ;
			}
			ofstream ofpass;
			ofpass.open (passwordFile.c_str());
			ofpass << crypt(password, salt.c_str());
			ofpass.close();
			cout << "Password reseted"<< endl ;
			exit(0) ;
			break;
		}
		default:
			cerr << "Unknow option : " << opt << endl;
			break;
		}
	}

	string authChain ;
	ifstream pfile;
	pfile.open (passwordFile.c_str());
	pfile >> authChain ;
	pfile.close();

	if( foreground ) {
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

	try {
		xmlrpc_c::registry registry;

		xmlrpc_c::methodPtr const getServiceInfoP(new GetServiceInfo(statusFile, authChain));

		registry.addMethod("get.service.info", getServiceInfoP);

		if ( port <= 0 ) port = MonitorBroker::DEFAULT_PORT ;
		xmlrpc_c::serverAbyss server(
				xmlrpc_c::serverAbyss::constrOpt()
		        .registryP(&registry)
		        .portNumber(port)
		        .logFileName("/tmp/xmlrpc.log"));

		server.run();
		assert(false);
	} catch (exception const& e) {
		cerr << "Failure when starting the server : " << e.what() << endl;
	}
	return 0;
}
