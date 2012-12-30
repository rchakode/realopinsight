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

QString utils::statusToString(const qint32& _status)
{
  switch(static_cast<MonitorBroker::CriticityT>(_status))
    {
    case MonitorBroker::CRITICITY_NORMAL:
      return "Normal";
      break;

    case MonitorBroker::CRITICITY_MINOR:
      return  "Info";
      break;

    case MonitorBroker::CRITICITY_MAJOR:
      return  "Warning";
      break;

    case MonitorBroker::CRITICITY_HIGH:
      return  "Critical";
      break;

    default:
      break;
    }

  return "Unknown";
}

void utils::clear(CoreDataT& data) {
  data.cnodes.clear();
  data.bpnodes.clear();
  data.tree_items.clear();
}

void utils::alert(const QString & msg) {
  QMessageBox::warning(0, QObject::tr("%1 - Warning").arg(appName), msg, QMessageBox::Yes);
}

QString utils::getAbsolutePath(const QString& _path) {
  QFileInfo fileInfo(_path);
  return fileInfo.absolutePath()%"/"%basename(_path.toAscii());
}

void utils::delay(const qint32& d) {
  sleep(d);
}


MonitorBroker::CriticityT utils::getCriticity(const int& _monitor, const int& _statusOrSeverity) {

  int criticity = MonitorBroker::CRITICITY_UNKNOWN;

  if(_monitor == MonitorBroker::NAGIOS) {

      switch(_statusOrSeverity) {
        case MonitorBroker::NAGIOS_OK:
          criticity = MonitorBroker::CRITICITY_NORMAL;
          break;

        case MonitorBroker::NAGIOS_WARNING:
          criticity = MonitorBroker::CRITICITY_MAJOR;
          break;

        case MonitorBroker::NAGIOS_CRITICAL:
          criticity = MonitorBroker::CRITICITY_HIGH;
          break;

        default:
          // MonitorBroker::NAGIOS_UNKNOWN
          // keep the default criticity
          break;
        }

    } else if (_monitor == MonitorBroker::ZABBIX) {

      switch(_statusOrSeverity) {
        case MonitorBroker::ZABBIX_INFO:
          criticity = MonitorBroker::CRITICITY_NORMAL;
          break;

        case MonitorBroker::ZABBIX_WARN:
          criticity = MonitorBroker::CRITICITY_MINOR;
          break;

        case MonitorBroker::ZABBIX_AVERAGE:
          criticity = MonitorBroker::CRITICITY_MAJOR;
          break;

        case MonitorBroker::ZABBIX_HIGH:
        case MonitorBroker::ZABBIX_DISASTER:
          criticity = MonitorBroker::CRITICITY_HIGH;
          break;
        default:
          // MonitorBroker::ZABBIX_UNCLASSIFIED
          // keep the default criticity
          break;
        }

    } else if (_monitor == MonitorBroker::ZENOSS){

      switch(_statusOrSeverity) {
        case MonitorBroker::ZENOSS_CLEAR:
          criticity = MonitorBroker::CRITICITY_NORMAL;
          break;

        case MonitorBroker::ZENOSS_DEBUG:
          criticity = MonitorBroker::CRITICITY_MINOR;
          break;

        case MonitorBroker::ZENOSS_WARNING:
          criticity = MonitorBroker::CRITICITY_MAJOR;
          break;

        case MonitorBroker::ZENOSS_ERROR:
        case MonitorBroker::ZENOSS_CRITICAL:
          criticity = MonitorBroker::CRITICITY_HIGH;
          break;
        default:
          // keep the default criticity
          break;
        }
    }

  return static_cast<MonitorBroker::CriticityT>(criticity);
}


QColor utils::getColor(const int& _criticity) {

  QColor color(StatsLegend::COLOR_UNKNOWN);
  switch (static_cast<MonitorBroker::CriticityT>(_criticity)) {
    case MonitorBroker::CRITICITY_NORMAL:
      color = StatsLegend::COLOR_NORMAL;
      break;

    case MonitorBroker::CRITICITY_MINOR:
      color = StatsLegend::COLOR_MINOR;
      break;

    case MonitorBroker::CRITICITY_MAJOR:
      color = StatsLegend::COLOR_MAJOR;
      break;

    case MonitorBroker::CRITICITY_HIGH:
      color = StatsLegend::COLOR_CRITICAL;
      break;

    default:
      // color = StatsLegend::COLOR_UNKNOWN;
      break;
    }

  return color;
}


QIcon utils::getTreeIcon(const int& _criticity) {

  QString ipath(":/images/built-in/unknown.png");
  switch (static_cast<MonitorBroker::CriticityT>(_criticity)) {
    case MonitorBroker::CRITICITY_NORMAL:
      ipath = ":/images/built-in/normal.png";
      break;

    case MonitorBroker::CRITICITY_MINOR:
      ipath = ":/images/built-in/minor.png";
      break;

    case MonitorBroker::CRITICITY_MAJOR:
      ipath = ":/images/built-in/major.png";
      break;

    case MonitorBroker::CRITICITY_HIGH:
      ipath = ":/images/built-in/critical.png";
      break;

    default:
      // color = ":/images/built-in/unknown.png";
      break;
    }

  return QIcon(ipath);
}

bool utils::findNode(NodeListT& bpnodes,
                     NodeListT& cnodes,
                     const QString& nodeId,
                     NodeListT::iterator& node) {
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

bool utils::findNode(CoreDataT* coreData, const QString& nodeId, NodeListT::iterator& node) {
  return findNode(coreData->bpnodes, coreData->cnodes, nodeId, node);
}
