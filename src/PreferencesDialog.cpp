/*
 * PreferencesDialog.cpp
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

#include "../include/PreferencesDialog.hpp"

const qint32 PreferencesDialog::ChangePassword = 0 ;
const qint32 PreferencesDialog::ForceChangePassword = 1 ;
const  qint32 PreferencesDialog::ChangeMonitoringSettings = 2 ;
const  qint32 PreferencesDialog::ShowHelp = 3 ;
const  qint32 PreferencesDialog::ShowAbout = 4 ;

PreferencesDialog::PreferencesDialog(const qint32 & _user_role, const qint32 & _action)
: userRole(_user_role),
  settings(new Settings()) ,
  nagiosHomeField(new QLineEdit()) ,
  statusFileField (new QLineEdit()) ,
  updateIntervalField( new QSpinBox() ) ,
  bBrowse( new QPushButton("&Browse...") ) ,
  passwdField( new QLineEdit() ) ,
  newPasswdField (new QLineEdit() ) ,
  RenewPasswdField( new QLineEdit() ) ,
  cancelButton( new QPushButton("&Close") ) ,
  applySettingButton( new QPushButton("&Apply settings")) ,
  changePasswdButton( new QPushButton("C&hange password")) ,
  layout( new QGridLayout(this))
  {
	qint32 line = -1;
	passwdField->setEchoMode( QLineEdit::Password ) ;
	newPasswdField->setEchoMode( QLineEdit::Password ) ;
	RenewPasswdField->setEchoMode( QLineEdit::Password ) ;

	switch (_action)
	{
	case PreferencesDialog::ChangeMonitoringSettings:

		line += 1,
		layout->addWidget(new QLabel("Default Web Interface"), line, 0),
		layout->addWidget(nagiosHomeField, line, 1, 1, 3) ;
		line += 1,
				layout->addWidget(new QLabel("Status Database"), line, 0),
				layout->addWidget(statusFileField, line, 1, 1, 2),
				layout->addWidget(bBrowse, line, 3) ;
		line += 1,
				layout->addWidget(new QLabel("Console Update Interval"), line, 0),
				layout->addWidget(updateIntervalField, line, 1, 1, 2),
				layout->addWidget(new QLabel("seconds"), line, 3) ;
		line += 1,
				layout->addWidget(cancelButton, line, 2),
				layout->addWidget(applySettingButton, line, 3);

		if( _user_role == OP_USER_ROLE )
		{
			nagiosHomeField->setEnabled(false) ;
			statusFileField->setEnabled(false) ;
			bBrowse->setEnabled(false) ;
			updateIntervalField->setEnabled(false) ;
			applySettingButton->setEnabled(false) ;
		}
		setWindowTitle("Preferences - Change Monitoring Settings | " + APP_SHORT_NAME) ;

		break;

	case PreferencesDialog::ChangePassword:
	case PreferencesDialog::ForceChangePassword:
		line += 1,
		layout->addWidget(new QLabel("Current Password"), line, 0),
		layout->addWidget(passwdField, line, 1, 1, 2) ;

		line += 1,
				layout->addWidget(new QLabel("New password"), line, 0),
				layout->addWidget(newPasswdField, line, 1, 1, 2) ;

		line += 1,
				layout->addWidget(new QLabel("Retype new password"), line, 0),
				layout->addWidget(RenewPasswdField, line, 1, 1, 2) ;

		line += 1,
				layout->addWidget(cancelButton, line, 1),
				layout->addWidget(changePasswdButton, line, 2) ;

		if(_action == PreferencesDialog::ForceChangePassword) cancelButton->setEnabled(false) ;

		setWindowTitle("Preferences - Change User Password | " + APP_SHORT_NAME) ;
		break;

	case PreferencesDialog::ShowAbout:
		line += 1,
		layout->addWidget(new QLabel(ABOUT_MSG), line, 0) ;
		line += 1,
				layout->addWidget(cancelButton, line, 0) ;
		break ;
	}

	setContent() ;
	addEvents() ;
  }

PreferencesDialog::~PreferencesDialog()
{
	delete updateIntervalField ;
	delete statusFileField ;
	delete passwdField ;
	delete newPasswdField ;
	delete RenewPasswdField ;
	delete changePasswdButton ;
	delete cancelButton ;
	delete applySettingButton ;
	delete layout;
}


void PreferencesDialog::showEvent (QShowEvent * )
{
	passwdField->setText("") ;
	newPasswdField->setText("") ;
	RenewPasswdField->setText("") ;
}

void PreferencesDialog::browse(void)
{
	QString status_file = QFileDialog::getOpenFileName();

	if ( ! status_file.isEmpty() )  statusFileField->setText(status_file);
}

void PreferencesDialog::applySettings(void)
{
	QString nagios_home;

	nagios_home = nagiosHomeField->text();

	settings->setValue(NAGIOS_URL_KEY, nagios_home) ;
	settings->setValue(STATUS_FILE_KEY, statusFileField->text()) ;
	settings->setValue(UPDATE_INTERVAL_KEY, updateIntervalField->text()) ;
	settings->sync() ;

	close() ;

	emit urlChanged(nagios_home);
}


void PreferencesDialog::changePasswd(void)
{
	QString clear_passwd, user_passwd, passwd, new_passwd, renew_passwd, key;

	if ( userRole == ADM_USER_ROLE )
	{
		key = ADM_PASSWD_KEY ;
		user_passwd = settings->value(key).toString() ;
	}
	else
	{
		key = OP_PASSWD_KEY ;
		user_passwd = settings->value(key).toString() ;
	}
	passwd = QCryptographicHash::hash(passwdField->text().toAscii(), QCryptographicHash::Md5) ;
	new_passwd = QCryptographicHash::hash(newPasswdField->text().toAscii(), QCryptographicHash::Md5);
	renew_passwd = QCryptographicHash::hash(RenewPasswdField->text().toAscii(), QCryptographicHash::Md5);

	if( user_passwd == passwd )
	{
		if( new_passwd == renew_passwd )
		{
			settings->setKeyValue( key, new_passwd ) ;
			QMessageBox::information(this, APP_SHORT_NAME, "Password updated", QMessageBox::Ok) ;

			done(0) ;
		}
		else
		{
			QMessageBox::warning(this, APP_SHORT_NAME, "Sorry, passwords do not match", QMessageBox::Ok) ;
		}
	}
	else
	{
		QMessageBox::warning(this, APP_SHORT_NAME, "Authentification failed", QMessageBox::Ok) ;
	}
}

void PreferencesDialog::setContent(void)
{
	statusFile = settings->value(STATUS_FILE_KEY).toString() ;
	updateInterval = settings->value(UPDATE_INTERVAL_KEY).toInt() ;
	nagiosUrl = settings->value(NAGIOS_URL_KEY).toString() ;

	nagiosHomeField->setText(nagiosUrl) ;
	updateIntervalField->setValue(updateInterval) ;
	updateIntervalField->setMinimum(5);
	updateIntervalField->setMaximum(600);
	statusFileField->setText(statusFile) ;
}

void PreferencesDialog::addEvents(void)
{
	connect(bBrowse, SIGNAL(clicked()), this, SLOT(browse()));
	connect(applySettingButton, SIGNAL(clicked()),  this, SLOT(applySettings()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	connect(changePasswdButton, SIGNAL(clicked()),  this, SLOT(changePasswd()));
}
