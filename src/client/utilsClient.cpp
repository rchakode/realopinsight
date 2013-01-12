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
#include <unistd.h>

QString utils::criticityToText(const qint32& _status)
{
  switch(static_cast<MonitorBroker::CriticityT>(_status))
    {
    case MonitorBroker::CriticityNormal:
      return "Normal";
      break;
    case MonitorBroker::CriticityMinor:
      return  "Minor";
      break;
    case MonitorBroker::CriticityMajor:
      return  "Major";
      break;
    case MonitorBroker::CriticityHigh:
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

MonitorBroker::CriticityT utils::computeCriticity(const int& _monitor, const int& _statusOrSeverity)
{
  int criticity = MonitorBroker::CriticityUnknown;
  if(_monitor == MonitorBroker::Nagios) {
      switch(_statusOrSeverity) {
        case MonitorBroker::NagiosOk:
          criticity = MonitorBroker::CriticityNormal;
          break;
        case MonitorBroker::NagiosWarning:
          criticity = MonitorBroker::CriticityMajor;
          break;
        case MonitorBroker::NagiosCritical:
          criticity = MonitorBroker::CriticityHigh;
          break;
        default:
          break;
        }
    } else if (_monitor == MonitorBroker::Zabbix) {
      switch(_statusOrSeverity) {
        case MonitorBroker::ZabbixClear:
          criticity = MonitorBroker::CriticityNormal;
          break;
        case MonitorBroker::ZabbixInfo:
        case MonitorBroker::ZabbixWarn:
          criticity = MonitorBroker::CriticityMinor;
          break;
        case MonitorBroker::ZabbixAverage:
          criticity = MonitorBroker::CriticityMajor;
          break;
        case MonitorBroker::ZabbixHigh:
        case MonitorBroker::ZabbixDisaster:
          criticity = MonitorBroker::CriticityHigh;
          break;
        default:
          break;
        }
    } else if (_monitor == MonitorBroker::Zenoss){
      switch(_statusOrSeverity) {
        case MonitorBroker::ZenossClear:
          criticity = MonitorBroker::CriticityNormal;
          break;
        case MonitorBroker::ZenossDebug:
          criticity = MonitorBroker::CriticityMinor;
          break;
        case MonitorBroker::ZenossWarning:
          criticity = MonitorBroker::CriticityMajor;
          break;
        case MonitorBroker::ZenossError:
        case MonitorBroker::ZenossCritical:
          criticity = MonitorBroker::CriticityHigh;
          break;
        default:
          break;
        }
    }
  return static_cast<MonitorBroker::CriticityT>(criticity);
}

int utils::computePropCriticity(const qint8& _critValue, const qint8& propRule)
{
  MonitorBroker::CriticityT propCriticity = static_cast<MonitorBroker::CriticityT>(_critValue);
  Criticity criticity(static_cast<MonitorBroker::CriticityT>(_critValue));
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
  switch (static_cast<MonitorBroker::CriticityT>(_criticity)) {
    case MonitorBroker::CriticityNormal:
      color = StatsLegend::COLOR_NORMAL;
      break;
    case MonitorBroker::CriticityMinor:
      color = StatsLegend::COLOR_MINOR;
      break;
    case MonitorBroker::CriticityMajor:
      color = StatsLegend::COLOR_MAJOR;
      break;
    case MonitorBroker::CriticityHigh:
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
  switch (static_cast<MonitorBroker::CriticityT>(_criticity)) {
    case MonitorBroker::CriticityNormal:
      ipath = ":/images/built-in/normal.png";
      break;
    case MonitorBroker::CriticityMinor:
      ipath = ":/images/built-in/minor.png";
      break;
    case MonitorBroker::CriticityMajor:
      ipath = ":/images/built-in/major.png";
      break;
    case MonitorBroker::CriticityHigh:
      ipath = ":/images/built-in/critical.png";
      break;
    default:
      break;
    }
  return QIcon(ipath);
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

bool utils::findNode(CoreDataT* coreData, const QString& nodeId, NodeListT::iterator& node)
{
  return findNode(coreData->bpnodes, coreData->cnodes, nodeId, node);
}
