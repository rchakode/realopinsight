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
#include <zmq.hpp>
#include <crypt.h>
#include <fstream>
#include <sstream>

using namespace std;

int numWorkers = 2 ;
string statusFile = "/usr/local/nagios/var/status.dat" ;

void *worker_routine (void *arg)
{
	zmq::context_t *ctx = (zmq::context_t*) arg;
	zmq::socket_t s (*ctx, ZMQ_REP);
	s.connect ("inproc://workers");

	MonitorBroker* monitor = new MonitorBroker( statusFile ) ;

	while (true) {
		zmq::message_t request;
		s.recv (&request);

		//TODO
		string result =  monitor->getInfOfService(static_cast<char*>(request.data())) ;

		zmq::message_t reply (10);
		memset (reply.data (), 0, result.size ());
		s.send (reply);
	}

	return NULL ;
}


int main(int argc, char ** argv) {

	if( getuid() != 0) {
		cerr << "The program must be run as root" << endl;
		exit(1) ;
	}

	string passwordFile = string(getenv("HOME")) + "/.ngrt4nd_auth" ;
	string salt = "$1$$";
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
	ostringstream tcpAddr;  tcpAddr << "tcp://*:" << port ;
	zmq::context_t ctx(1);
	zmq::socket_t workers(ctx, ZMQ_XREQ);
	workers.bind("inproc://workers");
	zmq::socket_t clients(ctx, ZMQ_XREP);
	clients.bind(tcpAddr.str().c_str());

	for (int i = 0; i < numWorkers; i++) {
		pthread_t worker;
		int rc = pthread_create (&worker, NULL, worker_routine, (void*) &ctx);
		assert (rc == 0);
	}

	zmq::device (ZMQ_QUEUE, clients, workers);


	return 0;
}
