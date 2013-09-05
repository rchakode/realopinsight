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
#include "core/ZmqSocket.hpp"
#include <zmq.h>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <crypt.h>
#include <sstream>
#include <fstream>
#include <libgen.h>
#include <iostream>
#include <stdio.h>
#include <memory>


std::string packageName = PACKAGE_NAME;
std::string packageVersion = PACKAGE_VERSION;
std::string packageUrl = PACKAGE_URL;
std::string statusFile = "/usr/local/nagios/var/status.dat";
std::string progName = "";
std::string authChain= "";

std::string help() {
  std::ostringstream msg("SYNOPSIS\n"
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
                         "	 Print this help.\n\n");
  return msg.str();
}

std::string getVersionMsg(const std::string& progName)
{
  char msg[1024];
  sprintf(msg, "> %s (%s) version %s"
          "\n>> Belongs to NGRT4N Dashboard Monitoring Suite"
          "\n>> Copyright (c) 2010-2013 NGRT4N Project. All rights reserved"
          "\n>> License GNU GPLv3 or later <http://gnu.org/licenses/gpl.html>"
          "\n>> For bug reporting instructions, see: <%s>",
          packageName.c_str(), progName.c_str(), packageVersion.c_str(), packageUrl.c_str());
  return std::string(msg);
}

void ngrt4n::setPassChain(char* authChain)
{
  std::ofstream ofpass;
  ofpass.open(ngrt4n::AUTH_FILE.c_str());
  if(!ofpass.good()) {
      std::clog << "Unable to set the authentication token." << "\n";
      exit(1);
    }
  ofpass << crypt(authChain, salt.c_str());
  ofpass.close();
}

std::string ngrt4n::getPassChain()
{
  std::string authChain;
  std::ifstream pfile;
  pfile.open (ngrt4n::AUTH_FILE.c_str());
  if(!pfile.good()) {
      std::clog << "Unable to load the application's settings" << "\n";
      exit(1);
    }

  pfile >> authChain;
  pfile.close();
  return authChain;
}

int main(int argc, char ** argv)
{
  progName = basename(argv[0]);
  int port = MonitorBroker::DefaultPort;
  bool foreground = false;
  char opt;
  static const char *shotOpt="DTPhvc:p:";
  while ((opt = getopt(argc, argv, shotOpt)) != -1) {
      switch (opt){
        case 'D':
          foreground = true;
          break;
        case 'c':
          statusFile = optarg;
          break;
        case 'p':
          port = atoi(optarg);
          if(port <= 0 ) {
              std::cerr << "Bad port number\n";
              exit(1);
            }
          break;
        case 'P':
          {
            ngrt4n::checkUser();
            ngrt4n::initApp();
            char* pass = getpass("Type a passphrase:");
            ngrt4n::setPassChain(pass);
            std::cout << ngrt4n::getPassChain()<<"\n";
            exit(0);
          }
        case 'T':
          ngrt4n::checkUser();
          std::cout << ngrt4n::getPassChain()<<"\n";
          exit(0);
        case 'v':
          std::cout << getVersionMsg(progName)<<"\n";
          exit(0);
        case 'h':
          std::cout << help();
          exit(0);
        default:
          std::cout << help();
          exit(1);
        }
    }
  ngrt4n::checkUser();
  ngrt4n::initApp();
  authChain = ngrt4n::getPassChain();
  std::clog << getVersionMsg(progName)<< "\nStarting...\n";
  if(!foreground) {
      pid_t pid = fork();
      if(pid <= -1) {
          std::clog << "Failed while starting the daemon service\n";
          exit(1);
        } else if(pid > 0) {
          exit (0);
        }
      setsid();
    }

  std::ostringstream uri;
  uri << "tcp://0.0.0.0:" << port;
  ZmqSocket socket(ZMQ_REP);
  if(!socket.bind(uri.str())) {
      std::clog << "ERROR\n";exit(1);
    }
  std::clog << "Listening address => "<<uri.str()
            << "\nNagios status file => "<<statusFile
            << "\n============>started\n";

  auto monitor = std::unique_ptr<MonitorBroker>(new MonitorBroker(statusFile));
  while (true) {
      std::string msg = socket.recv();
      std::string reply;
      if(msg == "PING") {
          reply = "ALIVE:"+packageVersion;
        } else {
          size_t pos = msg.find(":");
          std::string pass = "";
          std::string sid = "";
          if(pos != std::string::npos) {
              pass = msg.substr(0, pos);
              sid = msg.substr(pos+1, std::string::npos);
            }
          if(pass == authChain) {
              reply = monitor->getInfOfService(sid);
            } else {
              reply = "{\"return_code\" : \"-2\", \"message\" : \"Authentication failed\"}";
            }
        }
      socket.send(reply);
    }
  monitor.reset(NULL);
  return 0;
}
