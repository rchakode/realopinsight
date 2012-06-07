/*
 * utils.cpp
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

#include "ns.hpp"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <exception>
#include <string>
#include <iostream>
#include <crypt.h>
#include <fstream>
using namespace std ;

void ngrt4n::initApp()
{
	int ret = mkdir(ngrt4n::APP_HOME.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) ;

	if(ret == -1 && errno != EEXIST) {
		cerr << "Unable to set the application files" ;
		EXIT_FAILURE ;
	}
}


void ngrt4n::checkUser() {
	if( getuid() != 0) {
		cerr << "The program must be run as root" << endl;
		exit(1) ;
	}
}


void ngrt4n::setPassChain(char* authChain) {

	ofstream ofpass;

	ofpass.open( ngrt4n::AUTH_FILE.c_str() );
	if( ! ofpass.good()) {
		cerr << "Unable to set the password :  perhaps the application's settings file is not well configured." << endl;
		exit(1) ;
	}

	ofpass << crypt(authChain, salt.c_str());
	ofpass.close();
	cout << "Password reseted"<< endl ;
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


