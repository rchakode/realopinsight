/*
 * Base.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 24-05-2012                                                 #
#                                                                          #
# This file is part of RealOpInsight (http://RealOpInsight.com) authored   #
# by Rodrigue Chakode <rodrigue.chakode@gmail.com>                         #
#                                                                          #
# RealOpInsight is free software: you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with RealOpInsight.  If not, see <http://www.gnu.org/licenses/>.   #
#--------------------------------------------------------------------------#
 */

#ifndef BASE_HPP
#define BASE_HPP
#include "MonitorBroker.hpp"
#include "ns.hpp"
#include <QtCore/QtGlobal>
#include <QtCore>
#include <QtXml>
#include <QtGui>
#include <QSettings>
#include <memory>



#define INIT_TRANSLATION \
  QTranslator translator; \
  translator.load(QString(":i18n/ngrt4n_%1").arg(QLocale::system().name())); \
  app->installTranslator(&translator); \
  QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));


const QString PROJECT = "NGRT4N";
const QString USER_BN = "ngrt4n";
const QString PJT_NAME = PROJECT;
const QString APP_NAME = APPLICATION_NAME;
const QString PKG_NAME = "";
const QString PKG_VERSION = PACKAGE_VERSION;
const QString PKG_URL = PACKAGE_URL;
const QString REL_NAME = RELEASE_NAME;
const QString REL_YEAR = RELEASE_YEAR;
const QString REL_INFO = QString("%1/%2").arg(PKG_VERSION, REL_YEAR);
const QString REPORT_BUG = BUG_REPORT_EMAIL;
const QString ID_PATTERN("%1/%2");

const qint32 MAX_SRCS = 10;

class LIBREALOPINSIGHTSHARED_EXPORT LsHelper;
class LIBREALOPINSIGHTSHARED_EXPORT ZbxHelper;
class LIBREALOPINSIGHTSHARED_EXPORT ZnsHelper;
class LIBREALOPINSIGHTSHARED_EXPORT ZmqSocket;

class LIBREALOPINSIGHTSHARED_EXPORT PropRules {
public:
  enum PropRulesT{
    Unchanged = 0,
    Decreased = 1,
    Increased = 2
  };

  static QString toString(PropRulesT rule) {
    return QString::number(rule);
  }

  static QString label(qint32 rule) {
    return label(static_cast<PropRulesT>(rule));
  }
  static QString label(PropRulesT rule) {
    switch(rule) {
    case Unchanged: return QObject::tr("Unchanged");
    case Decreased: return QObject::tr("Decreased");
    case Increased: return QObject::tr("Increased");
    }

    return QObject::tr("Unchanged");
  }
};


class LIBREALOPINSIGHTSHARED_EXPORT CalcRules {
public:
  enum CalcRulesT{
    HighCriticity = 0,
    WeightedCriticity = 1
  };
  static QString toString(CalcRulesT rule) { return QString::number(rule);}
  static QString label(qint32 rule) { return label(static_cast<CalcRulesT>(rule));}
  static QString label(CalcRulesT rule) {
    if (rule == WeightedCriticity) return QObject::tr("Average");
    return QObject::tr("High Severity");}
};

class LIBREALOPINSIGHTSHARED_EXPORT NodeType {
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


class LIBREALOPINSIGHTSHARED_EXPORT Criticity {
public:
  Criticity(MonitorBroker::SeverityT _value): value(_value) {}
  void setValue(MonitorBroker::SeverityT _value) {value = _value;}
  MonitorBroker::SeverityT getValue() const {return value;}

  Criticity operator *(Criticity& _criticity) const {
    switch(value) {
    case MonitorBroker::Critical:
      return Criticity(value);
      break;
    case MonitorBroker::Normal:
      return _criticity;
      break;
    case MonitorBroker::Minor:
      if(_criticity.value == MonitorBroker::Critical ||
         _criticity.value == MonitorBroker::Major ||
         _criticity.value == MonitorBroker::Unknown)
        return _criticity;

      return Criticity(value);
      break;
    case MonitorBroker::Major:
      if(_criticity.value == MonitorBroker::Critical ||
         _criticity.value == MonitorBroker::Unknown)
        return _criticity;

      return Criticity(value);
      break;
    default:
      // MonitorBroker::CRITICITY_UNKNOWN
      if(_criticity.value == MonitorBroker::Critical)
        return _criticity;
      break;
    }  //end switch

    return Criticity(MonitorBroker::Unknown);
  }


  Criticity operator / (Criticity& st) const {
    if(value == st.value)
      return st;

    if(value == MonitorBroker::Critical ||
       st.value == MonitorBroker::Critical)
      return Criticity(MonitorBroker::Critical);

    if(value == MonitorBroker::Unknown ||
       st.value == MonitorBroker::Unknown)
      return Criticity(MonitorBroker::Unknown);

    if(value == MonitorBroker::Major ||
       st.value == MonitorBroker::Major)
      return Criticity(MonitorBroker::Major);

    if(value == MonitorBroker::Minor ||
       st.value == MonitorBroker::Minor)
      return Criticity(MonitorBroker::Minor);

    return Criticity(MonitorBroker::Normal);
  }

  Criticity operator ++(int) {
    switch(value) {
    case MonitorBroker::Minor:
      return Criticity(MonitorBroker::Major);
      break;

    case MonitorBroker::Major:
      return Criticity(MonitorBroker::Critical);
      break;

    default:
      //MonitorBroker::CRITICITY_NORMAL:
      //MonitorBroker::CRITICITY_UNKNOWN:
      //MonitorBroker::CRITICITY_HIGH:
      break;
    }

    return Criticity(value);
  }

  Criticity operator--(int) {

    switch(value) {
    case MonitorBroker::Critical:
      return Criticity(MonitorBroker::Major);
      break;

    case MonitorBroker::Major:
      return Criticity(MonitorBroker::Minor);
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
  MonitorBroker::SeverityT value;
};

struct NodeT {
  QString id;
  QString name;
  qint32 type;
  qint32 sev_crule;
  qint32 sev_prule;
  QString icon;
  QString description;
  QString parent;
  QString alarm_msg;
  QString notification_msg;
  QString actual_msg;
  qint32 severity;
  qint32 prop_sev;
  QString child_nodes;
  CheckT check;
  bool monitored;
  double pos_x;
  double pos_y;
};

typedef QMap<qint32, qint32> CheckStatusCountT;
typedef QHash<QString, NodeT> NodeListT;
typedef NodeListT::Iterator NodeListIteratorT;
typedef CheckT CheckT;
typedef QHash<QString, CheckT> CheckListT;
typedef CheckListT::Iterator CheckListIterT;
typedef CheckListT::ConstIterator CheckListCstIterT;
typedef QHash<QString, QStringList> HostListT;
typedef QHash<QString, QTreeWidgetItem*> GuiTreeItemListT;
typedef QMultiMap<QString, QString> StringListT;

struct CoreDataT {
  qint8 monitor;
  NodeListT bpnodes;
  NodeListT cnodes;
  CheckStatusCountT check_status_count;
  HostListT hosts;
  QSet<QString> sources;
  StringListT edges;
  double map_height;
  double map_width;
};

struct GNodeT {
  QGraphicsTextItem* label;
  QGraphicsPixmapItem* icon;
  QGraphicsPixmapItem* exp_icon;
  qint32 type;
  bool expand;
};
typedef QHash<QString, GNodeT> GNodeListT;

struct GEdgeT {
  QGraphicsPathItem* edge;
};
typedef QHash<QString, GEdgeT> GEdgeListT;


struct SourceT {
  QString id;
  qint8 mon_type;
  QString mon_url;
  qint8 use_ls;
  QString ls_addr;
  qint32 ls_port;
  QString auth;
  qint8 verify_ssl_peer;
  QString icon;
  std::shared_ptr<ZmqSocket> d4n_handler;
  std::shared_ptr<LsHelper> ls_handler;
  std::shared_ptr<ZbxHelper> zbx_handler;
  std::shared_ptr<ZnsHelper> zns_handler;
};

typedef QHash<int, SourceT> SourceListT;

typedef QMap<QString, QString> IconMapT;
typedef QList<QListWidgetItem*> CheckItemList;
typedef QMap<QString, QMenu*> MenuListT;
typedef QMap<QString, QAction*> SubMenuListT;
typedef QMap<QString, QString> StringMapT;
typedef QMap<qint32, QString> RequestListT;
typedef QPair<QString, QString> StringPairT;

enum {
  XSCAL_FACTOR = 72,
  YSCAL_FACTOR = 100
};


#endif /* BASE_HPP */
