/*
 * utilsCore.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
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

#include "utilsCore.hpp"
#include "ZbxHelper.hpp"
#include "LsHelper.hpp"
#include "ZnsHelper.hpp"
#include "PandoraHelper.hpp"
#include "OpManagerHelper.hpp"
#include "ThresholdHelper.hpp"
#include "K8sHelper.hpp"

#include <QFileInfo>


QString ngrt4n::getAbsolutePath(const QString& _path)
{
  QFileInfo fileInfo(_path);
  return fileInfo.absoluteFilePath();
}

qint8 ngrt4n::severityFromProbeStatus(const int& monitorType, const int& statusValue)
{
  qint8 criticity = ngrt4n::Unknown;
  if (monitorType == MonitorT::Nagios) {
    switch(statusValue) {
      case ngrt4n::NagiosOk:
        criticity = ngrt4n::Normal;
        break;
      case ngrt4n::NagiosWarning:
        criticity = ngrt4n::Major;
        break;
      case ngrt4n::NagiosCritical:
        criticity = ngrt4n::Critical;
        break;
      default:
        break;
    }
    return static_cast<ngrt4n::SeverityT>(criticity);
  }

  if (monitorType == MonitorT::Zabbix) {
    switch(statusValue) {
      case ngrt4n::ZabbixClear:
        criticity = ngrt4n::Normal;
        break;
      case ngrt4n::ZabbixInfo:
      case ngrt4n::ZabbixWarn:
        criticity = ngrt4n::Minor;
        break;
      case ngrt4n::ZabbixAverage:
        criticity = ngrt4n::Major;
        break;
      case ngrt4n::ZabbixHigh:
      case ngrt4n::ZabbixDisaster:
        criticity = ngrt4n::Critical;
        break;
      default:
        break;
    }
    return static_cast<ngrt4n::SeverityT>(criticity);
  }

  if (monitorType == MonitorT::Zenoss){
    switch(statusValue) {
      case ngrt4n::ZenossClear:
        criticity = ngrt4n::Normal;
        break;
      case ngrt4n::ZenossDebug:
        criticity = ngrt4n::Minor;
        break;
      case ngrt4n::ZenossWarning:
        criticity = ngrt4n::Major;
        break;
      case ngrt4n::ZenossError:
      case ngrt4n::ZenossCritical:
        criticity = ngrt4n::Critical;
        break;
      default:
        break;
    }
    return static_cast<ngrt4n::SeverityT>(criticity);
  }

  if (monitorType == MonitorT::Pandora) {
    switch(statusValue) {
      case ngrt4n::PandoraNormal:
        criticity = ngrt4n::Normal;
        break;
      case ngrt4n::PandoraWarning:
        criticity = ngrt4n::Major;
        break;
      case ngrt4n::PandoraCritical:
        criticity = ngrt4n::Critical;
        break;
      case ngrt4n::PandoraUnknown:
      default:
        criticity = ngrt4n::Unknown;
        break;
    }

    return static_cast<ngrt4n::SeverityT>(criticity);
  }

  if (monitorType == MonitorT::OpManager) {
    switch(statusValue) {
      case ngrt4n::OpManagerClear:
        criticity = ngrt4n::Normal;
        break;
      case ngrt4n::OpManagerAttention:
        criticity = ngrt4n::Minor;
        break;
      case ngrt4n::OpManagerTrouble:
        criticity = ngrt4n::Major;
        break;
      case ngrt4n::OpManagerCritical:
        criticity = ngrt4n::Critical;
        break;
      case ngrt4n::OpManagerDown:
      default:
        criticity = ngrt4n::Unknown;
        break;
    }
    return static_cast<ngrt4n::SeverityT>(criticity);
  }

  if (monitorType == MonitorT::Kubernetes) {
    switch(statusValue) {
      case ngrt4n::K8sRunning:
      case ngrt4n::K8sTerminatedNormal:
        criticity = ngrt4n::Normal;
        break;
      case ngrt4n::K8sTerminatedError:
        criticity = ngrt4n::Critical;
        break;
      case ngrt4n::K8sPending:
      default:
        criticity = ngrt4n::Unknown;
        break;
    }
    return static_cast<ngrt4n::SeverityT>(criticity);
  }

  return ngrt4n::Unknown;
}


QString ngrt4n::getIconPath(int _severity)
{
  QString ipath("images/built-in/unknown.png");
  switch (static_cast<ngrt4n::SeverityT>(_severity)) {
    case ngrt4n::Normal:
      ipath = "images/built-in/normal.png";
      break;
    case ngrt4n::Minor:
      ipath = "images/built-in/minor.png";
      break;
    case ngrt4n::Major:
      ipath = "images/built-in/major.png";
      break;
    case ngrt4n::Critical:
      ipath = "images/built-in/critical.png";
      break;
    default:
      break;
  }
  return ipath;
}

bool ngrt4n::findNode(CoreDataT* coreData, const QString& nodeId, NodeListT::iterator& node)
{
  return findNode(coreData->bpnodes, coreData->cnodes, nodeId, node);
}

bool ngrt4n::findNode(NodeListT& bpnodes,
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

bool ngrt4n::findNode(const NodeListT& bpnodes,
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

bool ngrt4n::findNode(const NodeListT& nodes, const QString& nodeId, NodeListT::const_iterator& node)
{
  bool found = false;
  node = nodes.find(nodeId);
  if(node != nodes.end()) {
    found = true;
  }
  return found;
}


QString ngrt4n::sourceData2Json(const SourceT& src)
{
  return QString("{\"sid\":\"%1\","
                 "\"mon_type\":\"%2\","
                 "\"mon_url\":\"%3\","
                 "\"ls_addr\":\"%4\","
                 "\"ls_port\":\"%5\","
                 "\"auth\":\"%6\","
                 "\"verify_ssl_peer\":\"%7\""
                 "}").arg(src.id,
                          QString::number(src.mon_type),
                          src.mon_url,
                          src.ls_addr,
                          QString::number(src.ls_port),
                          src.auth,
                          QString::number(src.verify_ssl_peer));
}



void ngrt4n::setCheckOnError(int status, const QString& msg, CheckT& invalidCheck)
{
  invalidCheck.status = status;
  invalidCheck.last_state_change = humanTimeText("0");
  invalidCheck.host = "-";
  invalidCheck.check_command = "-";
  invalidCheck.alarm_msg = msg.toStdString();
}


/* return <[SourceId:]hostaddr, checkid> */
StringPairT ngrt4n::splitDataPointInfo(const QString& info)
{
  int pos = info.indexOf("/");
  QString second = ((pos == -1)? "ping" : info.mid(pos+1));
  return QPair<QString, QString>(info.left(pos), second);
}


/* return <source, hostaddr> */
StringPairT ngrt4n::splitSourceDataPointInfo(const QString& info)
{
  int pos = info.indexOf(":");
  QString first;
  if (pos == -1) {
    first = SRC_BASENAME%"0";
    return QPair<QString, QString>(first, info);
  }

  return QPair<QString, QString>(info.left(pos), info.mid(pos+1));
}


QString ngrt4n::getSourceIdFromStr(const QString& str)
{
  QString srcid = "";
  int pos = str.indexOf(":");
  if (pos != -1) {
    srcid = str.mid(0, pos);
  }
  return srcid;
}

QPair<bool, int> ngrt4n::checkSourceId(const QString &id)
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




QStringList ngrt4n::getAuthInfo(const QString& authString)
{
  QStringList authInfo = QStringList();
  int pos = authString.indexOf(":");
  if (pos != -1) {
    authInfo.push_back(authString.left(pos));
    authInfo.push_back(authString.mid(pos+1, -1));
  }
  return authInfo;
}



QString ngrt4n::basename(const QString& path)
{
  int lastSlash = path.lastIndexOf('/');

  if (lastSlash < 0)
    return path;

  return path.mid(lastSlash + 1, -1);
}


std::pair<int, QString> ngrt4n::importHostGroupAsBusinessView(const SourceT& srcInfo, const QString& filter, CoreDataT& cdata)
{
  const auto MONITOR_NAME = MonitorT::toString(srcInfo.mon_type);

  ChecksT checks;
  auto importOut = loadDataPoints(srcInfo, filter, checks);
  if (importOut.first != ngrt4n::RcSuccess) {
    return std::make_pair(-1, QObject::tr("%1: %2").arg(MONITOR_NAME, importOut.second));
  }

  // handle results
  if (checks.empty()) {
    return std::make_pair(-1, QObject::tr("Import from %1 (filter: %2): no item found").arg(MONITOR_NAME, filter));
  }

  cdata.clear();
  cdata.monitor = MonitorT::Any;

  NodeT rootService;
  rootService.id = ngrt4n::ROOT_ID;
  rootService.name = filter.isEmpty() ? QObject::tr("%1 Services").arg(MONITOR_NAME) : filter;
  rootService.type = NodeType::BusinessService;

  NodeT hostNode;
  NodeT triggerNode;
  hostNode.type = NodeType::BusinessService;
  triggerNode.type = NodeType::ITService;

  for (ChecksT::ConstIterator check = checks.begin(); check != checks.end(); ++check) {
    hostNode.parent = rootService.id;
    hostNode.name = hostNode.description = QString::fromStdString(check->host);
    hostNode.id = "";

    Q_FOREACH(QChar c, hostNode.name) { if (c.isLetterOrNumber()) { hostNode.id.append(c); } }

    QString checkId = QString::fromStdString(check->id);
    triggerNode.id = ngrt4n::generateId();
    triggerNode.parent = hostNode.id;
    triggerNode.name = checkId.startsWith(hostNode.name+"/") ? checkId.mid(hostNode.name.size() + 1) : checkId;
    triggerNode.child_nodes = QString::fromStdString("%1:%2").arg(srcInfo.id, checkId);

    NodeListIteratorT hostIterPos =  cdata.bpnodes.find(hostNode.id);
    if (hostIterPos != cdata.bpnodes.end()) {
      hostIterPos->child_nodes.append(ngrt4n::CHILD_Q_SEP).append(triggerNode.id);
    } else {
      hostNode.child_nodes = triggerNode.id;
      if (rootService.child_nodes.isEmpty()) {
        rootService.child_nodes = hostNode.id;
      } else {
        rootService.child_nodes.append(ngrt4n::CHILD_Q_SEP).append(hostNode.id);
      }
      cdata.bpnodes.insert(hostNode.id, hostNode);
    }
    cdata.cnodes.insert(triggerNode.id, triggerNode);
  }

  // finally insert the root node and update UI widgets
  cdata.bpnodes.insert(ngrt4n::ROOT_ID, rootService);

  return std::make_pair(0, "");
}


std::pair<int, QString> ngrt4n::loadDataPoints(const SourceT& srcInfo, const QString& filter, ChecksT& checks)
{
  // Nagios
  if (srcInfo.mon_type == MonitorT::Nagios) {
    int retcode = ngrt4n::RcGenericFailure;
    LsHelper handler(srcInfo.ls_addr, srcInfo.ls_port);
    if (handler.setupSocket() == 0 && handler.loadChecks(filter, checks) == 0) {
      retcode = ngrt4n::RcSuccess;
    }
    return std::make_pair(retcode, handler.lastError());
  }

  // Zabbix
  if (srcInfo.mon_type == MonitorT::Zabbix) {
    ZbxHelper handler;
    int retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::GroupFilter);
    if (checks.empty()) {
      retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::HostFilter);
    }
    return std::make_pair(retcode, handler.lastError());
  }


  // Zenoss
  if (srcInfo.mon_type == MonitorT::Zenoss) {
    ZnsHelper handler(srcInfo.mon_url);
    int retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::HostFilter);
    if (checks.empty()) {
      retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::GroupFilter);
    }
    return std::make_pair(retcode, handler.lastError());
  }


  // Pandora
  if (srcInfo.mon_type == MonitorT::Pandora) {
    PandoraHelper handler(srcInfo.mon_url);
    int retcode = handler.loadChecks(srcInfo, checks, filter);
    return std::make_pair(retcode, handler.lastError());
  }


  // OpManager
  if (srcInfo.mon_type == MonitorT::OpManager) {
    int retcode = ngrt4n::RcGenericFailure;
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
    return std::make_pair(retcode, handler.lastError());
  }

  // Kubernetes
  if (srcInfo.mon_type == MonitorT::Kubernetes) {
    K8sHelper k8s;
    return std::make_pair(ngrt4n::RcGenericFailure, "TODO import k8s data points");
  }

  return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("Cannot load data points for unknown data source: %1").arg(srcInfo.mon_type));
}


std::pair<int, QString> ngrt4n::saveViewDataToPath(const CoreDataT& cdata, const QString& path)
{
  if (! ngrt4n::MonitorSourceTypes.contains(MonitorT::toString(cdata.monitor))) {
    const_cast<CoreDataT&>(cdata).monitor = MonitorT::Any;
  }

  QFile file(path);
  if (! file.open(QIODevice::WriteOnly|QIODevice::Text)) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("Cannot open file: %1").arg(path));
  }


  QTextStream outStream(&file);
  outStream << "<?xml version=\"1.0\"?>\n";

  outStream << QString("<ServiceView compat=\"3.1\" monitor=\"%1\">\n").arg( QString::number(cdata.monitor) );

  for (auto&& bpnode: cdata.bpnodes) {
    outStream << generateNodeXml(bpnode);
  }

  for (auto&& cnode: cdata.cnodes) {
    if (! cnode.parent.isEmpty()) {
      outStream << generateNodeXml(cnode);
    }
  }

  outStream << "</ServiceView>\n";

  file.close();
  return std::make_pair(ngrt4n::RcSuccess, "");;
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


void ngrt4n::fixupDependencies(CoreDataT& cdata)
{
  // First clear all existing children for bpnodes
  for (auto& node: cdata.bpnodes) {
    node.child_nodes.clear();
  }

  // build dependencies for bpnodes
  for (const auto& node: cdata.bpnodes) {
    setParentChildDependency(node.id, node.parent, cdata.bpnodes);
  }

  // build dependencies for cnodes
  for (const auto& node: cdata.cnodes) {
    setParentChildDependency(node.id, node.parent, cdata.bpnodes);
  }
}


void ngrt4n::setParentChildDependency(const QString& childId, const QString& parentId, NodeListT& pnodes)
{
  auto parent_it = pnodes.find(parentId);
  if (parent_it == pnodes.end()) {
    return ;
  }

  if (parent_it->child_nodes.isEmpty()) {
    parent_it->child_nodes = childId;
  } else {
    parent_it->child_nodes += (QString(ngrt4n::CHILD_SEP.c_str()) % childId);
  }
}
