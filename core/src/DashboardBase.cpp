/*
 * DashboardBase.cpp
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


#include "DashboardBase.hpp"
#include "utilsCore.hpp"
#include "LsHelper.hpp"
#include "StatusAggregator.hpp"
#include "K8sHelper.hpp"
#include <QNetworkCookieJar>
#include <sstream>
#include <QObject>
#include <QNetworkCookie>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <regex>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#   include <QUrlQuery>
#endif


namespace {
  const QString SERVICE_OFFLINE_MSG(QObject::tr("Failed to connect to %1 (%2)"));
  const QString JSON_ERROR_MSG("{\"return_code\": \"-1\", \"message\": \""%SERVICE_OFFLINE_MSG%"\"}");
} //namespace

StringMapT DashboardBase::propRules() {
  PropRules unchanged(PropRules::Unchanged);
  PropRules decreased(PropRules::Decreased);
  PropRules increased(PropRules::Increased);
  
  StringMapT map;
  map.insert(unchanged.toString(), unchanged.data());
  map.insert(decreased.toString(), decreased.data());
  map.insert(increased.toString(), increased.data());
  return map;
}

StringMapT DashboardBase::calcRules() {
  CalcRules worst(CalcRules::Worst);
  CalcRules average(CalcRules::Average);
  CalcRules weighted(CalcRules::WeightedAverageWithThresholds);
  
  StringMapT map;
  map.insert(worst.toString(), worst.data());
  map.insert(average.toString(), average.data());
  map.insert(weighted.toString(), weighted.data());
  return map;
}

DashboardBase::DashboardBase(DbSession* dbSession)
  : m_dbSession(dbSession),
    m_timerId(-1)
{
  resetStatData();
}

DashboardBase::~DashboardBase()
{
}

std::pair<int, QString> DashboardBase::initialize(const QString& vfile)
{
  if (vfile.isEmpty()) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("empty description file"));
  }

  if (! m_dbSession) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("initialize: db session not initialized"));
  }

  Parser parser{&m_cdata, Parser::ParsingModeDashboard, m_dbSession};
  auto parseOut = parser.parse(vfile);
  if (parseOut.first != ngrt4n::RcSuccess) {
    return std::make_pair(parseOut.first, parseOut.second);
  }

  auto loadDsOut = loadDataSources();
  if (loadDsOut.first != ngrt4n::RcSuccess) {
    return loadDsOut;
  }

  int rc = parser.processRenderingData();
  if (rc != ngrt4n::RcSuccess) {
    return std::make_pair(rc, parser.lastErrorMsg());
  }

  return std::make_pair(ngrt4n::RcSuccess, "");
}

std::pair<int, QString> DashboardBase::updateAllNodesStatus(void)
{
  if (! m_dbSession) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("updateAllNodesStatus: db session not initialized"));
  }

  resetStatData();
  for (const auto& sid: m_cdata.sources) {
    auto src = m_sources.constFind(sid);
    if (src != std::cend(m_sources)) {
      signalUpdateProcessing(*src);
      if (m_cdata.monitor != MonitorT::Any) {
        runDynamicViewByGroupUpdate(*src);
      } else {
        runGenericViewUpdate(*src);
      }
      finalizeUpdate(*src);
    } else {
       SourceT unknownSrc;
       unknownSrc.id = sid;
       updateDashboardOnError(unknownSrc, QObject::tr("source not set %1").arg(sid));
    }
  }

  computeBpNodeStatus(ngrt4n::ROOT_ID, m_dbSession);

  updateChart();

  return std::make_pair(ngrt4n::RcSuccess, QObject::tr(""));
}


void DashboardBase::signalUpdateProcessing(const SourceT& src)
{
  QString monitorName = MonitorT::toString(src.mon_type);
  if (src.mon_type == MonitorT::Nagios) {
    Q_EMIT updateMessageChanged(QObject::tr("querying %1/%2 => %3:%4...").arg(monitorName, src.id, src.ls_addr, QString::number(src.ls_port)).toStdString());
  } else {
    Q_EMIT updateMessageChanged(QObject::tr("querying %1/%2 => %3)...").arg(monitorName, src.id, src.mon_url).toStdString());
  }
}

void DashboardBase::runDynamicViewByGroupUpdate(const SourceT& sinfo)
{
  if (sinfo.mon_type == MonitorT::Kubernetes) {
    CoreDataT newCData;
    auto viewLoaded = K8sHelper(sinfo.mon_url, sinfo.verify_ssl_peer, sinfo.auth).loadNamespaceView(rootNode().name, newCData);
    if (viewLoaded.second != ngrt4n::RcSuccess) {
      updateDashboardOnError(sinfo, viewLoaded.first);
      return ;
    }

    for (const auto& newCNode: newCData.cnodes) {
      auto cnode = m_cdata.cnodes.find(newCNode.id);
      if (cnode != m_cdata.cnodes.end()) { // pod may disappear due to restart, but a notification should be displayed in event feed.
        cnode->check = newCNode.check;
        updateNodeStatusInfo(*cnode, sinfo);
        updateDashboard(*cnode);
        cnode->monitored = true;
      }
    }
  } else {
    ChecksT checks;
    auto importResult = ngrt4n::loadDataItems(sinfo, rootNode().name, checks);
    if (importResult.first != ngrt4n::RcSuccess) {
      updateDashboardOnError(sinfo, importResult.second);
    } else {
      updateCNodesWithChecks(checks, sinfo);
    }
  }
}


void DashboardBase::runGenericViewUpdate(const SourceT& srcInfo)
{
  for (const auto& hitem: m_cdata.hosts.keys()) { //FIXME: avoid iteration for Pandora FMS => all modules are fetched once
    StringPairT info = ngrt4n::splitSourceDataPointInfo(hitem);
    if (info.first != srcInfo.id) {
      continue;
    }
    ChecksT checks;
    auto importResult = ngrt4n::loadDataItems(srcInfo, info.second, checks);
    if (importResult.first != ngrt4n::RcSuccess) {
      updateDashboardOnError(srcInfo, importResult.second);
    } else {
      updateCNodesWithChecks(checks, srcInfo);
      break;
    }
  }
}


void DashboardBase::resetStatData(void)
{
  m_cdata.check_status_count[ngrt4n::Normal] = 0;
  m_cdata.check_status_count[ngrt4n::Minor] = 0;
  m_cdata.check_status_count[ngrt4n::Major] = 0;
  m_cdata.check_status_count[ngrt4n::Critical] = 0;
  m_cdata.check_status_count[ngrt4n::Unknown] = m_cdata.cnodes.size();
}


void DashboardBase::updateDashboard(const NodeT& _node)
{
  QString tooltip = _node.toString();
  updateTree(_node, tooltip);
  updateMap(_node, tooltip);
  updateMsgConsole(_node);
  updateEventFeeds(_node);
}

void DashboardBase::updateCNodesWithCheck(const CheckT& check, const SourceT& src)
{
  for (auto& cnode: m_cdata.cnodes) {
    if (cnode.child_nodes.toLower() != ngrt4n::realCheckId(src.id, QString::fromStdString(check.id)).toLower()) {
      continue;
    }
    cnode.check = check;
    updateNodeStatusInfo(cnode, src);
    updateDashboard(cnode);
    cnode.monitored = true;
  }
}

void DashboardBase::updateCNodesWithChecks(const ChecksT& checks, const SourceT& src)
{
  for (const auto& check: checks) {
    updateCNodesWithCheck(check, src);
  }
}

void DashboardBase::updateNodeStatusInfo(NodeT& _node, const SourceT& src)
{
  QRegExp regexp;
  _node.sev = ngrt4n::severityFromProbeStatus(src.mon_type, _node.check.status);
  _node.sev_prop = StatusAggregator::propagate(_node.sev, _node.sev_prule);
  _node.actual_msg = QString::fromStdString(_node.check.alarm_msg);
  
  if (_node.check.host == "-") {
    return;
  }
  
  if (m_cdata.monitor == MonitorT::Zabbix) {
    regexp.setPattern(ngrt4n::TAG_ZABBIX_HOSTNAME.c_str());
    _node.actual_msg.replace(regexp, _node.check.host.c_str());
    regexp.setPattern(ngrt4n::TAG_ZABBIX_HOSTNAME2.c_str());
    _node.actual_msg.replace(regexp, _node.check.host.c_str());
  }
  
  if (_node.sev == ngrt4n::Normal) {
    if (_node.notification_msg.isEmpty()) {
      return ;
    } else {
      _node.actual_msg = _node.notification_msg;
    }
  } else if (_node.alarm_msg.isEmpty())  {
    return ;
  } else {
    _node.actual_msg = _node.alarm_msg;
  }
  
  regexp.setPattern(ngrt4n::TAG_HOSTNAME.c_str());
  _node.actual_msg.replace(regexp, _node.check.host.c_str());
  auto info = QString(_node.check.id.c_str()).split("/");
  
  if (info.length() > 1) {
    regexp.setPattern(ngrt4n::TAG_CHECK.c_str());
    _node.actual_msg.replace(regexp, info[1]);
  }
  
  if (m_cdata.monitor == MonitorT::Nagios) {
    info = QString(_node.check.check_command.c_str()).split("!");
    if (info.length() >= 3) {
      regexp.setPattern(ngrt4n::TAG_THERESHOLD.c_str());
      _node.actual_msg.replace(regexp, info[1]);
      if (_node.sev == ngrt4n::Major)
        _node.actual_msg.replace(regexp, info[2]);
    }
  }
}

ngrt4n::AggregatedSeverityT DashboardBase::computeBpNodeStatus(const QString& _nodeId, DbSession* p_dbSession)
{
  ngrt4n::AggregatedSeverityT status2Propagate;

  NodeListT::iterator node;
  if (! ngrt4n::findNode(&m_cdata, _nodeId, node)) {
    status2Propagate.sev = ngrt4n::Unknown;
    status2Propagate.weight = ngrt4n::WEIGHT_UNIT;
    return status2Propagate;
  }
  
  status2Propagate.weight = node->weight;
  
  if (node->child_nodes.isEmpty()) {
    status2Propagate.sev = ngrt4n::Unknown;

    return status2Propagate;
  }
  
  // if IT service handle it directly
  if (node->type == NodeType::ITService) {
    status2Propagate.sev = node->sev_prop;

    return status2Propagate;
  }

  // if external service handle it through last status fetched from database
  if (node->type == NodeType::ExternalService) {
    constexpr long intervalDurationSec = 10 * 60;
    long toDate = std::time(nullptr);
    long fromDate = toDate - intervalDurationSec;
    PlatformMappedStatusHistoryT pfStatusMap;

    node->check.host = "-";
    node->check.host_groups = "-";
    node->check.check_command = "-";
    node->check.last_state_change = std::to_string(toDate);

    auto listOfExternalViews = node->child_nodes.toStdString();
    int rc = p_dbSession->listStatusHistory(pfStatusMap, listOfExternalViews, fromDate, toDate);
    if (rc > 0) {
      node->sev = pfStatusMap[listOfExternalViews].back().status;
      node->actual_msg = QObject::tr("external service - %1").arg(node->child_nodes);
    } else {
      node->sev = ngrt4n::Unknown;
      status2Propagate.sev = ngrt4n::Unknown;
      node->actual_msg = QObject::tr("external service - %1 - no status found in last %2 minute(s)")
                         .arg(node->child_nodes
                              .arg(intervalDurationSec / 60));
    }

    status2Propagate.sev = StatusAggregator::propagate(node->sev, node->sev_prule);
    updateDashboard(*node);

    return status2Propagate;
  }


  StatusAggregator severityAggregator;
  for (auto&& childId: node->child_nodes.split(ngrt4n::CHILD_Q_SEP)) {
    status2Propagate = computeBpNodeStatus(childId, p_dbSession);
    severityAggregator.addSeverity(status2Propagate.sev, status2Propagate.weight);
  }
  
  node->sev = severityAggregator.aggregate(node->sev_crule, node->thresholdLimits);
  node->sev_prop = severityAggregator.propagate(node->sev, node->sev_prule);
  node->actual_msg = severityAggregator.toDetailsString();
  
  status2Propagate.sev = node->sev_prop;
  status2Propagate.weight = node->weight;

  QString tooltip = node->toString();
  updateMap(*node, tooltip);
  updateTree(*node, tooltip);
  
  return status2Propagate;
}


void DashboardBase::updateDashboardOnError(const SourceT& src, const QString& msg)
{
  if (! msg.isEmpty()) {
    Q_EMIT updateMessageChanged(msg.toStdString());
  }

  for (auto& cnode: m_cdata.cnodes) {
    StringPairT info = ngrt4n::splitSourceDataPointInfo(cnode.child_nodes);
    if (info.first != src.id) continue;
    ngrt4n::setCheckOnError(-1, msg, cnode.check);
    updateNodeStatusInfo(cnode, src);
    cnode.monitored = true;
    updateDashboard(cnode);
  }
}

std::pair<int, QString> DashboardBase::loadDataSources(void)
{
  if (! m_dbSession) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("loadDataSources: db session not initialized"));
  }
  m_sources = m_dbSession->listSources(MonitorT::Any);
  return std::make_pair(ngrt4n::RcSuccess, QObject::tr(""));
}

void DashboardBase::finalizeUpdate(const SourceT& src)
{
  for (auto& cnode: m_cdata.cnodes) {

    if (cnode.monitored) {
      cnode.monitored = false;
      continue;
    }

    switch (src.mon_type) {
      case MonitorT::Any:
        if (std::regex_match(cnode.child_nodes.toStdString(), std::regex(QString("%1:.+").arg(src.id).toStdString()))) {
          ngrt4n::setCheckOnError(ngrt4n::Unset, tr("Undefined service (%1)").arg(cnode.child_nodes), cnode.check);
          updateNodeStatusInfo(cnode, src);
          updateDashboard(cnode);
        }
        break;
      case MonitorT::Kubernetes:
        cnode.sev = ngrt4n::Critical;
        cnode.check.status = ngrt4n::K8sFailed;
        cnode.check.alarm_msg = QObject::tr("Pod %1 seems to no longer exist").arg(cnode.child_nodes).toStdString();
        updateNodeStatusInfo(cnode, src);
        updateDashboard(cnode);
        break;
      default:
        cnode.sev = ngrt4n::Critical;
        cnode.check.status = ngrt4n::Unset;
        cnode.check.alarm_msg = QObject::tr("Item %1 seems to no longer exist").arg(cnode.child_nodes).toStdString();
        updateNodeStatusInfo(cnode, src);
        updateDashboard(cnode);
        break;
    }

  }
}

NodeT DashboardBase::rootNode(void)
{
  NodeListT::iterator root = m_cdata.bpnodes.find(ngrt4n::ROOT_ID);
  if (root != m_cdata.bpnodes.end()) {
    return *root;
  }
  return NodeT();
}



int DashboardBase::extractStatsData(CheckStatusCountT& statsData)
{
  for (const auto& node : m_cdata.cnodes) {
    switch (node.sev) {
      case ngrt4n::Normal:
        ++statsData[ngrt4n::Normal];
        break;
      case ngrt4n::Minor:
        ++statsData[ngrt4n::Minor];
        break;
      case ngrt4n::Major:
        ++statsData[ngrt4n::Major];
        break;
      case ngrt4n::Critical:
        ++statsData[ngrt4n::Critical];
        break;
      case ngrt4n::Unknown:
      default:
        ++statsData[ngrt4n::Unknown];
        break;
    }
  }

  return m_cdata.cnodes.size();
}
