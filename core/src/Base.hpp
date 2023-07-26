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
#include <QSettings>
#include <memory>
#include <unordered_map>


#define INIT_TRANSLATION \
  QTranslator translator; \
  translator.load(QString(":i18n/ngrt4n_%1").arg(Settings().language())); \
  app->installTranslator(&translator); \
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

const QString APP_NAME = REALOPINSIGHT_APPLICATION_NAME;
const int MAX_FILE_UPLOAD = 4 * 1024; // 4MB => see also max-request-size wt config file
const int NEATO_EDGE_LENGTH = 3;
const double NEATO_X_TRANSLATION_FACTOR = 50;

const QString COMPANY = "RealOpInsight Labs";
const QString USER_BN = "ngrt4n";
const QString PKG_VERSION = REALOPINSIGHT_PACKAGE_VERSION;
const QString PKG_URL = REALOPINSIGHT_PACKAGE_URL;
const QString DOCS_URL = REALOPINSIGHT_GET_HELP_URL;
const QString REL_NAME = REALOPINSIGHT_RELEASE_NAME;
const QString REL_YEAR = REALOPINSIGHT_RELEASE_YEAR;
const QString REPORT_BUG = REALOPINSIGHT_BUG_REPORT_EMAIL;
const long BUILD_TIME = REALOPINSIGHT_BUILD_DATE;
const QString REL_INFO = QString("%1/%2").arg(PKG_VERSION, REL_YEAR);
const QString ID_PATTERN("%1/%2");
const qint32 MAX_SRCS = 10;

struct CheckT {
  std::string id;
  std::string host;
  std::string check_command;
  std::string last_state_change;
  std::string alarm_msg;
  std::string host_groups;
  int status;
};
typedef QMap<std::string, CheckT> ChecksT;

class MonitorT {
public:
  enum {
    Nagios  = 0,
    Zabbix  = 1,
    Kubernetes = 5,
    Any   = 99
  };

  static QString toString(int type) {
    QString value = "";
    switch (type) {
      case Nagios:
        value = QObject::tr("Nagios");
        break;
      case Zabbix:
        value = QObject::tr("Zabbix");
        break;
      case Kubernetes:
        value = QObject::tr("Kubernetes");
        break;
      case Any:
      default:
        value = QObject::tr("Undefined");
        break;
    }
    return value;
  }
};


namespace ngrt4n {
  enum SeverityT {
    Unset    = -1,
    Normal   = 0,
    Minor    = 1,
    Major    = 2,
    Critical = 3,
    Unknown  = 4
  };

  struct AggregatedSeverityT {
    int sev;
    double weight;
  };

  enum NagiosStatusT {
    NagiosOk       = 0,
    NagiosWarning  = 1,
    NagiosCritical = 2,
    NagiosUnknown  = 3
  };

  enum ZabbixSeverityT {
    ZabbixClear    = 0,
    ZabbixInfo     = 1,
    ZabbixWarn     = 2,
    ZabbixAverage  = 3,
    ZabbixHigh     = 4,
    ZabbixDisaster = 5
  };

  enum ZenossSeverityT {
    ZenossClear    = 0,
    ZenossDebug    = 1,
    ZenossInfo     = 2,
    ZenossWarning  = 3,
    ZenossError    = 4,
    ZenossCritical = 5
  };

  enum PandoraSeverityT {
    PandoraNormal   = 0,
    PandoraCritical = 1,
    PandoraWarning  = 2,
    PandoraUnknown  = 3
  };

  enum OpManagerSeverityT {
    OpManagerCritical  = 1,
    OpManagerTrouble   = 2,
    OpManagerAttention = 3,
    OpManagerDown      = 4,
    OpManagerClear     = 5
  };

  enum K8sSummaryStatusT {
    K8sPending  = 1,
    K8sRunning  = 2,
    K8sSucceeded = 3,
    K8sFailed = 4
  };

  enum K8sPodPhaseStatusT {
    K8sPodPhaseUndefined  = 0,
    K8sPodPhasePending  = 1,
    K8sPodPhaseRunning  = 2,
    K8sPodPhaseSucceeded = 3,
    K8sPodPhaseFailed = 4,
    K8sPodPhaseCrashLoopBackoff = 5
  };

  enum {
    AdmUserRole = 100,
    OpUserRole  = 101
  };

  enum VisibilityT {
    Hidden    = 0x0,
    Visible   = 0x1,
    Expanded  = 0x2,
    Collapsed = 0xFC
  };



  enum RequestFilterT {
    HostFilter  = 0,
    GroupFilter = 1
  };


  enum GraphLayoutT {
    DotLayout = 1,
    NeatoLayout = 2
  };

  enum ReturnCodeT {
    RcGenericFailure = -1,
    RcSuccess = 0,
    RcRpcError = 1,
    RcUnexpectedFailure = 2,
    RcParseError = 3,
    RcDbError = 10,
    RcDbDuplicationError = 11
  };

  const std::string CHILD_SEP = ",";
  const QString CHILD_Q_SEP = QString::fromStdString(CHILD_SEP);
  const std::string TAG_ZABBIX_HOSTNAME  = "\\{HOSTNAME\\}";
  const std::string TAG_ZABBIX_HOSTNAME2 = "\\{HOST.NAME\\}";
  const std::string TAG_HOSTNAME   = "\\{hostname\\}";
  const std::string TAG_CHECK      = "\\{check_name\\}";
  const std::string TAG_THERESHOLD = "\\{threshold\\}";
  const std::string TAG_PLUGIN_OUTPUT = "\\{plugin_output\\}";
  const double WEIGHT_UNIT = 1.0;
  const double WEIGHT_MIN  = 0;
  const double WEIGHT_MAX  = 10;

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
  QString toString(void) const;
private:
  int m_rule;
};


class CalcRules {
public:
  enum CalcRulesT{
    Worst = 0,
    Average = 1,
    WeightedAverageWithThresholds = 2
  };
  CalcRules(int rule) : m_rule(rule) {}
  QString data(void) { return QString::number(m_rule);}
  QString toString(void) const;
private:
  int m_rule;
};

class NodeType {
public:
  enum {
    BusinessService = 0,
    ITService = 1,
    ExternalService = 2,
    K8sClusterService = 3
  };
  static QString toString(int _type);
};


class Severity {
public:
  Severity(int sev): m_sev(sev) {}
  void setValue(int _value) {m_sev = _value;}
  int value() const {return m_sev;}
  QString valueString(void) const {return QString::number(m_sev);}
  bool isValid() { return m_sev >= static_cast<int>(ngrt4n::Normal) && m_sev <= static_cast<int>(ngrt4n::Unknown);}
  QString toString(void) const;
  std::string toStdString(void) const;
  Severity operator *(Severity& sev) const;
  Severity operator / (Severity& st) const;
  Severity operator ++();
  Severity operator--();

private:
  int m_sev;
};

struct ThresholdT {
  double weight;
  int sev_in;
  int sev_out;
};

struct ThresholdLessThanFnt {
  bool operator () (const ThresholdT& th1, const ThresholdT& th2)
  {
    if (th1.sev_out < th2.sev_out)
      return true;

    if (th1.sev_out == th2.sev_out) {
      if (th1.sev_in < th2.sev_in)
        return true;

      if (th1.sev_in == th2.sev_in)
        return th1.weight < th2.weight;

      return false;
    }
    return false;
  }
};


struct NodeT {
  QString id;
  QString name;
  qint32 type;
  qint32 sev;
  qint32 sev_crule;
  qint32 sev_prule;
  qint32 sev_prop;
  QString icon;
  QString description;
  QSet<QString> parents;
  QString alarm_msg;
  QString notification_msg;
  QString actual_msg;
  double weight;
  QString child_nodes;
  CheckT check;
  QVector<ThresholdT> thresholdLimits;
  bool monitored;
  qint8 visibility;
  double pos_x;
  double pos_y;
  double text_w;
  double text_h;

  QString toThresholdsString(void) const;
  QString toString(void) const;
};

typedef QHash<QString, NodeT> NodeListT;
typedef QMap<qint32, qint32> CheckStatusCountT;
typedef QHash<QString, QStringList> HostListT;

struct CoreDataT {
  qint8 graph_mode;
  double min_x;
  double min_y;
  qint8 monitor;
  double format_version;
  NodeListT bpnodes;
  NodeListT cnodes;
  CheckStatusCountT check_status_count;
  HostListT hosts;
  QSet<QString> sources;
  QMultiMap<QString, QString>  edges;
  double map_height;
  double map_width;

  void clear(void)
  {
    cnodes.clear();
    bpnodes.clear();
    edges.clear();
    monitor = MonitorT::Any;
  }
};


struct SourceT {
  QString id;
  qint8 mon_type;
  QString mon_url;
  QString ls_addr;
  qint32 ls_port;
  QString auth;
  qint8 verify_ssl_peer;
  QString icon;
};



class ScaleFactors {
public:
  struct Coordinates {
    double x;
    double y;
  };
  ScaleFactors (int graphLayout) {
    const double LESS_SCALE_FACTOR = 72;
    const double GREAT_SCALE_FACTOR = 100;
    switch (graphLayout) {
      case ngrt4n::NeatoLayout:
        m_factors = {GREAT_SCALE_FACTOR, GREAT_SCALE_FACTOR};
        break;
      case ngrt4n::DotLayout:
      default:
        m_factors = {LESS_SCALE_FACTOR, GREAT_SCALE_FACTOR};
        break;
    }
  }

  double x() const {return m_factors.x;}
  double y() const {return m_factors.y;}

private:
  Coordinates m_factors;
};



typedef QHash<QString, SourceT> SourceListT;
typedef QMap<QString, std::string> IconMapT;
typedef QMap<QString, QString> StringMapT;
typedef QMap<qint32, QString> RequestListT;
typedef QPair<QString, QString> StringPairT;



#endif /* BASE_HPP */
