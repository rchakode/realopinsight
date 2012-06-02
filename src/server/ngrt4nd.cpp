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

#include "core/ns.hpp"
#include "core/MonitorBroker.hpp"
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <zmq.hpp>
#include <crypt.h>
#include <fstream>
#include <sstream>
#include <zmq.h>

using namespace std;


pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int numWorkers = 2 ;
string statusFile = "/usr/local/nagios/var/status.dat" ;
ostringstream help(""
		"SYNOPSIS\n"
		"	ngrt4nd [OPTIONS]\n"
		"\n"
		"DESCRIPTION\n"
		"	Start the " + ngrt4n::APP_NAME + " server.\n"
		"	-c FILE\n"
		"	  specifies the path of the status file. Default : " + statusFile + ".\n"
		"	-d\n"
		"	  start the server like a daemon.\n"
		"	-p\n"
		"	  sets the port of listening. Default : 1983.\n"
		"	-a\n"
		"	  change the authentification passphrase.\n"
		"	-h\n"
		"	  print this help.\n") ;

void *worker_routine (void *arg)
{
	zmq::context_t *ctx = (zmq::context_t*) arg;
	zmq::socket_t comChannel (*ctx, ZMQ_REP);
	comChannel.connect ("inproc://workers");

	MonitorBroker* monitor = new MonitorBroker( statusFile ) ;
	while (true) {
		//TODO Authentificate request

		zmq::message_t request;
		request.rebuild() ;
		comChannel.recv(&request) ;
		size_t msize = request.size() ;
		char* sid = (char*)malloc(msize * sizeof(char)) ;
		memcpy(sid, request.data(), msize) ;

		string result =  monitor->getInfOfService(sid) + "\0" ;
		free(sid) ;
		msize = result.size() ;
		zmq::message_t reply( MonitorBroker::MAX_MSG) ;
		memset(reply.data(), 0, MonitorBroker::MAX_MSG) ;
		memcpy(reply.data(), result.c_str(), msize);
		comChannel.send(reply);
	}

	comChannel.close() ;
	return NULL ;
}


int main(int argc, char ** argv)
{


	string passwordFile = string(getenv("HOME")) + "/.ngrt4nd_auth" ;
	string salt = "$1$$";
	bool foreground = false;
	static const char *shotOpt="dahc:p:" ;
	int port = MonitorBroker::DEFAULT_PORT ;
	char opt ;
	while ((opt = getopt(argc, argv, shotOpt)) != -1) {
		switch (opt) {
		case 'd':		// daemon mode
			foreground = true;
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
		case 'a': {		// force to remove existing semaphore
			char* password = getpass("Type the passphrase:");
			char* rePassword = getpass("Retype the passphrase:");

			if( static_cast<string>(password) != static_cast<string>(rePassword) ) {
				cerr << "ERROR : The two passphrases are different." << endl ;
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
		case 'h':		// alternative location of status.dat
			cout << help.str() << endl ;
			exit(0) ;
		default:
			cerr << "Unknow option : " << opt << endl;
			cout << help.str() << endl ;
			exit(0) ;
		}
	}

	if( getuid() != 0) {
		cerr << "The program must be run as root" << endl;
		exit(1) ;
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

	zmq::context_t ctx(1);
	zmq::socket_t workersComChannel(ctx, ZMQ_XREQ);
	workersComChannel.bind("inproc://workers");

	ostringstream tcpAddr;
	tcpAddr << "tcp://*:" << port ;

	zmq::socket_t clientsComChannel(ctx, ZMQ_XREP);
	clientsComChannel.bind(tcpAddr.str().c_str());

	cout << "server started successfully : " << tcpAddr.str() << endl ;
	cout << "Status file : " << statusFile << endl ;

	for (int i = 0; i < numWorkers; i++) {
		pthread_t worker;
		int rc = pthread_create (&worker, NULL, worker_routine, (void*) &ctx);
		assert (rc == 0);
	}

	zmq::device (ZMQ_QUEUE, clientsComChannel, workersComChannel);

	return 0;
}
