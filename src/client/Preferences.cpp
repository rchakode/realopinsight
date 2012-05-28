/*
 * Preferences.cpp
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
#include "core/MonitorBroker.hpp"
#include "Preferences.hpp"
#include "Auth.hpp"


const qint32 Preferences::ChangePassword = 0 ;
const qint32 Preferences::ForceChangePassword = 1 ;
const  qint32 Preferences::ChangeMonitoringSettings = 2 ;
const  qint32 Preferences::ShowHelp = 3 ;
const  qint32 Preferences::ShowAbout = 4 ;
const QString Preferences::NAGIOS_URL_KEY = "/Monitor/nagiosHome" ;
const QString Preferences::UPDATE_INTERVAL_KEY = "/Monitor/updateInterval";
const QString Preferences::SERVER_ADDR_KEY = "/Monitor/serverAddr" ;
const QString Preferences::SERVER_PORT_KEY = "/Monitor/serverPort" ;
const QString Preferences::ADM_UNSERNAME_KEY = "/Auth/admUser" ;
const QString Preferences::OP_UNSERNAME_KEY = "/Auth/opUsername" ;
const QString Preferences::ADM_PASSWD_KEY = "/Auth/admPasswd" ;
const QString Preferences::OP_PASSWD_KEY = "/Auth/opPasswd" ;

Preferences::Preferences(const qint32 & _user_role, const qint32 & _action)
: userRole(_user_role),
  settings(new Settings()) ,
  monitorHomeField(new QLineEdit()) ,
  updateIntervalField( new QSpinBox() ) ,
  bBrowse( new QPushButton("&Browse...") ) ,
  oldPasswdField( new QLineEdit() ) ,
  passwdField (new QLineEdit() ) ,
  rePasswdField( new QLineEdit() ) ,
  serverAddrField (new QLineEdit() ) ,
  serverPortField( new QLineEdit() ) ,
  cancelButton( new QPushButton("&Close") ) ,
  applySettingButton( new QPushButton("&Apply settings")) ,
  changePasswdButton( new QPushButton("C&hange password")) ,
  layout( new QGridLayout(this))
{
	qint32 line = -1;
	oldPasswdField->setEchoMode( QLineEdit::Password ) ;
	passwdField->setEchoMode( QLineEdit::Password ) ;
	rePasswdField->setEchoMode( QLineEdit::Password ) ;

	switch (_action)
	{
	case Preferences::ChangeMonitoringSettings:

		line += 1,
		layout->addWidget(new QLabel("Web Interface"), line, 0),
		layout->addWidget(monitorHomeField, line, 1, 1, 4) ;
		line += 1,
				layout->addWidget(new QLabel("Update Interval"), line, 0),
				layout->addWidget(updateIntervalField, line, 1, 1, 2),
				layout->addWidget(new QLabel("seconds"), line, 3) ;
		line += 1,
				layout->addWidget(new QLabel("Server Address"), line, 0),
				layout->addWidget(serverAddrField, line, 1),
				layout->addWidget(new QLabel("Port"), line, 2, Qt::AlignRight),
				layout->addWidget(serverPortField, line, 3) ;
		line += 1,
				layout->addWidget(cancelButton, line, 1, 1, 2, Qt::AlignRight),
				layout->addWidget(applySettingButton, line, 3, 1, 2);
		layout->setColumnStretch(0, 0) ;
		layout->setColumnStretch(1, 6) ;
		layout->setColumnStretch(2, 0) ;
		layout->setColumnStretch(3, 1) ;
		if( _user_role == Auth::OP_USER_ROLE )
		{
			monitorHomeField->setEnabled(false) ;
			bBrowse->setEnabled(false) ;
			updateIntervalField->setEnabled(false) ;
			applySettingButton->setEnabled(false) ;
			serverAddrField->setEnabled(false) ;
			serverPortField->setEnabled(false) ;
		}
		setWindowTitle("Preferences - Monitoring Settings | " + QString(ngrt4n::APP_NAME.c_str())) ;

		break;

	case Preferences::ChangePassword:
	case Preferences::ForceChangePassword:
		line += 1,
		layout->addWidget(new QLabel("Current Password"), line, 0),
		layout->addWidget(oldPasswdField, line, 1, 1, 2) ;

		line += 1,
				layout->addWidget(new QLabel("New password"), line, 0),
				layout->addWidget(passwdField, line, 1, 1, 2) ;

		line += 1,
				layout->addWidget(new QLabel("Retype new password"), line, 0),
				layout->addWidget(rePasswdField, line, 1, 1, 2) ;

		line += 1,
				layout->addWidget(cancelButton, line, 1),
				layout->addWidget(changePasswdButton, line, 2) ;

		if(_action == Preferences::ForceChangePassword) cancelButton->setEnabled(false) ;

		setWindowTitle("Preferences - Change User Password | " + QString(ngrt4n::APP_NAME.c_str())) ;
		break;

	case Preferences::ShowAbout:
		line += 1,
		layout->addWidget(new QLabel(ABOUT_MSG), line, 0) ;
		line += 1,
				layout->addWidget(cancelButton, line, 0) ;
		break ;
	}

	setContent() ;
	addEvents() ;
}

Preferences::~Preferences()
{
	delete updateIntervalField ;
	delete oldPasswdField ;
	delete passwdField ;
	delete rePasswdField ;
	delete changePasswdButton ;
	delete cancelButton ;
	delete applySettingButton ;
	delete serverAddrField ;
	delete serverPortField ;
	delete layout;
}


void Preferences::showEvent (QShowEvent * )
{
	oldPasswdField->setText("") ;
	passwdField->setText("") ;
	rePasswdField->setText("") ;
}

void Preferences::applySettings(void)
{
	QString nagios_home;

	nagios_home = monitorHomeField->text();

	settings->setValue(NAGIOS_URL_KEY, nagios_home) ;
	settings->setValue(UPDATE_INTERVAL_KEY, updateIntervalField->text()) ;
	settings->setValue(SERVER_ADDR_KEY, serverAddrField->text()) ;

	if(serverPortField->text().toInt() <= 0) {
		serverPortField->setText(QString::number(MonitorBroker::DEFAULT_PORT)) ;
	}
	settings->setValue(SERVER_PORT_KEY, serverPortField->text()) ;
	settings->sync() ;

	close() ;

	emit urlChanged(nagios_home);
}


void Preferences::changePasswd(void)
{
	QString clear_passwd, user_passwd, passwd, new_passwd, renew_passwd, key;

	if ( userRole == Auth::ADM_USER_ROLE )
	{
		key = ADM_PASSWD_KEY ;
		user_passwd = settings->value(key).toString() ;
	}
	else
	{
		key = OP_PASSWD_KEY ;
		user_passwd = settings->value(key).toString() ;
	}
	passwd = QCryptographicHash::hash(oldPasswdField->text().toAscii(), QCryptographicHash::Md5) ;
	new_passwd = QCryptographicHash::hash(passwdField->text().toAscii(), QCryptographicHash::Md5);
	renew_passwd = QCryptographicHash::hash(rePasswdField->text().toAscii(), QCryptographicHash::Md5);

	if( user_passwd == passwd )
	{
		if( new_passwd == renew_passwd )
		{
			settings->setKeyValue( key, new_passwd ) ;
			QMessageBox::information(this,
					QString(ngrt4n::APP_NAME.c_str()).toUpper(),
					"Password updated",
					QMessageBox::Ok) ;

			done(0) ;
		}
		else
		{
			QMessageBox::warning(this,
					QString(ngrt4n::APP_NAME.c_str()),
					"Sorry, passwords do not match",
					QMessageBox::Ok) ;
		}
	}
	else
	{
		QMessageBox::warning(this, QString(ngrt4n::APP_NAME.c_str()), "Authentification failed", QMessageBox::Ok) ;
	}
}

void Preferences::setContent(void)
{
	updateIntervalField->setMinimum(5);
	updateIntervalField->setMaximum(600);
	updateInterval = settings->value(UPDATE_INTERVAL_KEY).toInt() ;
	updateIntervalField->setValue(updateInterval) ;

	monitorUrl = settings->value(NAGIOS_URL_KEY).toString() ;
	monitorHomeField->setText(monitorUrl) ;



	serverAddr = settings->value(SERVER_ADDR_KEY).toString() ;
	serverAddrField->setText(serverAddr) ;

	serverPort = settings->value(SERVER_PORT_KEY).toString() ;
	serverPortField->setText(serverPort) ;


}

void Preferences::addEvents(void)
{
	connect(applySettingButton, SIGNAL(clicked()),  this, SLOT(applySettings()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	connect(changePasswdButton, SIGNAL(clicked()),  this, SLOT(changePasswd()));
}
