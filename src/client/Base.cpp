/*
 * Base.cpp
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


#include "Base.hpp"
#include "Auth.hpp"
#include "Preferences.hpp"
#include "core/MonitorBroker.hpp"
#include "core/ns.hpp"
void Struct::clear(void)
{
	tree_item_list.clear() ;
	check_list.clear() ;
	node_list.clear() ;
}

Settings::Settings(): QSettings(QString(ngrt4n::SETTINGS_FILE.c_str()), QSettings::NativeFormat )
{
	Q_INIT_RESOURCE(ngrt4n);

	QString updateInterval = value(Preferences::UPDATE_INTERVAL_KEY).toString();
	QString monitorHome = value(Preferences::NAGIOS_URL_KEY).toString() ;
	QString admUser = value(Preferences::ADM_UNSERNAME_KEY).toString();
	QString admPasswd = value(Preferences::ADM_PASSWD_KEY).toString();
	QString opUser = value(Preferences::OP_UNSERNAME_KEY).toString();
	QString opPasswd = value(Preferences::OP_PASSWD_KEY).toString();
	QString serverAddr = value(Preferences::SERVER_ADDR_KEY).toString();
	QString serverPort = value(Preferences::SERVER_PORT_KEY).toString();

	if ( updateInterval.isEmpty() ) {
		setValue(Preferences::UPDATE_INTERVAL_KEY, QString::number(MonitorBroker::DEFAULT_UPDATE_INTERVAL));
	}

	if ( serverAddr.isEmpty() ) {
		setValue(Preferences::SERVER_ADDR_KEY, "localhost");
	}

	if ( serverPort.isEmpty() )	{
		setValue(Preferences::SERVER_PORT_KEY, QString::number(MonitorBroker::DEFAULT_PORT));
	}

	if ( monitorHome.isEmpty() ) {
		setValue(Preferences::NAGIOS_URL_KEY, "http://localhost/nagios");
	}

	if ( admUser.isEmpty() ) {
		setValue(Preferences::ADM_UNSERNAME_KEY, Auth::ADM_USER_NAME);
	}

	if ( admPasswd.isEmpty() ) {
		QString passwd = QCryptographicHash::hash(Auth::ADM_USER_NAME.toAscii(), QCryptographicHash::Md5) ;
		setValue(Preferences::ADM_PASSWD_KEY, passwd );
	}

	if ( opUser.isEmpty() ) {
		setValue(Preferences::OP_UNSERNAME_KEY, Auth::OP_USER_NAME);
	}

	if ( opPasswd.isEmpty() ) {
		QString passwd = QCryptographicHash::hash(Auth::OP_USER_NAME.toAscii(), QCryptographicHash::Md5) ;
		setValue(Preferences::OP_PASSWD_KEY, passwd);
	}

	sync();
 }

void Settings::setKeyValue(const QString & _key, const QString & _value)
{
	setValue(_key, _value ) ;
	sync() ;
}


QString Utils::statusToString(qint32 _status)
{
	switch(_status)
	{
	case MonitorBroker::NAGIOS_OK:
		return "Normal";
		break;

	case MonitorBroker::NAGIOS_WARNING:
		return  "Warning";
		break;

	case MonitorBroker::NAGIOS_CRITICAL:
		return  "Critical";
		break;

	default:
		return "Unknown";
		break;
	}

	return "Unknown";
}


