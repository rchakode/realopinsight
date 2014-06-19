/*
 * Utils.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 11-08-2012                                                 #
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
#include "StatsLegend.hpp"
#include "utilsClient.hpp"
#include <QFileInfo>

QString utils::criticityToText(const qint32& _status)
{
  switch(static_cast<MonitorBroker::SeverityT>(_status))
    {
    case MonitorBroker::Normal:
      return "Normal";
      break;
    case MonitorBroker::Minor:
      return  "Minor";
      break;
    case MonitorBroker::Major:
      return  "Major";
      break;
    case MonitorBroker::Critical:
      return  "Critical";
      break;
    default:
      break;
    }
  return "Unknown";
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

