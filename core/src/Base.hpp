/*
 * Base.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-03-2014                                                  #
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
#include "global.hpp"
#include <QtCore/QtGlobal>
#include <QtCore>
#include <QtXml>
#include <QSettings>
#include <memory>


#define INIT_TRANSLATION \
  QTranslator translator; \
  translator.load(QString(":i18n/ngrt4n_%1").arg(QLocale::system().name())); \
  app->installTranslator(&translator); \
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

#ifdef REALOPINSIGHT_WEB
const QString APP_NAME = QObject::tr("%1 Ultimate").arg(REALOPINSIGHT_APPLICATION_NAME);
#else
const QString APP_NAME = QObject::tr("%1 Workstation").arg(REALOPINSIGHT_APPLICATION_NAME);
#endif
const QString COMPANY = "RealOpInsight Labs";
const QString USER_BN = "ngrt4n";
const QString CORE_VERSION = REALOPINSIGHT_CORE_VERSION;
const QString PKG_VERSION = REALOPINSIGHT_PACKAGE_VERSION;
const QString PKG_URL = REALOPINSIGHT_PACKAGE_URL;
const QString REL_NAME = REALOPINSIGHT_RELEASE_NAME;
const QString REL_YEAR = REALOPINSIGHT_RELEASE_YEAR;
const QString REPORT_BUG = REALOPINSIGHT_BUG_REPORT_EMAIL;
const long long BUILD_TIME = REALOPINSIGHT_BUILD_DATE;
const QString REL_INFO = QString("%1/%2").arg(PKG_VERSION, REL_YEAR);
const QString ID_PATTERN("%1/%2");
const qint32 MAX_SRCS = 10;

class LsHelper;
class ZbxHelper;
class ZnsHelper;
class ZmqSocket;

class PropRules {
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


class CalcRules {
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

class NodeType {
public:
  enum {
    ServiceNode = 0,
    AlarmNode = 1
  };
  static QString toString(int _type ) {
    if (_type == AlarmNode )
      return QObject::tr("Native Check");
    return QObject::tr("Business Process");
  }
};


class SeverityHelper {
public:
  SeverityHelper(ngrt4n::SeverityT _value): value(_value) {}
  void setValue(ngrt4n::SeverityT _value) {value = _value;}
  ngrt4n::SeverityT getValue() const {return value;}

  SeverityHelper operator *(SeverityHelper& sh) const {
    switch(value) {
    case ngrt4n::Critical:
      return SeverityHelper(value);
      break;
    case ngrt4n::Normal:
      return sh;
      break;
    case ngrt4n::Minor:
      if(sh.value == ngrt4n::Critical ||
         sh.value == ngrt4n::Major ||
         sh.value == ngrt4n::Unknown)
        return sh;

      return SeverityHelper(value);
      break;
    case ngrt4n::Major:
      if(sh.value == ngrt4n::Critical ||
         sh.value == ngrt4n::Unknown)
        return sh;

      return SeverityHelper(value);
      break;
    default:
      // MonitorBroker::CRITICITY_UNKNOWN
      if(sh.value == ngrt4n::Critical)
        return sh;
      break;
    }  //end switch

    return SeverityHelper(ngrt4n::Unknown);
  }


  SeverityHelper operator / (SeverityHelper& st) const {
    if(value == st.value)
      return st;

    if(value == ngrt4n::Critical ||
       st.value == ngrt4n::Critical)
      return SeverityHelper(ngrt4n::Critical);

    if(value == ngrt4n::Unknown ||
       st.value == ngrt4n::Unknown)
      return SeverityHelper(ngrt4n::Unknown);

    if(value == ngrt4n::Major ||
       st.value == ngrt4n::Major)
      return SeverityHelper(ngrt4n::Major);

    if(value == ngrt4n::Minor ||
       st.value == ngrt4n::Minor)
      return SeverityHelper(ngrt4n::Minor);

    return SeverityHelper(ngrt4n::Normal);
  }

  SeverityHelper operator ++() {
    switch(value) {
    case ngrt4n::Minor:
      return SeverityHelper(ngrt4n::Major);
      break;
    case ngrt4n::Major:
      return SeverityHelper(ngrt4n::Critical);
      break;
    default:
      //leave unchanged
      break;
    }
    return SeverityHelper(value);
  }

  SeverityHelper operator--() {

    switch(value) {
    case ngrt4n::Critical:
      return SeverityHelper(ngrt4n::Major);
      break;
    case ngrt4n::Major:
      return SeverityHelper(ngrt4n::Minor);
      break;
    default:
      //leave unchanged
      break;
    }
    return SeverityHelper(value);
  }

private:
  ngrt4n::SeverityT value;
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
  qint32 sev;
  qint32 sev_prop;
  int weight;
  QString child_nodes;
  CheckT check;
  bool monitored;
  qint8 visibility;
  double pos_x;
  double pos_y;
  NodeT():
    sev_crule(PropRules::Unchanged),
    sev_prule(CalcRules::HighCriticity),
    sev(ngrt4n::Unknown),
    weight(1){}
};

struct SeverityWeightInfoT {
  int sev;
  int weight;
  SeverityWeightInfoT() : sev(ngrt4n::Unknown), weight(1){}
  friend bool operator < (const SeverityWeightInfoT& s1, const SeverityWeightInfoT& s2) {
    return s1.sev < s2.sev;
  }
};

typedef QMap<qint32, qint32> CheckStatusCountT;
typedef QHash<QString, NodeT> NodeListT;
typedef NodeListT::Iterator NodeListIteratorT;
typedef CheckT CheckT;
typedef QHash<QString, CheckT> CheckListT;
typedef CheckListT::Iterator CheckListIterT;
typedef CheckListT::ConstIterator CheckListCstIterT;
typedef QHash<QString, QStringList> HostListT;
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


struct SourceT {
  QString id;
  qint8 mon_type;
  QString mon_url;
  qint8 use_ngrt4nd;
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
typedef QMap<QString, QString> StringMapT;
typedef QMap<qint32, QString> RequestListT;
typedef QPair<QString, QString> StringPairT;

enum {
  XSCAL_FACTOR = 72,
  YSCAL_FACTOR = 100
};


#endif /* BASE_HPP */
