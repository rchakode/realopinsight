/*
 * Base.hpp
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

#ifndef SNAV_HPP_
#define SNAV_HPP_
#include <QtCore>
#include <QtXml>
#include <QtGui>
#include <QtWebKit>
#include <QSettings>
#include <bitset>
#include "core/MonitorBroker.hpp"

typedef QMap<QString, QString> IconMapT;
typedef QList<QListWidgetItem  *> CheckItemList;
typedef QHash<QString, QTreeWidgetItem*> TreeNodeItemListT ;
typedef bitset<4> StatusInfoT ;

enum TreeColumnsDesc{
	NODE_ID_DATA_INDEX = 0,			// Id of data field used to stored nodes ids in the graph
	TREE_NODE_ID_COLUMN = 1
};


class StatusPropRules {
public :
	enum StatusPropRulesT{
		Unchanged = 0,
		Decreased = 1,
		Increased = 2
	};

	static QString toString(StatusPropRulesT rule) {
		return QString::number(rule) ;
	}

	static QString label(StatusPropRulesT rule) {
		switch(rule) {
		case Unchanged: return "Unchanged" ;
		case Decreased: return "Decreased" ;
		case Increased: return "Increased" ;
		}
	}
};


class StatusCalcRules {
public :
	enum StatusCalcRulesT{
		HighCriticity = 0,
		WeightedCriticity = 1
	};

	static QString toString(StatusCalcRulesT rule) {
		return QString::number(rule) ;
	}

	static QString label(StatusCalcRulesT rule) {
		switch(rule) {
		case HighCriticity: return "High Criticity" ;
		case WeightedCriticity: return "Weighted Criticity" ;
		}
	}
};


class NodeType {
public :
	enum {
		SERVICE_NODE = 0,
		ALARM_NODE = 1,
	} ;
	static QString toString(int _type ) {

		if (_type == ALARM_NODE ) return "Native Check" ;

		return  "Business Process" ;
	}

};


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
	qint32 prop_status ;
	StatusInfoT status_info ;
	QString child_nodes ;
	MonitorBroker::NagiosCheckT check ;
} NodeT;

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

typedef QHash<QString,GNodeT> GNodeListT;
typedef QHash<QString,GEdgeT> GEdgeListT;
typedef QMap<QString,QMenu*> MenuListT;
typedef QMap<QString,QAction*> SubMenuListT;
typedef QMap<QString,QString> ComboBoxItemsT;


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
