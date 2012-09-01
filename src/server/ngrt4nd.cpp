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
#include "core/ZmqHelper.hpp"
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

string packageName = PACKAGE_NAME ;
string packageVersion = PACKAGE_VERSION ;
string packageUrl = PACKAGE_URL ;
string statusFile = "/usr/local/nagios/var/status.dat" ;
string progName = "";
string authChain= "" ;

string help() {
    ostringstream msg("SYNOPSIS\n"
                      "	" + progName +" [OPTIONS]\n"
                      "\n"
                      "OPTIONS\n"
                      "	-c FILE\n"
                      "	 Specify the path of the Nagios status file. Default is " + statusFile + ".\n"
                      "	-D\n"
                      "	 Run the program in foreground mode. \n"
                      "	-p\n"
                      "	 Set the listening port. Default is 1983.\n"
                      "	-P\n"
                      "	 Change the authentication token.\n"
                      "	-T\n"
                      "	 Print the authentication token.\n"
                      "	-v\n"
                      "	 Print the version and copyright information.\n"
                      "	-h\n"
                      "	 Print this help.\n") ;

    return msg.str();
}

string version(){
    ostringstream versionMsg(progName + " (" + packageName + "), Version " + packageVersion +  ".\n"
                             + "Copyright (c) 2010-2012 NGRT4N Project <contact@ngrt4n.com>.\n"
                             + "All rights reserved. Visit "+ packageUrl + " for more information.");

    return versionMsg.str();
}

void ngrt4n::setPassChain(char* authChain) {

    ofstream ofpass;

    ofpass.open( ngrt4n::AUTH_FILE.c_str() );
    if( ! ofpass.good()) {
        cerr << "Unable to set the authentication token." << endl;
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
        cerr << "Unable to load the application's settings" << endl;
        exit(1) ;
    }

    pfile >> authChain ;
    pfile.close();
    return authChain ;
}

int main(int argc, char ** argv)
{
    progName = basename(argv[0]);

    bool foreground = false;
    static const char *shotOpt="DTPhvc:p:" ;
    int port = MonitorBroker::DEFAULT_PORT ;
    char opt ;
    while ((opt = getopt(argc, argv, shotOpt)) != -1) {
        switch (opt)
        {
        case 'D':
            foreground = true ;
            break;

        case 'c':
            statusFile = optarg ;
            break;

        case 'p': {
            port = atoi(optarg) ;
            if(port <= 0 ) {
                cerr << "ERROR: bad port number." << endl ;
                exit(1) ;
            }
            break;
        }

        case 'P': {
            ngrt4n::checkUser() ;
            ngrt4n::initApp() ;

            char* pass = getpass("Type a passphrase:");
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
            cout << version() << endl ;
            exit(0) ;
        }

        case 'h': {
            cout << help() << endl ;
            exit(0) ;
        }
        default: {
            cout << help() << endl ;
            exit(1) ;
        }
        }
    }

    ngrt4n::checkUser() ;
    ngrt4n::initApp() ;
    authChain = ngrt4n::getPassChain() ;

    if( ! foreground ) {
        pid_t pid = fork();
        if(pid <= -1) {
            cerr << "ERROR: failed while starting the program in daemon mode" << endl;
            exit(1);
        }
        else if(pid > 0) {
            exit (0);
        }
        setsid();
    }

    ostringstream tcpAddr;
    tcpAddr << "tcp://*:" << port ;

    cout << "Starting "<< version() << endl << endl ;
    cout << "Listening address => " << tcpAddr.str() << endl ;
    cout << "Nagios status file => " << statusFile << endl ;

    zmq::context_t ctx(1);
    zmq::socket_t* comChannel = new zmq::socket_t(ctx, ZMQ_REP);
    comChannel->bind(tcpAddr.str().c_str());

    cout << "Service started." << endl ;

    MonitorBroker* monitor = new MonitorBroker( statusFile ) ;
    while (true) {

        string msg = ZmqHelper::recvFromSocket(*comChannel) ;
        string reply = "" ;
        if(msg == "PING") {
            reply = "ALIVE";
        } else {
            size_t pos = msg.find(":") ;
            string pass = "";
            string sid = "";
            if(pos != string::npos) {
                pass = msg.substr(0, pos);
                sid = msg.substr(pos+1, string::npos);
            }
            if(pass == authChain) {
                reply = monitor->getInfOfService(sid) ;
            } else {
                reply = "{\"return_code\" : \"-2\", \"message\" : \"ERROR: Wrong authentication\"}";
            }
        }

        ZmqHelper::sendFromSocket(*comChannel, reply) ;
    }

    comChannel->close() ;
    delete comChannel ;

    return 0;
}
