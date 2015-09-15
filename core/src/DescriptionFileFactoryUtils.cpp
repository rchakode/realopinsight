/*
* DescriptionFileFactoryUtils.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Creation : 26-06-2015                                                    #
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

#include "DescriptionFileFactoryUtils.hpp"
#include "utilsCore.hpp"
#include "ZbxHelper.hpp"
#include "LsHelper.hpp"
#include "ZnsHelper.hpp"
#include "PandoraHelper.hpp"
#include "OpManagerHelper.hpp"
#include "ThresholdHelper.hpp"


int ngrt4n::importHostGroupAsBusinessView(const SourceT& srcInfo, const QString& filter, CoreDataT& cdata, QString& errorMsg)
{
  ChecksT checks;
  if (importMonitorItemAsDataPoints(srcInfo, filter, checks, errorMsg) != 0) {
    errorMsg = MonitorT::toString(srcInfo.mon_type).append(": ").append(errorMsg);
    return -1;
  }

  // handle results
  if (! checks.empty()) {
    ngrt4n::clearCoreData(cdata);
    cdata.monitor = MonitorT::Auto;

    NodeT root;
    root.id = ngrt4n::ROOT_ID;
    root.name = filter.isEmpty() ? QObject::tr("%1 Services").arg(MonitorT::toString(srcInfo.mon_type)) : filter;
    root.type = NodeType::BusinessService;

    NodeT hostNode;
    NodeT triggerNode;
    hostNode.type = NodeType::BusinessService;
    triggerNode.type = NodeType::ITService;

    for (ChecksT::ConstIterator check = checks.begin(); check != checks.end(); ++check) {
      hostNode.parent = root.id;
      hostNode.name = hostNode.description = QString::fromStdString(check->host);
      hostNode.id = "";
      Q_FOREACH(QChar c, hostNode.name) { if (c.isLetterOrNumber()) { hostNode.id.append(c); } }
      QString checkId = QString::fromStdString(check->id);
      triggerNode.id = ngrt4n::genNodeId();
      triggerNode.parent = hostNode.id;
      triggerNode.name = checkId.startsWith(hostNode.name+"/") ? checkId.mid(hostNode.name.size() + 1) : checkId;
      triggerNode.child_nodes = QString::fromStdString("%1:%2").arg(srcInfo.id, checkId);

      NodeListIteratorT hostIterPos =  cdata.bpnodes.find(hostNode.id);
      if (hostIterPos != cdata.bpnodes.end()) {
        hostIterPos->child_nodes.append(ngrt4n::CHILD_Q_SEP).append(triggerNode.id);
      } else {
        hostNode.child_nodes = triggerNode.id;
        if (root.child_nodes.isEmpty()) {
          root.child_nodes = hostNode.id;
        } else {
          root.child_nodes.append(ngrt4n::CHILD_Q_SEP).append(hostNode.id);
        }
        cdata.bpnodes.insert(hostNode.id, hostNode);
      }
      cdata.cnodes.insert(triggerNode.id, triggerNode);
    }

    // finally insert the root node and update UI widgets
    cdata.bpnodes.insert(ngrt4n::ROOT_ID, root);
  }

  return 0;
}


int ngrt4n::importMonitorItemAsDataPoints(const SourceT& srcInfo, const QString& filter, ChecksT& checks, QString& errorMsg)
{
  int retcode = -1;
  if (srcInfo.mon_type == MonitorT::Nagios) {
    /* Nagios monitor ::  only Livestatus is now officially supported */
    LsHelper handler(srcInfo.ls_addr, srcInfo.ls_port);
    if (handler.setupSocket() == 0 && handler.loadChecks(filter, checks) == 0) {
      retcode = 0;
    }
    errorMsg = handler.lastError();
  } else if (srcInfo.mon_type == MonitorT::Zabbix) {
    /* Zabbix monitor */
    ZbxHelper handler;
    retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::GroupFilter);
    if (checks.empty()) {
      retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::HostFilter);
    }
    errorMsg = handler.lastError();

  } else if (srcInfo.mon_type == MonitorT::Zenoss) {
    /* Zenoss monitor */
    ZnsHelper handler(srcInfo.mon_url);
    retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::HostFilter);
    if (checks.empty()) {
      retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::GroupFilter);
    }
    errorMsg = handler.lastError();

  } else if (srcInfo.mon_type == MonitorT::Pandora) {
    /* Panadora FMS monitor */
    PandoraHelper handler(srcInfo.mon_url);
    retcode = handler.loadChecks(srcInfo, checks, filter);
    errorMsg = handler.lastError();

  } else if (srcInfo.mon_type == MonitorT::OpManager) {
    /* OpManager monitor */
    OpManagerHelper handler(srcInfo.mon_url);
    if (filter.isEmpty()) {
      retcode = handler.loadChecks(srcInfo, OpManagerHelper::ListAllDevices, filter, checks);
    } else {
      retcode = handler.loadChecks(srcInfo, OpManagerHelper::ListDeviceByName, filter, checks);
      if (checks.empty()) {
        retcode = handler.loadChecks(srcInfo, OpManagerHelper::ListDeviceByCategory, filter, checks);
        if (checks.empty()) {
          retcode = handler.loadChecks(srcInfo, OpManagerHelper::ListDeviceByType, filter, checks);
        }
      }
    }
    errorMsg = handler.lastError();
  } else {
    errorMsg = QObject::tr("Unknown data source type");
  }

  return retcode;
}


int ngrt4n::saveDataAsDescriptionFile(const QString& path, const CoreDataT& cdata, QString& errorMsg)
{
  QFile file(path);
  if (! file.open(QIODevice::WriteOnly|QIODevice::Text)) {
    errorMsg = QObject::tr("Cannot open file: %1").arg(path);
    return -1;
  }

  NodeListT::ConstIterator rootNode = cdata.bpnodes.find(ngrt4n::ROOT_ID);
  if (rootNode == cdata.bpnodes.end()) {
    file.close();
    errorMsg = QObject::tr("The hierarchy does not have root");
    return -1;
  }

  QTextStream outStream(&file);
  outStream << "<?xml version=\"1.0\"?>\n"
            << QString("<ServiceView compat=\"3.1\" monitor=\"%1\">\n").arg( QString::number(cdata.monitor) )
            << generateNodeXml(*rootNode);

  Q_FOREACH(const NodeT& service, cdata.bpnodes) {
    if (service.id != ngrt4n::ROOT_ID && ! service.parent.isEmpty()) {
      outStream << generateNodeXml(service);
    }
  }

  Q_FOREACH(const NodeT& service, cdata.cnodes) {
    if (! service.parent.isEmpty()) {
      outStream << generateNodeXml(service);
    }
  }

  outStream << "</ServiceView>\n";

  file.close();
  return 0;
}


QString ngrt4n::generateNodeXml(const NodeT& node)
{
  QString xml = QString("<Service id=\"%1\" "
                        " type=\"%2\" "
                        " statusCalcRule=\"%3\" "
                        " statusPropRule=\"%4\" "
                        " weight=\"%5\"> \n"
                        ).arg(node.id,
                              QString::number(node.type),
                              QString::number(node.sev_crule),
                              QString::number(node.sev_prule),
                              QString::number(node.weight));

  xml.append( QString(" <Name>%1</Name>\n").arg(node.name) )
      .append( QString(" <Icon>%1</Icon>\n").arg(node.icon) )
      .append( QString(" <Description>%1</Description>\n").arg(node.description) )
      .append( QString(" <AlarmMsg>%1</AlarmMsg>\n").arg(node.alarm_msg) )
      .append( QString(" <NotificationMsg>%1</NotificationMsg>\n").arg(node.notification_msg) )
      .append( QString(" <SubServices>%1</SubServices>\n").arg(node.child_nodes) ) ;

  if (node.sev_crule == CalcRules::WeightedAverageWithThresholds) {

    xml.append( QString(" <Thresholds>%1</Thresholds>\n").arg(ThresholdHelper::listToData(node.thresholdLimits)) );
  }

  xml.append("</Service>\n");

  return xml;
}

