/*
 * Auth.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 13-05-2012												   #
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

#include "Auth.hpp"
#include "Preferences.hpp"
#include "ns.hpp"

const QString Auth::ADM_USER_NAME = QString(ngrt4n::APP_NAME.c_str()).toLower() + "_adm" ;
const QString Auth::OP_USER_NAME = QString(ngrt4n::APP_NAME.c_str()).toLower()+ "_op" ;
const qint32 Auth::ADM_USER_ROLE = 100 ;
const qint32 Auth::OP_USER_ROLE = 101 ;

Auth::Auth()
{
	login = new QLineEdit(OP_USER_NAME);
	password = new QLineEdit(), password->setEchoMode( QLineEdit::Password );
	buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

	qint32 line;
	layout = new QGridLayout(this);
	line = 0,
			layout->addWidget(new QLabel("Login"), line, 0),
			layout->addWidget(login, line, 1);

	line += 1,
			layout->addWidget(new QLabel("Password"), line, 0),
			layout->addWidget(password, line, 1);

	line += 1,
			layout->addWidget(buttonBox, line, 1);

	addEvents();
	setWindowTitle( "Authentification - " + QString(ngrt4n::APP_NAME.c_str()) );

	settings = new Settings();
}

Auth::~Auth()
{
	delete login ;
	delete password ;
	delete layout ;
}


void Auth::cancel(void)
{
	exit( 1 );
}


void Auth::authentificate(void)
{
	QString userName = login->text() ;
	QString userPasswd = QCryptographicHash::hash(password->text().toAscii(), QCryptographicHash::Md5) ;
	QString rootPasswd =  settings->value(Preferences::ADM_PASSWD_KEY).toString() ;
	QString opPasswd =  settings->value(Preferences::OP_PASSWD_KEY).toString() ;

	if(	! rootPasswd.isEmpty()
			&& userName == ADM_USER_NAME
			&& userPasswd == rootPasswd ) {

		done( ADM_USER_ROLE );
	} else if( ! opPasswd.isEmpty()
			&& userName == OP_USER_NAME
			&& userPasswd == opPasswd ) {
		done(OP_USER_ROLE);
	} else {
		QMessageBox::warning(this,
				QString(ngrt4n::APP_NAME.c_str()),
				tr("Authentifcation failed. Wrong username or password"),
				QMessageBox::Ok) ;
	}
}


void Auth::addEvents(void)
{
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(authentificate()));
}
