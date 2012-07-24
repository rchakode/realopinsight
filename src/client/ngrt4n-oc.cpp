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
#include <sstream>


const string appName = APPLICATION_NAME ;
const string releaseYear = RELEASE_YEAR;
const string packageName = PACKAGE_NAME ;
const string packageVersion = PACKAGE_VERSION;
const string packageUrl = PACKAGE_URL;

QString  usage = "usage: " + QString(packageName.c_str()) + " [OPTION] [view_config]\n"
		"Options: \n"
		"	-c\n"
		"	   Launch the configuration utility\n"
		"	-v\n"
		"	  Print the version and license information.\n"
		"	-h \n"
		"	   Print this help.\n" ;


ostringstream versionMsg(appName + " Operations Console, version " + packageVersion + ".\n"
		+"This program is part of the NGRT4N Software.\n"
		+"Copyright (c) 2010-" + releaseYear + " NGRT4N Project <contact@ngrt4n.com>." + "\n"
		+"Visit "+ packageUrl + " for further information.") ;

int main(int argc, char **argv)
{
	QApplication* app = new QApplication(argc, argv) ;
	QIcon app_icon (":images/built-in/icon.png") ;
	app->setWindowIcon( app_icon ) ;
	app->setApplicationName(  QString(appName.c_str()) ) ;
	app->setStyleSheet(Preferences::style());

	if(argc > 3) {
		qDebug() << usage ;
		exit (1) ;
	}
	ngrt4n::initApp() ;
	bool config = false ;
	QString file = argv[1] ;
	int opt ;

	if ( (opt = getopt(argc, argv, "chv") ) != -1) {
		switch (opt) {
		case 'c':
			config = true ;
			break ;

		case 'v': {
			cout << versionMsg.str() << endl;
			exit(0) ;
		}

		case 'h': {
			cout << usage.toStdString() ;
			exit(0) ;
		}

		default:
			cout << "Syntax Error :: " << usage.toStdString() ;
			exit (1) ;
			break ;
		}
	}
	cout << "Launching..." << endl << versionMsg.str() << endl;
	Auth authentification;
	int userRole = authentification.exec() ;
	if( userRole != Auth::ADM_USER_ROLE && userRole != Auth::OP_USER_ROLE ) exit( 1 ) ;

	if( config ) {
		Preferences* update_settings = new Preferences(userRole, Preferences::ChangeMonitoringSettings) ;
		Preferences* change_passwd = new Preferences(userRole, Preferences::ChangePassword) ;
		update_settings->exec() ;
		change_passwd->exec() ;
		exit(0) ;
	}

	if(file == "") {
		qDebug() << "invalid file !" ;
		QMessageBox::warning(0,
				"Error | " + QString(packageName.c_str()),
				" :: You need to specify a configuration file !!",
				QMessageBox::Ok);
		exit (1) ;
	}
	SvNavigator *monitor= new SvNavigator(userRole, file) ; monitor->startMonitor() ;

	return app->exec() ;
}
