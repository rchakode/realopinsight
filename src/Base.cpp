/*
 * Base.cpp
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


#include "../include/Base.hpp"


void Struct::clear(void)
{
	tree_item_list.clear() ;
	check_list.clear() ;
	node_list.clear() ;
	root_id.clear() ;
}

Settings::Settings():QSettings(CONFIG_FILE, QSettings::NativeFormat )
{
	Q_INIT_RESOURCE(ngrt4n);

	QString update_interval, nagios_home, status_file, crypted_passwd ;
	QString adm_username, adm_passwd, op_username, op_passwd ;

	status_file = value(STATUS_FILE_KEY).toString();
	update_interval = value(UPDATE_INTERVAL_KEY).toString();
	nagios_home = value(NAGIOS_URL_KEY).toString() ;
	adm_username = value(ADM_UNSERNAME_KEY).toString();
	adm_passwd = value(ADM_PASSWD_KEY).toString();
	op_username = value(OP_UNSERNAME_KEY).toString();
	op_passwd = value(OP_PASSWD_KEY).toString();

	if ( status_file.isEmpty() )
	{
		setValue(STATUS_FILE_KEY, "/volume1/install/var/status.dat");
	}

	if ( update_interval.isEmpty() )
	{
		setValue(UPDATE_INTERVAL_KEY, DEFAULT_UPDATE_INTERVAL);
	}

	if ( nagios_home.isEmpty() )
	{
		setValue(NAGIOS_URL_KEY, "http://localhost/nagios");
	}

	if ( adm_username.isEmpty() ){
		setValue(ADM_UNSERNAME_KEY, ADM_USER_NAME);
	}

	if ( adm_passwd.isEmpty() )
	{
		crypted_passwd = QCryptographicHash::hash(ADM_USER_NAME.toAscii(), QCryptographicHash::Md5) ;
		setValue(ADM_PASSWD_KEY, crypted_passwd );
	}

	if ( op_username.isEmpty() ){
		setValue(OP_UNSERNAME_KEY, OP_USER_NAME);
	}

	if ( op_passwd.isEmpty() )
	{
		crypted_passwd = QCryptographicHash::hash(OP_USER_NAME.toAscii(), QCryptographicHash::Md5) ;
		setValue(OP_PASSWD_KEY, crypted_passwd);
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
	case NAGIOS_OK:
		return "Normal";
		break;

	case NAGIOS_WARNING:
		return  "Warning";
		break;

	case NAGIOS_CRITICAL:
		return  "Critical";
		break;

	default:
		return "Unknown";
		break;
	}

	return "Unknown";
}


