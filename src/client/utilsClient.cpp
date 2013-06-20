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

QString utils::criticityToText(const qint32& _status)
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

void utils::clear(CoreDataT& data)
{
  data.cnodes.clear();
  data.bpnodes.clear();
  data.tree_items.clear();
}

QString utils::getAbsolutePath(const QString& _path)
{
  QFileInfo fileInfo(_path);
  return fileInfo.absolutePath()%"/"%basename(_path.toAscii());
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

int utils::computePropCriticity(const qint8& _critValue, const qint8& propRule)
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


QColor utils::computeColor(const int& _criticity)
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


QIcon utils::computeCriticityIcon(const int& _criticity)
{
  QString ipath(":/images/built-in/unknown.png");
  switch (static_cast<MonitorBroker::SeverityT>(_criticity)) {
    case MonitorBroker::Normal:
      ipath = ":/images/built-in/normal.png";
      break;
    case MonitorBroker::Minor:
      ipath = ":/images/built-in/minor.png";
      break;
    case MonitorBroker::Major:
      ipath = ":/images/built-in/major.png";
      break;
    case MonitorBroker::Critical:
      ipath = ":/images/built-in/critical.png";
      break;
    default:
      break;
  }
  return QIcon(ipath);
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
  QSplashScreen* screen = new QSplashScreen(QPixmap(":images/built-in/loading-screen.png"));
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
                          QString::number(src.use_ls),
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


CheckT utils::getUnknownService(int status, const QString& sid)
{
  CheckT invalidCheck;
  invalidCheck.status = status;
  invalidCheck.last_state_change = getCtime(0);
  invalidCheck.host = "-";
  invalidCheck.check_command = "-";
  invalidCheck.alarm_msg = QObject::tr("Unknown service (%1)").arg(sid).toStdString();;
  return invalidCheck;
}

QStringList utils::sourceTypes(void)
{
  return QStringList() << "Nagios-like"
                       << "Zabbix"
                       << "Zenoss";
}

QStringList utils::sourceIndexes(void)
{
  return QStringList() << "1"
                       << "2"
                       << "3"
                       << "4"
                       << "5"
                       << "6"
                       << "7"
                       << "8"
                       << "9";
}
