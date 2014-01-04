/*
 * Utils.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 11-08-2012                                                 #
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

#include "StatsLegend.hpp"
#include "utilsClient.hpp"
#include <QFileInfo>


namespace {
  const QString DEFAULT_TIP_PATTERN(QObject::tr("Service: %1\nDescription: %2\nSeverity: %3\n   Calc. Rule: %4\n   Prop. Rule: %5"));
  const QString ALARM_SPECIFIC_TIP_PATTERN(QObject::tr("\nTarget Host: %6\nData Point: %7\nRaw Output: %8\nOther Details: %9"));
}

QString utils::severityText(const qint32& _status)
{
  switch(static_cast<MonitorBroker::SeverityT>(_status))
  {
    case MonitorBroker::Normal:
      return QObject::tr("Normal");
      break;
    case MonitorBroker::Minor:
      return  QObject::tr("Minor");
      break;
    case MonitorBroker::Major:
      return  QObject::tr("Major");
      break;
    case MonitorBroker::Critical:
      return  QObject::tr("Critical");
      break;
    default:
      break;
  }
  return QObject::tr("Unknown");
}

void utils::clear(CoreDataT& _cdata)
{
  _cdata.cnodes.clear();
  _cdata.bpnodes.clear();
  _cdata.edges.clear();
}

QString utils::getAbsolutePath(const QString& _path)
{
  QFileInfo fileInfo(_path);
  return fileInfo.absoluteFilePath();
}

MonitorBroker::SeverityT utils::computeCriticity(const int& _monitor, const int& _statusOrSeverity)
{
  int criticity = MonitorBroker::Unknown;
  if(_monitor == MonitorBroker::Nagios) {
    switch(_statusOrSeverity) {
      case MonitorBroker::NagiosOk:
        criticity = MonitorBroker::Normal;
        break;
      case MonitorBroker::NagiosWarning:
        criticity = MonitorBroker::Major;
        break;
      case MonitorBroker::NagiosCritical:
        criticity = MonitorBroker::Critical;
        break;
      default:
        break;
    }
  } else if (_monitor == MonitorBroker::Zabbix) {
    switch(_statusOrSeverity) {
      case MonitorBroker::ZabbixClear:
        criticity = MonitorBroker::Normal;
        break;
      case MonitorBroker::ZabbixInfo:
      case MonitorBroker::ZabbixWarn:
        criticity = MonitorBroker::Minor;
        break;
      case MonitorBroker::ZabbixAverage:
        criticity = MonitorBroker::Major;
        break;
      case MonitorBroker::ZabbixHigh:
      case MonitorBroker::ZabbixDisaster:
        criticity = MonitorBroker::Critical;
        break;
      default:
        break;
    }
  } else if (_monitor == MonitorBroker::Zenoss){
    switch(_statusOrSeverity) {
      case MonitorBroker::ZenossClear:
        criticity = MonitorBroker::Normal;
        break;
      case MonitorBroker::ZenossDebug:
        criticity = MonitorBroker::Minor;
        break;
      case MonitorBroker::ZenossWarning:
        criticity = MonitorBroker::Major;
        break;
      case MonitorBroker::ZenossError:
      case MonitorBroker::ZenossCritical:
        criticity = MonitorBroker::Critical;
        break;
      default:
        break;
    }
  }
  return static_cast<MonitorBroker::SeverityT>(criticity);
}

int utils::computeSeverity2Propagate(const qint8& _critValue, const qint8& propRule)
{
  MonitorBroker::SeverityT propCriticity = static_cast<MonitorBroker::SeverityT>(_critValue);
  Criticity criticity(static_cast<MonitorBroker::SeverityT>(_critValue));
  switch(propRule) {
    case PropRules::Increased: propCriticity = (criticity++).getValue();
      break;
    case PropRules::Decreased: propCriticity = (criticity--).getValue();
      break;
    default:
      break;
  }
  return propCriticity;
}


QColor utils::severityColor(const int& _criticity)
{
  QColor color(StatsLegend::COLOR_UNKNOWN);
  switch (static_cast<MonitorBroker::SeverityT>(_criticity)) {
    case MonitorBroker::Normal:
      color = StatsLegend::COLOR_NORMAL;
      break;
    case MonitorBroker::Minor:
      color = StatsLegend::COLOR_MINOR;
      break;
    case MonitorBroker::Major:
      color = StatsLegend::COLOR_MAJOR;
      break;
    case MonitorBroker::Critical:
      color = StatsLegend::COLOR_CRITICAL;
      break;
    default:
      break;
  }
  return color;
}


QIcon utils::severityIcon(int _severity)
{
  return QIcon(":/"+getIconPath(_severity));
}

QString utils::getIconPath(int _severity)
{
  QString ipath("/images/built-in/unknown.png");
  switch (static_cast<MonitorBroker::SeverityT>(_severity)) {
    case MonitorBroker::Normal:
      ipath = "/images/built-in/normal.png";
      break;
    case MonitorBroker::Minor:
      ipath = "/images/built-in/minor.png";
      break;
    case MonitorBroker::Major:
      ipath = "/images/built-in/major.png";
      break;
    case MonitorBroker::Critical:
      ipath = "/images/built-in/critical.png";
      break;
    default:
      break;
  }
  return ipath;
}

bool utils::findNode(CoreDataT* coreData, const QString& nodeId, NodeListT::iterator& node)
{
  return findNode(coreData->bpnodes, coreData->cnodes, nodeId, node);
}

bool utils::findNode(NodeListT& bpnodes,
                     NodeListT& cnodes,
                     const QString& nodeId,
                     NodeListT::iterator& node)
{
  bool found = false;
  node = bpnodes.find(nodeId);
  if(node != bpnodes.end()) {
    found = true;
  } else {
    node = cnodes.find(nodeId);
    if(node != cnodes.end()) {
      found = true;
    }
  }
  return found;
}

bool utils::findNode(const NodeListT& bpnodes,
                     const NodeListT& cnodes,
                     const QString& nodeId,
                     NodeListT::const_iterator& node)
{
  bool found = false;
  node = bpnodes.find(nodeId);
  if(node != bpnodes.end()) {
    found = true;
  } else {
    node = cnodes.find(nodeId);
    if(node != cnodes.end()) {
      found = true;
    }
  }
  return found;
}

QSplashScreen* utils::infoScreen(const QString & msg) {
  QSplashScreen* screen = new QSplashScreen(QPixmap(":/images/built-in/loading-screen.png"));
  screen->showMessage(msg, Qt::AlignJustify|Qt::AlignVCenter);
  screen->show();
  return screen;
}

QString utils::getWelcomeMsg(const QString& utility)
{
  return QObject::tr("       > %1 %2 %3 (codename: %4)"
                     "\n        >> Realease ID: %5"
                     "\n        >> Copyright (C) 2010 - %6 NGRT4N Project. All rights reserved"
                     "\n        >> For bug reporting instructions, see: <%7>").arg(APP_NAME,
                                                                                   utility,
                                                                                   PKG_VERSION,
                                                                                   REL_NAME,
                                                                                   REL_INFO,
                                                                                   REL_YEAR,
                                                                                   PKG_URL);
}

QString utils::source2Str(const SourceT& src)
{
  return QString("{\"sid\":\"%1\","
                 "\"mon_type\":\"%2\","
                 "\"use_ls\":\"%3\","
                 "\"mon_url\":\"%4\","
                 "\"ls_addr\":\"%5\","
                 "\"ls_port\":\"%6\","
                 "\"auth\":\"%7\","
                 "\"verify_ssl_peer\":\"%8\""
                 "}").arg(src.id,
                          QString::number(src.mon_type),
                          QString::number(src.use_ngrt4nd),
                          src.mon_url,
                          src.ls_addr,
                          QString::number(src.ls_port),
                          src.auth,
                          QString::number(src.verify_ssl_peer));
}

qint32 utils::convert2ApiType(const QString& str)
{
  QStringList types = sourceTypes();
  int type;
  if (str == types[MonitorBroker::Nagios])
    type = MonitorBroker::Nagios;
  else if (str == types[MonitorBroker::Zabbix])
    type = MonitorBroker::Zabbix;
  else if (str == types[MonitorBroker::Zenoss])
    type = MonitorBroker::Zenoss;
  else
    type = MonitorBroker::Auto;

  return type;
}


void utils::setCheckOnError(int status, const QString& msg, CheckT& invalidCheck)
{
  invalidCheck.status = status;
  invalidCheck.last_state_change = getCtime(0);
  invalidCheck.host = "-";
  invalidCheck.check_command = "-";
  invalidCheck.alarm_msg = msg.toStdString();
}

QStringList utils::sourceTypes(void)
{
  return QStringList() << "Nagios-like"
                       << "Zabbix"
                       << "Zenoss";
}

QStringList utils::sourceIndexes(void)
{
  return QStringList() << "0"
                       << "1"
                       << "2"
                       << "3"
                       << "4"
                       << "5"
                       << "6"
                       << "7"
                       << "8"
                       << "9";
}


/* return <[sourcei:]hostaddr, checkid> */
StringPairT utils::splitHostCheckInfo(const QString& info)
{
  int pos = info.indexOf("/");
  QString second = ((pos == -1)? "ping" : info.mid(pos+1));

  return QPair<QString, QString>(info.left(pos), second);
}


/* return <source, hostaddr> */
StringPairT utils::splitSourceHostInfo(const QString& info)
{
  int pos = info.indexOf(":");
  QString first;
  if (pos == -1) {
    first = SRC_BASENAME%"0";
    return QPair<QString, QString>(first, info);
  }

  return QPair<QString, QString>(info.left(pos), info.mid(pos+1));
}


QString utils::getSourceIdFromStr(const QString& str)
{
  QString srcid = "";
  int pos = str.indexOf(":");
  if (pos != -1) {
    srcid = str.mid(0, pos);
  }
  return srcid;
}

QPair<bool, int> utils::checkSourceId(const QString &id)
{
  int index = -1;
  bool valid = false;
  if (! id.isEmpty()) {
    QString idStr = id.at(id.size()-1);
    if (id == SRC_BASENAME%idStr) {
      valid = true;
      index = idStr.toInt();
    }
  }
  return QPair<bool, int>(valid, index);
}


QString utils::getNodeToolTip(const NodeT& _node)
{
  QString toolTip = DEFAULT_TIP_PATTERN.arg(_node.name,
                                            const_cast<QString&>(_node.description).replace("\n", " "),
                                            utils::severityText(_node.severity),
                                            CalcRules::label(_node.sev_crule),
                                            PropRules::label(_node.sev_prule));
  if (_node.type == NodeType::AlarmNode) {
    toolTip += ALARM_SPECIFIC_TIP_PATTERN.arg(QString::fromStdString(_node.check.host).replace("\n", " "),
                                              _node.child_nodes,
                                              QString::fromStdString(_node.check.alarm_msg),
                                              _node.actual_msg);
  }
  return toolTip;
}

IconMapT utils::nodeIcons() {
  IconMapT icons;
  icons[DEFAULT_ICON]= ":/images/business-process.png";
  icons["Other Check"] = ":/images/check.png";
  icons["Server"] = ":/images/server.png";
  icons["Firewall"] = ":/images/firewall.png";
  icons["Router"] = ":/images/router.png";
  icons["Network"] = ":/images/network.png";
  icons["Switch"] = ":/images/switch.png";
  icons["Filer"] = ":/images/filer.png";
  icons["Hard disk"] = ":/images/harddisk.png";
  icons["Storage Area"] = ":/images/storage.png";
  icons["Linux"] = ":/images/linux.png";
  icons["Windows OS"] = ":/images/windows.png";
  icons["Solaris"] = ":/images/solaris.png";
  icons["Cloud"] = ":/images/cloud.png";
  icons["Hypervisor"] = ":/images/hypervisor.png";
  icons["Application"] = ":/images/application.png";
  icons["Web Accessibility"] = ":/images/web.png";
  icons["Web server"] = ":/images/web-server.png";
  icons["Database Engine"] = ":/images/db.png";
  icons["Database Server"] = ":/images/db-server.png";
  icons["Process"] = ":/images/process.png";
  icons["Logfile"] = ":/images/log.png";
  icons["Network Bandwith"] = ":/images/network-usage.png";
  icons["CPU"] = ":/images/cpu.png";
  icons["CPU Load"] = ":/images/performance-level.png";
  icons["Memory"] = ":/images/memory.png";
  icons["Memory Usage"] = ":/images/memory-usage.png";
  icons["Resource Utilization"] = ":/images/resource-usage.png";
  icons["Performance"] = ":/images/performance.png";
  icons[PLUS] = ":/images/plus.png";
  icons[MINUS] = ":/images/minus.png";
  icons["Nagios Basic Logo"] = ":/images/nagios-logo-n.png";
  icons["Zabbix Basic Logo"] = ":/images/zabbix-logo-z.png";
  icons["Zenoss Basic Logo"] = ":/images/zenoss-logo-o.png";
  icons["Hierarchy"] = ":/images/hierarchy.png";
  icons[MINUS] = ":/images/minus.png";
  return icons;
}


void utils::alert(const QString& msg)
{
  QMessageBox::warning(0, QObject::tr("%1 - Warning").arg(APP_NAME), msg, QMessageBox::Yes);
}

