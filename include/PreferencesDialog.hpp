/*
 * PreferenceDialog.hpp
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

#ifndef SNAVPREFERENCESDIALOG_HPP_
#define SNAVPREFERENCESDIALOG_HPP_

#include "Base.hpp"


class PreferencesDialog : public QDialog
{
	Q_OBJECT

public:
	PreferencesDialog(const qint32 & _user_role = OP_USER_ROLE, const qint32 & _action = PreferencesDialog::ChangePassword);
	virtual ~PreferencesDialog();

	static const qint32 ChangePassword ;
	static const qint32 ForceChangePassword ;
	static const  qint32 ChangeMonitoringSettings ;
	static const  qint32 ShowHelp ;
	static const  qint32 ShowAbout ;


public slots:
	void browse(void);
	void applySettings(void);
	void changePasswd(void);

signals:
	void urlChanged( QString );

protected :
	void showEvent (QShowEvent * ) ;


private:
	qint32 userRole ;
	Settings* settings;
	QLineEdit* nagiosHomeField ;
	QLineEdit* statusFileField ;
	QSpinBox* updateIntervalField ;
	QPushButton *bBrowse ;
	qint32 updateInterval ;
	QString nagiosUrl ;
	QString statusFile ;


	QLineEdit* passwdField ;
	QLineEdit* newPasswdField ;
	QLineEdit* RenewPasswdField ;

	QPushButton* cancelButton ;
	QPushButton* applySettingButton ;
	QPushButton* changePasswdButton ;

	QGridLayout* layout ;

	struct settingParams{
		QString status_file ;
		qint32 update_interval ;
	};

	void setContent(void) ;
	void addEvents(void) ;

};

#endif /* SNAVPREFERENCESDIALOG_HPP_ */
