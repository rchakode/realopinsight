/*
 * Base.hpp
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

#ifndef SNAV_HPP_
#define SNAV_HPP_
#include <QtCore>
#include <QtXml>
#include <QtGui>
#include <QtWebKit>

const QString ABOUT_MSG = "NGRT4N\n\n"
		"Version: 1.0\n"
		"Build id: 20111001 1100\n\n"
		"(c) Copyright 2011 NGRT4N Development Team.\n"
		"Visit http://www.ngrt4n.com";

const QString COMPAGNY_NAME = "ITSoftbyrc.com";
const QString APP_SHORT_NAME = "NGRT4N";
const QString APP_NAME = APP_SHORT_NAME + " :: New Generation Reporting Tool For Nagios";
const QString CONFIG_FILE = "/var/lib/" + APP_SHORT_NAME.toLower() + "/db" ;

const qint32 TREE_NODE_ID_COLUMN = 1 ;
const qint32 NODE_ID_DATA_INDEX = 0;			// Id of data field used to stored nodes ids in the graph

const QString ADM_USER_NAME = APP_SHORT_NAME.toLower() + "_adm" ;
const QString OP_USER_NAME = APP_SHORT_NAME.toLower()+ "_op" ;

const QString NAGIOS_URL_KEY = "/Monitor/nagiosHome" ;
const QString UPDATE_INTERVAL_KEY = "/Monitor/updateInterval";
const QString STATUS_FILE_KEY = "/Monitor/statusFile" ;
const QString ADM_UNSERNAME_KEY = "/Auth/rootUsername" ;
const QString OP_UNSERNAME_KEY = "/Auth/opUsername" ;
const QString ADM_PASSWD_KEY = "/Auth/rootPasswd" ;
const QString OP_PASSWD_KEY = "/Auth/opPasswd" ;

const qint32 ADM_USER_ROLE = 100 ;
const qint32 OP_USER_ROLE = 101 ;

const qint32 SERVICE_NODE = 0 ;
const qint32 ALARM_NODE = 1 ;

const qint32 DEFAULT_UPDATE_INTERVAL = 60 ;
const qint32 MAX_NODE_NAME = 24 ;

const qint32 NAGIOS_OK = 0 ;
const qint32 NAGIOS_WARNING = 1 ;
const qint32 NAGIOS_CRITICAL = 2 ;
const qint32 NAGIOS_UNKNOWN = 3 ;
const qint32 UNSET_STATUS = 4 ;

const QColor OK_COLOR =  Qt::green ;
const QColor WARNING_COLOR = Qt::yellow ;
const QColor CRITICAL_COLOR = Qt::red ;
const QColor UNKNOWN_COLOR = Qt::lightGray ;
const QColor HIGHLIGHT_COLOR = QColor(255, 255, 200) ;

//EDITOR FIELDS
const QString NAME_FIELD = "name" ;
const QString TYPE_FIELD = "type" ;
const QString STATUS_CALC_RULE_FIELD = "Status Calc. Rule" ;
const QString ICON_FIELD = "icon" ;
const QString DESCRIPTION_FIELD = "description" ;
const QString ALARM_MSG_FIELD = "alarmMsg" ;
const QString NOTIFICATION_MSG_FIELD = "notificationMsg" ;
const QString CHECK_LIST_FIELD = "ChecksField" ;
const QString CHECK_FIELD = "CheckField" ;
const QString CHILD_NODES_SEP = "," ;

//RELATED TO COMBOBOXes
const QString HIGH_CRITICITY_CALC_RULE = "Higher Criticity";
const QString WEIGHTED_CALC_RULE = "Equal-weighted Criticity";

const QString BUSINESS_PROCESS_NODE = "Business Process" ;
const QString NATIVE_CHECK_NODE = "Native Check" ;

const qint32 HIGH_CRITICITY_CALC_RULE_INDEX = 0 ;
const qint32 WEIGHTED_CALC_RULE_INDEX = 1 ;

//ICON-RELATED META DATA
const QString PLUS = "plus" ;
const QString MINUS = "minus" ;
const QString DEFAULT_ICON = BUSINESS_PROCESS_NODE ;
const QString NETWORK_ICON = "Network" ;
const QString ROUTER_ICON = "--> Router" ;
const QString SWITCH_ICON = "--> Switch" ;
const QString FIREWALL_ICON = "--> Firewall" ;
const QString STORAGE_ICON = "Storage" ;
const QString FILER_ICON = "--> Storage Area" ;
const QString HARDDISK_ICON = "--> Hard disk" ;
const QString SERVER_ICON = "Server" ;
const QString LINUX_ICON = "--> Linux OS" ;
const QString WINDOWS_ICON = "--> Windows OS" ;
const QString SOLARIS_ICON = "--> Solaris OS" ;
const QString WEBSERVER_ICON = "--> Web Server" ;
const QString DBSERVER_ICON = "--> Database Server" ;
const QString APP_ICON = "Application" ;
const QString WEB_ICON = "--> Web Accessibility" ;
const QString DB_ICON = "--> Database Engine" ;
const QString PROCESS_ICON = "--> Process" ;
const QString LOG_ICON = "--> Logfile" ;
const QString CLOUD_ICON = "Cloud" ;
const QString HYPERVISOR_ICON = "--> Hypervisor" ;
const QString OTH_CHECK_ICON = "Other Check" ;


// GRAPHVIEW ICON EXTENTION, FOR SUITABLE IDENTIFATION
const QString NODE_LABEL_ID_SFX = ":LABEL" ;
const QString  NODE_ICON_ID_SFX = ":ICON" ;
const QString  NODE_EXP_ICON_ID_SFX = ":EXPICON" ;


//EVENT CONSOLE META-MESSAGES
const QString HOSTNAME_META_MSG_PATERN = "\\{hostname\\}" ;
const QString SERVICE_META_MSG_PATERN = "\\{check_name\\}" ;
const QString THERESHOLD_META_MSG_PATERN = "\\{threshold\\}" ;
const QString PLUGIN_OUTPUT_META_MSG_PATERN = "\\{plugin_output\\}" ;


//GLOBAL DATA STRUCTURES
typedef QMap<QString, QString> IconMapT;
typedef QList<QListWidgetItem  *> CheckItemList;
typedef QHash<QString, QTreeWidgetItem*> TreeNodeItemListT ;

typedef struct _NagiosCheckT{
	QString id;
	QString check_command ;
	QString last_state_change ;
	QString alarm_msg ;
	qint32 status ;
}NagiosCheckT;

typedef struct _NodeT {
	QString id;
	QString name ;
	qint32 type ;
	qint32 status_calc_rule ;
	QString icon ;
	QString description ;
	QString parent ;
	QString propagation_rule ;
	QString alarm_msg ;
	QString notification_msg ;
	qint32 status ;
	QString child_nodes ;
	NagiosCheckT check ;
} NodeT;

typedef QHash<QString, NagiosCheckT> NagiosChecksT ;

typedef QHash<QString, NodeT> NodeListT ;

typedef QMap<qint32, qint32> CheckStatusCountT ;

typedef struct _GNode {
	QGraphicsTextItem* label ;
	QGraphicsPixmapItem* icon ;
	QGraphicsPixmapItem* exp_icon ;
	qint32 type ;
	bool expand ;
}GNodeT;

typedef struct _GEdge {
	QGraphicsPathItem* edge ;
}GEdgeT;

typedef QHash <QString, GNodeT> GNodeListT;

typedef QHash <QString, GEdgeT> GEdgeListT;

typedef QMap<QString, QMenu*> MenuListT;

typedef QMap<QString, QAction*> SubMenuListT;


//UTILITY CLASSES
class Struct
{

public:
	QString root_id ;
	TreeNodeItemListT tree_item_list ;
	NodeListT node_list ;
	QStringList check_list ;
	CheckStatusCountT check_status_count ;

	void clear(void) ;
};

class Settings : public QSettings
{
public:
	Settings() ;
	void setKeyValue(const QString & _key, const QString & _value) ;
};


class Utils
{
public :
	static QString statusToString(qint32 _status) ;
};

#endif /* SNAV_HPP_ */
