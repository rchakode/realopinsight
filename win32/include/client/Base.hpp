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

#ifndef BASE_HPP
#define BASE_HPP
#include <QtCore>
#include <QtXml>
#include <QtGui>
#include <QtWebKit>
#include <QSettings>
#include <bitset>
#include "core/MonitorBroker.hpp"
#include <QTranslator>

const QString PROJECT = "NGRT4N";
const QString USER_BASE_NAME = "ngrt4n";
const QString appName="RealOpInsight" ;
const QString packageName="UI Module";
const QString packageVersion="2.1.0beta1";
const QString packageUrl="http://RealOpInsight.com";
const QString releaseYear="2012";
const qint32 minServer = 110;

typedef QMap<QString, QString> IconMapT;
typedef QList<QListWidgetItem  *> CheckItemList;
typedef QHash<QString, QTreeWidgetItem*> TreeNodeItemListT;
typedef bitset<4> StatusInfoT;

class StatusPropRules {
public :
    enum StatusPropRulesT{
        Unchanged = 0,
        Decreased = 1,
        Increased = 2
    };

    static QString toString(StatusPropRulesT rule) {
        return QString::number(rule);
    }

    static QString label(qint32 rule) {
        return label(static_cast<StatusPropRulesT>(rule));
    }
    static QString label(StatusPropRulesT rule) {
        switch(rule) {
        case Unchanged: return "Unchanged";
        case Decreased: return "Decreased";
        case Increased: return "Increased";
        }

        return "Unchanged";
    }
};


class StatusCalcRules {
public :
    enum StatusCalcRulesT{
        HighCriticity = 0,
        WeightedCriticity = 1
    };

    static QString toString(StatusCalcRulesT rule) {
        return QString::number(rule);
    }

    static QString label(qint32 rule) {
        return label(static_cast<StatusCalcRulesT>(rule));
    }

    static QString label(StatusCalcRulesT rule) {

        if (rule == WeightedCriticity) return "Weighted Criticity";

        return "High Criticity";
    }
};


class NodeType {
public :
    enum {
        SERVICE_NODE = 0,
        ALARM_NODE = 1
    };
    static QString toString(int _type ) {

        if (_type == ALARM_NODE ) return "Native Check";

        return  "Business Process";
    }

};


class Status{

public:

    Status(MonitorBroker::StatusT _value=MonitorBroker::OK): value(_value) {}
    MonitorBroker::StatusT getValue() const{return value;}

    Status operator *(Status& st) const {
        switch(value) {
        case MonitorBroker::CRITICAL : return Status(MonitorBroker::CRITICAL);
        case MonitorBroker::OK : return st;
        case MonitorBroker::WARNING: {
            if(st.value == MonitorBroker::CRITICAL || st.value == MonitorBroker::UNKNOWN) return st;
            return Status(MonitorBroker::WARNING);
        }
        default : { //UNKNOWN
            if(st.value == MonitorBroker::CRITICAL) return st;
            return Status(MonitorBroker::UNKNOWN);
        }
        }
    }

    Status operator /(Status& st) const {
        if((value == MonitorBroker::CRITICAL) || (st.value == MonitorBroker::CRITICAL))
            return Status(MonitorBroker::CRITICAL);
        if((value == MonitorBroker::UNKNOWN) || (st.value == MonitorBroker::UNKNOWN))
            return Status(MonitorBroker::UNKNOWN);
        if(value == st.value) return  st;

        return Status(MonitorBroker::WARNING);
    }

    Status operator ++(int) {
        switch(value) {
        case MonitorBroker::WARNING: return Status(MonitorBroker::CRITICAL);
        case MonitorBroker::UNKNOWN : return Status(MonitorBroker::WARNING);
        default : break;
        }

        return Status(value);
    }

    Status operator --(int) {
        switch(value) {
        case MonitorBroker::CRITICAL: return Status(MonitorBroker::WARNING);
        default : break;
        }

        return Status(value);
    }
private:

    MonitorBroker::StatusT value;
};


typedef struct _NodeT {
    QString id;
    QString name;
    qint32 type;
    qint32 status_crule;
    qint32 status_prule;
    QString icon;
    QString description;
    QString parent;
    QString propagation_rule;
    QString alarm_msg;
    QString notification_msg;
    qint32 status;
    qint32 prop_status;
    QString child_nodes;
    MonitorBroker::NagiosCheckT check;
} NodeT;

typedef QHash<QString, NodeT> NodeListT;
typedef QMap<qint32, qint32> CheckStatusCountT;
typedef QHash<QString, MonitorBroker::CheckT> CheckListT;

typedef struct _GNode {
    QGraphicsTextItem* label;
    QGraphicsPixmapItem* icon;
    QGraphicsPixmapItem* exp_icon;
    qint32 type;
    bool expand;
}GNodeT;

typedef struct _GEdge {
    QGraphicsPathItem* edge;
}GEdgeT;

typedef QHash<QString,GNodeT> GNodeListT;
typedef QHash<QString,GEdgeT> GEdgeListT;
typedef QHash<QString,QStringList> HostListT;
typedef QMap<QString,QMenu*> MenuListT;
typedef QMap<QString,QAction*> SubMenuListT;
typedef QMap<QString,QString> ComboBoxItemsT;


struct Struct {
    qint8 monitor;
    NodeListT nodes;
    NodeListT cnodes;
    CheckListT checks_;
    CheckStatusCountT check_status_count;
    HostListT hosts;
    TreeNodeItemListT tree_items;
};

class Settings : public QSettings
{
public:
    Settings();
    void setKeyValue(const QString & _key, const QString & _value);

private:
    QTranslator* translator ;
};

#endif /* BASE_HPP */
