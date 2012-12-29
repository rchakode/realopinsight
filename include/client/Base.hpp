/*
 * Base.hpp
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
const QString USER_BASE_NAME = BUILTIN_USER_PREFIX;
const QString projectName = PROJECT;
const QString appName = APPLICATION_NAME;
const QString packageName = PACKAGE_NAME;
const QString packageVersion = PACKAGE_VERSION;
const QString packageUrl = PACKAGE_URL;
const QString releaseYear = RELEASE_YEAR;
const QString releaseName = RELEASE_NAME;

typedef QMap<QString, QString> IconMapT;
typedef QList<QListWidgetItem  *> CheckItemList;
typedef QHash<QString, QTreeWidgetItem*> TreeNodeItemListT;
typedef bitset<4> StatusInfoT;

class StatusPropRules {
public:
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
public:
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
public:
  enum {
    SERVICE_NODE = 0,
    ALARM_NODE = 1
  };
  static QString toString(int _type ) {

    if (_type == ALARM_NODE )
      return QObject::tr("Native Check");

    return QObject::tr("Business Process");
  }

};


class Criticity {

public:
  Criticity(MonitorBroker::CriticityT _value=MonitorBroker::CRITICITY_NORMAL): value(_value) {}
  void setValue(MonitorBroker::CriticityT _value) {value = _value;}
  MonitorBroker::CriticityT getValue() const {return value;}

  Criticity operator *(Criticity& _criticity) const {
    switch(value) {
      case MonitorBroker::CRITICITY_HIGH:
        return Criticity(value);
        break;
      case MonitorBroker::CRITICITY_NORMAL:
        return _criticity;
        break;
      case MonitorBroker::CRITICITY_MINOR:
        if(_criticity.value == MonitorBroker::CRITICITY_HIGH ||
           _criticity.value == MonitorBroker::CRITICITY_MAJOR ||
           _criticity.value == MonitorBroker::CRITICITY_UNKNOWN)
          return _criticity;

        return Criticity(value);
        break;
      case MonitorBroker::CRITICITY_MAJOR:
        if(_criticity.value == MonitorBroker::CRITICITY_HIGH ||
           _criticity.value == MonitorBroker::CRITICITY_UNKNOWN)
          return _criticity;

        return Criticity(value);
        break;
      default:
        // MonitorBroker::CRITICITY_UNKNOWN
        if(_criticity.value == MonitorBroker::CRITICITY_HIGH)
          return _criticity;
        break;
      }  //end switch

    return Criticity(MonitorBroker::CRITICITY_UNKNOWN);
  }


  Criticity operator / (Criticity& st) const {
    if(value == st.value)
      return  st;

    if(value == MonitorBroker::CRITICITY_HIGH ||
       st.value == MonitorBroker::CRITICITY_HIGH)
      return Criticity(MonitorBroker::CRITICITY_HIGH);

    if(value == MonitorBroker::CRITICITY_UNKNOWN ||
       st.value == MonitorBroker::CRITICITY_UNKNOWN)
      return Criticity(MonitorBroker::CRITICITY_UNKNOWN);

    if(value == MonitorBroker::CRITICITY_MAJOR ||
       st.value == MonitorBroker::CRITICITY_MAJOR)
      return Criticity(MonitorBroker::CRITICITY_MAJOR);

    if(value == MonitorBroker::CRITICITY_MINOR ||
       st.value == MonitorBroker::CRITICITY_MINOR)
      return Criticity(MonitorBroker::CRITICITY_MINOR);

    return Criticity(MonitorBroker::CRITICITY_NORMAL);
  }

  Criticity operator ++(int) {
    switch(value) {
      case MonitorBroker::CRITICITY_MINOR:
        return Criticity(MonitorBroker::CRITICITY_MAJOR);
        break;

      case MonitorBroker::CRITICITY_MAJOR:
        return Criticity(MonitorBroker::CRITICITY_HIGH);
        break;

      default:
        //MonitorBroker::CRITICITY_NORMAL:
        //MonitorBroker::CRITICITY_UNKNOWN:
        //MonitorBroker::CRITICITY_HIGH:
        break;
      }

    return Criticity(value);
  }

  Criticity operator --(int) {

    switch(value) {
      case MonitorBroker::CRITICITY_HIGH:
        return Criticity(MonitorBroker::CRITICITY_MAJOR);
        break;

      case MonitorBroker::CRITICITY_MAJOR:
        return Criticity(MonitorBroker::CRITICITY_MINOR);
        break;

      default:
        //MonitorBroker::CRITICITY_NORMAL:
        //MonitorBroker::CRITICITY_MINOR:
        //MonitorBroker::CRITICITY_UNKNOWN:
        break;
      }

    return Criticity(value);
  }

private:

  MonitorBroker::CriticityT value;
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
  qint32 criticity;
  qint32 prop_status;
  QString child_nodes;
  MonitorBroker::CheckT check;
} NodeT;

typedef QHash<QString, NodeT> NodeListT;
typedef QMap<qint32, qint32> CheckStatusCountT;
typedef QHash<QString, MonitorBroker::CheckT> CheckListT;
typedef QHash<QString, QStringList> HostListT;

typedef struct _CoreDataT {
  qint8 monitor;
  NodeListT bpnodes;
  NodeListT cnodes;
  CheckListT checks_;
  CheckStatusCountT check_status_count;
  HostListT hosts;
  TreeNodeItemListT tree_items;
}CoreDataT;

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

typedef QHash<QString, GNodeT> GNodeListT;
typedef QHash<QString, GEdgeT> GEdgeListT;
typedef QMap<QString, QMenu*> MenuListT;
typedef QMap<QString, QAction*> SubMenuListT;
typedef QMap<QString, QString> ComboBoxItemsT;
typedef QMap<qint32, QString> RequestListT;

#endif /* BASE_HPP */
