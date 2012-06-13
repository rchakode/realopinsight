/*
 * ngrt4n.cpp
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

#include "core/ns.hpp"
#include "client/Auth.hpp"
#include "client/SvNavigator.hpp"
#include "client/SvConfigCreator.hpp"


const string appName = APPLICATION_NAME ;
const string releaseYear = RELEASE_YEAR;
const string packageName = PACKAGE_NAME ;
const string packageVersion = PACKAGE_VERSION;
const string packageUrl = PACKAGE_URL;


int main(int argc, char **argv)
{
	QApplication* app = new QApplication(argc, argv) ;
	QIcon app_icon (":images/appicon.png") ;
	app->setWindowIcon( app_icon ) ;
	app->setApplicationName(  QString(appName.c_str()) ) ;
	app->setStyleSheet(Preferences::style());

	QString  usage = "usage: " + QString(packageName.c_str()) + " [OPTION] [view_config]\n"
			"Options: \n"
			"	-c\n"
			"	   Launches the configuration utility\n"
			"	-e [view_config]\n"
			"	   Runs the VE utility and load the file view_config if specified\n"
			"	-d view_config\n"
			"	   Runs the OC utility and load the file view_config\n"
			"	-v\n"
			"	  Prints the version and license information.\n"
			"	-h \n"
			"	   Prints this help" ;


	if(argc > 3) {
		qDebug() << usage ;
		exit (1) ;
	}

	ngrt4n::initApp() ;

	QString module = "dashboard" ;
	QString file = argv[1] ;
	int opt ;

	if ( (opt = getopt(argc, argv, "cdehv") ) != -1) {
		switch (opt) {
		case 'c':
			module = "config" ;
			break ;

		case 'd':
			module = "dashboard" ;
			file = argv[2] ;
			break ;

		case 'e':
			module = "editor" ;
			file = argv[2] ;
			break ;

		case 'v': {
			cout << appName << " (UI Module) " << packageVersion << endl ;
			cout << "Copyright (c) 2010-"<< releaseYear << " Rodrigue Chakode <rodrigue.chakode@ngrt4n.com>." << endl;
			cout << "Visit "<< packageUrl << " for further information." << endl;
			exit(0) ;
		}

		default: // -h for get help
			qDebug() << usage ;
			exit (1) ;
			break ;
		}
	}

	Auth authentification;
	int userRole = authentification.exec() ;
	if( userRole != Auth::ADM_USER_ROLE && userRole != Auth::OP_USER_ROLE ) exit( 1 ) ;

	if(module == "dashboard") {
		if(file == "") {
			qDebug() << "invalid file !" ;
			QMessageBox::warning(0,
					"Error | " + QString(packageName.c_str()),
					"No file specified to load. The system will exit !",
					QMessageBox::Ok);
			exit (1) ;
		}
		new SvNavigator(userRole, file) ;
	} else if(module == "editor") {
		SvCreator* svc = new SvCreator(userRole) ;
		svc->load( file ) ;
	}
	else if(module == "config") {
		Preferences* update_settings = new Preferences(userRole, Preferences::ChangeMonitoringSettings) ;
		Preferences* change_passwd = new Preferences(userRole, Preferences::ChangePassword) ;
		update_settings->exec() ;
		change_passwd->exec() ;
		exit(0) ;
	}

	return app->exec() ;
}
