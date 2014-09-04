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
#include <QtCore/QtGlobal>
#include <QtCore>
#include <QtXml>
#include <QSettings>
#include <memory>
#include <unordered_map>


#define INIT_TRANSLATION \
  QTranslator translator; \
  translator.load(QString(":i18n/ngrt4n_%1").arg(QLocale::system().name())); \
  app->installTranslator(&translator); \
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

#ifdef REALOPINSIGHT_WEB
const QString APP_NAME = QObject::tr("%1 Ultimate").arg(REALOPINSIGHT_APPLICATION_NAME);
const int MAX_FILE_UPLOAD = 2048; // 2MB
const std::string LINK_HOME ="/home";
const std::string LINK_LOAD ="/preview-view";
const std::string LINK_IMPORT ="/upload-view";
const std::string LINK_LOGIN ="/login";
const std::string LINK_LOGOUT ="/logout";
const std::string LINK_ADMIN_HOME ="/adm-console";
const std::string LINK_OP_HOME ="/op-console";
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

struct CheckT {
  std::string id;
  std::string host;
  std::string check_command;
  std::string last_state_change;
  std::string alarm_msg;
  std::string host_groups;
  int status;
};
typedef std::unordered_map<std::string, CheckT> ChecksT;

namespace ngrt4n {
  enum ApiTypeT {
    Nagios = 0,
    Zabbix = 1,
    Zenoss = 2,
    Auto=99
  };

  enum SeverityT {
    Unset = -1,
    Normal = 0,
    Minor = 1,
    Major = 2,
    Critical = 3,
    Unknown = 4
  };

  enum NagiosStatusT {
    NagiosOk = 0,
    NagiosWarning = 1,
    NagiosCritical = 2,
    NagiosUnknown = 3
  };

  enum ZabbixSeverityT {
    ZabbixClear = 0,
    ZabbixInfo = 1,
    ZabbixWarn = 2,
    ZabbixAverage = 3,
    ZabbixHigh = 4,
    ZabbixDisaster = 5
  };

  enum ZenossSeverityT {
    ZenossClear = 0,
    ZenossDebug = 1,
    ZenossInfo = 2,
    ZenossWarning = 3,
    ZenossError = 4,
    ZenossCritical = 5
  };

  enum {
    AdmUserRole = 100,
    OpUserRole = 101
  };

  enum VisibilityT {
    Hidden = 0x0,
    Visible = 0x1,
    Expanded = 0x2,
    Collapsed = 0xFC
  };

  enum {
    XSCAL_FACTOR = 72,
    YSCAL_FACTOR = 100
  };

  enum RequestFilterT {
    HostFilter = 0,
    GroupFilter = 1
  };

  const std::string AdmUser = "ngrt4n_adm";
  const std::string OpUser = "ngrt4n_op";
  const std::string CHILD_SEP = ",";
  const std::string TAG_ZABBIX_HOSTNAME = "\\{HOSTNAME\\}";
  const std::string TAG_ZABBIX_HOSTNAME2 = "\\{HOST.NAME\\}";
  const std::string TAG_HOSTNAME = "\\{hostname\\}";
  const std::string TAG_CHECK = "\\{check_name\\}";
  const std::string TAG_THERESHOLD = "\\{threshold\\}";
  const std::string TAG_PLUGIN_OUTPUT = "\\{plugin_output\\}";
  const double THRESHOLD_WEIGHT_UNIT = 0.1;
  const double THRESHOLD_WEIGHT_MIN = 0;
  const double THRESHOLD_WEIGHT_MAX = 1;

  } // namespace ngrt4n

class PropRules {
public:
  enum PropRulesT{
    Unchanged = 0,
    Decreased = 1,
    Increased = 2
  };

  PropRules(int rule) : m_rule(rule) {}

  QString data(void) { return QString::number(m_rule); }

  QString toString(void) {
    switch( static_cast<PropRulesT>(m_rule) ) {
    case Unchanged: return QObject::tr("Unchanged");
    case Decreased: return QObject::tr("Decreased");
    case Increased: return QObject::tr("Increased");
    }

    return QObject::tr("Unchanged");
  }



private:
  int m_rule;
};


class CalcRules {
public:
  enum CalcRulesT{
    WorstSeverity = 0,
    AverageSeverity = 1,
    WeightedThresholdSeverity = 2
  };

  CalcRules(int rule) : m_rule(rule) {}

  QString data(void) { return QString::number(m_rule);}
  QString toString(void) const {
    QString result = QObject::tr("Default");
    switch (m_rule) {
    case AverageSeverity:
      result = QObject::tr("Average");
      break;
    case WeightedThresholdSeverity:
      result = QObject::tr("Weighted Threshold");
      break;
    case WorstSeverity:
    default:
      result = QObject::tr("Worst Severity");
      break;
    }
    return result;
  }

private:
  int m_rule;
};

class NodeType {
public:
  enum {
    ServiceNode = 0,
    AlarmNode = 1
  };
  static QString toString(int _type) {
    if (_type == AlarmNode )
      return QObject::tr("Native Check");
    return QObject::tr("Business Process");
  }
};


class Severity {
public:
  Severity(int sev): m_sev(sev) {}

  void setValue(int _value) {m_sev = _value;}
  int value() const {return m_sev;}

  QString valueString(void) const {return QString::number(m_sev);}

  QString toString(void) const {
    switch( m_sev )
    {
    case ngrt4n::Normal:
      return QObject::tr("Normal");
      break;
    case ngrt4n::Minor:
      return  QObject::tr("Minor");
      break;
    case ngrt4n::Major:
      return  QObject::tr("Major");
      break;
    case ngrt4n::Critical:
      return  QObject::tr("Critical");
      break;
    default:
      break;
    }
    return QObject::tr("Unknown");
  }

  Severity operator *(Severity& sev) const {
    switch(m_sev) {
    case ngrt4n::Critical:
      return Severity(m_sev);
      break;
    case ngrt4n::Normal:
      return sev;
      break;
    case ngrt4n::Minor:
      if(sev.m_sev == ngrt4n::Critical ||
         sev.m_sev == ngrt4n::Major ||
         sev.m_sev == ngrt4n::Unknown)
        return sev;

      return Severity(m_sev);
      break;
    case ngrt4n::Major:
      if(sev.m_sev == ngrt4n::Critical ||
         sev.m_sev == ngrt4n::Unknown)
        return sev;

      return Severity(m_sev);
      break;
    default:
      // MonitorBroker::CRITICITY_UNKNOWN
      if(sev.m_sev == ngrt4n::Critical)
        return sev;
      break;
    }  //end switch

    return Severity(ngrt4n::Unknown);
  }


  Severity operator / (Severity& st) const {
    if(m_sev == st.m_sev)
      return st;

    if(m_sev == ngrt4n::Critical ||
       st.m_sev == ngrt4n::Critical)
      return Severity(ngrt4n::Critical);

    if(m_sev == ngrt4n::Unknown ||
       st.m_sev == ngrt4n::Unknown)
      return Severity(ngrt4n::Unknown);

    if(m_sev == ngrt4n::Major ||
       st.m_sev == ngrt4n::Major)
      return Severity(ngrt4n::Major);

    if(m_sev == ngrt4n::Minor ||
       st.m_sev == ngrt4n::Minor)
      return Severity(ngrt4n::Minor);

    return Severity(ngrt4n::Normal);
  }

  Severity operator ++() {
    switch(m_sev) {
    case ngrt4n::Minor:
      return Severity(ngrt4n::Major);
      break;
    case ngrt4n::Major:
      return Severity(ngrt4n::Critical);
      break;
    default:
      //leave unchanged
      break;
    }
    return Severity(m_sev);
  }

  Severity operator--() {

    switch(m_sev) {
    case ngrt4n::Critical:
      return Severity(ngrt4n::Major);
      break;
    case ngrt4n::Major:
      return Severity(ngrt4n::Minor);
      break;
    default:
      //leave unchanged
      break;
    }
    return Severity(m_sev);
  }

private:
  int m_sev;
};

struct ThresholdT {
  double weight;
  int sev_in;
  int sev_out;
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
  double weight;
  QString child_nodes;
  CheckT check;
  QVector<ThresholdT> thresholds;
  bool monitored;
  qint8 visibility;
  double pos_x;
  double pos_y;
  NodeT(): sev_crule(PropRules::Unchanged),
    sev_prule(CalcRules::WorstSeverity),
    sev(ngrt4n::Unknown),
    weight(ngrt4n::THRESHOLD_WEIGHT_UNIT){}
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


#endif /* BASE_HPP */
