/*
 * ngrt4n.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 24-05-2012                                                 #
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
#include <getopt.h>


#include "core/ZmqHelper.hpp"
#include "client/JsonRpcHelper.hpp"

QString cmdName = "" ;
QString  usage = "usage: " + cmdName + " [OPTION] [view_config]\n"
		"Options: \n"
		"	-c\n"
		"	   Launch the configuration utility\n"
		"	-e [view_config]\n"
		"	   Run the VE utility and load the file view_config if specified\n"
		"	-d view_config\n"
		"	   Run the OC utility and load the file view_config\n"
		"	-v\n"
		"	  Print the version and license information.\n"
		"	-h \n"
		"	   Print this help.\n" ;


ostringstream versionMsg(appName.toStdString() + " "+packageName.toStdString()+", Version " + packageVersion.toStdString() + ".\n\n"
		+"Copyright (c) 2010-"+releaseYear.toStdString()+" NGRT4N Project <contact@ngrt4n.com>.\n"
		+"All rights reserved. Visit "+packageUrl.toStdString()+" for more information.");

int main(int argc, char **argv)
{
	QApplication* app = new QApplication(argc, argv) ;
	app->setWindowIcon(QIcon(":images/built-in/icon.png")) ;
	app->setApplicationName(appName.toUpper() ) ;
	app->setStyleSheet(Preferences::style());
	cmdName=argv[0];
	if(argc > 3) {
		qDebug() << usage ;
		exit (1) ;
	}

 //   JsonRpcHelper jsonHelper ;
 //   ZmqHelper::init() ;
	QString module = "config" ;
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
			cout << versionMsg.str() ;
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
	cout <<"Launching "<<versionMsg.str()<<endl;
	Auth authentication;
	int userRole = authentication.exec() ;
	if( userRole != Auth::ADM_USER_ROLE && userRole != Auth::OP_USER_ROLE ) exit( 1 ) ;

	if(module == "dashboard") {
		QSplashScreen* info = Preferences::infoScreen("Welcome to NGRT4N Operations Concole..."+QString::fromStdString(versionMsg.str()));
		sleep(1);
		if(file == "") {
			info->clearMessage();
			info->showMessage("You need to select a configuration file!", Qt::AlignCenter|Qt::AlignCenter);
			sleep(1); info->finish(0);
			file = QFileDialog::getOpenFileName(0,
					appName.toUpper() + " :: Select a configuration file",
					".",
					"Xml files (*.xml);;All files (*)");

			if(! file.length()){
				QMessageBox::critical(0,
						appName.toUpper() + " :: Info",
						"No configuration file has been selected and the program will exit.",
						QMessageBox::Ok);
				exit (1) ;
			}

		}
		info->finish(0);
		SvNavigator *monitor= new SvNavigator(userRole, file) ; monitor->startMonitor() ;
	} else if(module == "editor") {
		SvCreator* svc = new SvCreator(userRole) ;
		svc->load(file) ;
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
