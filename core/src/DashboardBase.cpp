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
#include "JsonHelper.hpp"
#include "LsHelper.hpp"
#include "PandoraHelper.hpp"
#include "OpManagerHelper.hpp"
#include "StatusAggregator.hpp"
#include "DescriptionFileFactoryUtils.hpp"
#include <QScriptValueIterator>
#include <QNetworkCookieJar>
#include <QSystemTrayIcon>
#include <sstream>
#include <QStatusBar>
#include <QObject>
#include <QNetworkCookie>
#include <iostream>
#include <algorithm>
#include <cassert>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#   include <QUrlQuery>
#endif

#ifdef REALOPINSIGHT_ENABLE_ZMQ
#include "ZmqSocket.hpp"
#include <zmq.h>
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

DashboardBase::DashboardBase(const QString& descriptionFile)
  : m_descriptionFile(ngrt4n::getAbsolutePath(descriptionFile)),
    m_timerId(-1),
    m_updateCounter(0),
    m_showOnlyTroubles(false),
    m_lastErrorState(false)
{
  resetStatData();
}

DashboardBase::~DashboardBase()
{
}

void DashboardBase::initialize(Preferences* preferencePtr)
{
  m_preferences = preferencePtr;
  m_lastErrorState = false;
  if (! m_descriptionFile.isEmpty()) {
    Parser parser(m_descriptionFile, &m_cdata);
    connect(&parser, SIGNAL(errorOccurred(QString)), this, SLOT(handleErrorOccurred(QString)));
    if (parser.process(Parser::ParsingModeDashboard)) {
      buildTree();
      buildMap();
      initSettings(m_preferences);
    } else {
      m_lastErrorState = true;
      m_lastErrorMsg = parser.lastErrorMsg();
    }
  }
}

void DashboardBase::runMonitor()
{
  Q_EMIT updateInprogress();
  resetStatData();

  if (m_cdata.monitor == MonitorT::Auto) {
    for (SourceListT::Iterator src = m_sources.begin(), end = m_sources.end(); src!=end; ++src) { runMonitor(*src);}
  } else {
    SourceListT::Iterator src = m_sources.find(0);
    if (src != m_sources.end()) {
      runMonitor(*src);
    } else {
      Q_EMIT errorOccurred(tr("The default source is not yet set"));
    }
  }
  computeNodeSeverity(rootNode().id);
  updateChart();
  ++m_updateCounter;
  Q_EMIT updateFinished();
}

void DashboardBase::runMonitor(SourceT& src)
{
  prepareUpdate(src);
  if (src.mon_type == MonitorT::Nagios && src.use_ngrt4nd) {
#ifdef REALOPINSIGHT_ENABLE_ZMQ
    runNgrt4ndUpdate(src);
#else
    updateDashboardOnError(src, QObject::tr("This version is compiled without ngrt4nd support"));
#endif
  } else {
    runDataSourceUpdate(src);
  }

  finalizeUpdate(src);
}

#ifdef REALOPINSIGHT_ENABLE_ZMQ

void DashboardBase::runNgrt4ndUpdate(const SourceT& src)
{
  CheckT invalidCheck;
  ngrt4n::setCheckOnError(ngrt4n::Unknown, "", invalidCheck);
  
  // Check if the handler is connected
  std::string ngrt4ndUri = QString("tcp://%1:%2").arg(src.ls_addr, QString::number(src.ls_port)).toStdString();
  ZmqSocket d4nBroker(ngrt4ndUri, ZMQ_REQ);
  
  d4nBroker.setupSocket();
  d4nBroker.makeHandShake();
  if (! d4nBroker.isReady()) {
    updateDashboardOnError(src, d4nBroker.lastError());
    return;
  }
  
  if (d4nBroker.getServerSerial() < 110) {
    QString errmsg = tr("The server serial %1 is not supported").arg(d4nBroker.getServerSerial());
    updateDashboardOnError(src, errmsg);
    return;
  }
  
  // Now start doing the job
  for (NodeListIteratorT cnode = m_cdata.cnodes.begin(), end=m_cdata.cnodes.end(); cnode != end; ++cnode) {
    if (cnode->child_nodes.isEmpty()) {
      cnode->sev = ngrt4n::Unknown;
    } else {
      StringPairT sourceDataPointInfo = ngrt4n::splitSourceDataPointInfo(cnode->child_nodes);
      if (sourceDataPointInfo.first == src.id) {
        // Retrieve data
        QString requestData = QString("%1:%2").arg(src.auth, sourceDataPointInfo.second);
        d4nBroker.send(requestData.toStdString());
        JsonHelper jsHelper(d4nBroker.recv().c_str());
        
        // Treat data
        qint32 ret = jsHelper.getProperty("return_code").toInt32();
        cnode->check.status = (ret!=0)? ngrt4n::NagiosUnknown : jsHelper.getProperty("status").toInt32();
        cnode->check.host = jsHelper.getProperty("host").toString().toStdString();
        cnode->check.last_state_change = jsHelper.getProperty("lastchange").toString().toStdString();
        cnode->check.check_command = jsHelper.getProperty("command").toString().toStdString();
        cnode->check.alarm_msg = jsHelper.getProperty("message").toString().toStdString();
        
        computeStatusInfo(*cnode, src);
        updateDashboard(*cnode);
        cnode->monitored = true;
      }
    }
  }
}
#endif //#ifndef REALOPINSIGHT_ENABLE_ZMQ

void DashboardBase::runDataSourceUpdate(const SourceT& srcInfo)
{
  //FIXME: avoid iteration for Pandora FMS => all modules are fetched once
  Q_FOREACH (const QString& hitem, m_cdata.hosts.keys()) {
    StringPairT info = ngrt4n::splitSourceDataPointInfo(hitem);
    if (info.first == srcInfo.id) {
      QString errorMsg;
      ChecksT checks;
      if (ngrt4n::importMonitorItemAsDataPoints(srcInfo, info.second, checks, errorMsg) == 0) {
        updateCNodesWithChecks(checks, srcInfo);
      } else {
        updateDashboardOnError(srcInfo, errorMsg);
        break;
      }
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


void DashboardBase::prepareUpdate(const SourceT& src)
{
  QString msg = QObject::tr("updating %1 (%2)...");
  switch(src.mon_type) {
    case MonitorT::Nagios:
      msg = msg.arg(src.id, QString("tcp://%1:%2").arg(src.ls_addr, QString::number(src.ls_port)));
      break;
    case MonitorT::Zabbix:
    case MonitorT::Zenoss:
    case MonitorT::Pandora:
    case MonitorT::OpManager:
      msg = msg.arg(src.id, src.mon_url);
      break;
    default:
      msg = msg.arg(src.id, "undefined source type");
      break;
  }
  Q_EMIT updateStatusBar(msg);
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
  for (NodeListIteratorT cnode = m_cdata.cnodes.begin(), end = m_cdata.cnodes.end(); cnode!=end; ++cnode) {
    if (cnode->child_nodes.toLower() == ngrt4n::realCheckId(src.id, QString::fromStdString(check.id)).toLower()) {
      cnode->check = check;
      computeStatusInfo(*cnode, src);
      updateDashboard(*cnode);
      cnode->monitored = true;
    }
  }
}

void DashboardBase::updateCNodesWithChecks(const ChecksT& checks, const SourceT& src)
{
  for (ChecksT::const_iterator check=checks.begin(), end = checks.end(); check!=end; ++check) {
    updateCNodesWithCheck(*check, src);
  }
}

void DashboardBase::computeStatusInfo(NodeT& _node, const SourceT& src)
{
  QRegExp regexp;
  _node.sev = ngrt4n::severityFromProbeStatus(src.mon_type, _node.check.status);
  _node.sev_prop = StatusAggregator::propagate(_node.sev, _node.sev_prule);
  _node.actual_msg = QString::fromStdString(_node.check.alarm_msg);
  
  if (_node.check.host == "-")
    return;
  
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

ngrt4n::AggregatedSeverityT DashboardBase::computeNodeSeverity(const QString& _nodeId)
{
  ngrt4n::AggregatedSeverityT result;
  
  NodeListT::iterator node;
  if (! ngrt4n::findNode(&m_cdata, _nodeId, node)) {
    result.sev = ngrt4n::Unknown;
    result.weight = ngrt4n::WEIGHT_UNIT;
    return result;
  }
  
  result.weight = node->weight;
  
  if (node->child_nodes.isEmpty()) {
    result.sev = ngrt4n::Unknown;
    return result;
  }
  
  if (node->type == NodeType::ITService) {
    result.sev = node->sev_prop;
    return result;
  }
  
  StatusAggregator severityAggregator;
  
  Q_FOREACH(const QString& childId, node->child_nodes.split(ngrt4n::CHILD_SEP.c_str())) {
    result = computeNodeSeverity(childId);
    severityAggregator.addSeverity(result.sev, result.weight);
  }
  
  node->sev = severityAggregator.aggregate(node->sev_crule, node->thresholdLimits);
  node->sev_prop = severityAggregator.propagate(node->sev, node->sev_prule);
  
  result.sev = node->sev_prop;
  result.weight = node->weight;
  node->actual_msg = severityAggregator.toDetailsString();
  QString tooltip = node->toString();
  updateMap(*node, tooltip);
  updateTree(*node, tooltip);
  
  return result;
}

QStringList DashboardBase::getAuthInfo(int srcId)
{
  SourceListT::Iterator source = m_sources.find(srcId);
  if (source != m_sources.end()) {
    return ngrt4n::getAuthInfo(source->auth);
  }
  return QStringList();
}


void DashboardBase::updateDashboardOnError(const SourceT& src, const QString& msg)
{
  if (! msg.isEmpty()) {
    Q_EMIT updateStatusBar(msg);
  }
  for (NodeListIteratorT cnode = m_cdata.cnodes.begin(); cnode != m_cdata.cnodes.end(); ++cnode) {
    StringPairT info = ngrt4n::splitSourceDataPointInfo(cnode->child_nodes);
    if (info.first != src.id) continue;
    
    ngrt4n::setCheckOnError(-1, msg, cnode->check);
    computeStatusInfo(*cnode, src);
    cnode->monitored = true;
    updateDashboard(*cnode);
  }
}

void DashboardBase::initSettings(Preferences* preferencePtr)
{
  m_sources.clear();
  SourceT src;
  for (auto id = m_cdata.sources.begin(), end = m_cdata.sources.end(); id != end; ++id) {
    QPair<bool, int> srcinfo = ngrt4n::checkSourceId(*id);
    if (srcinfo.first) {
      if (preferencePtr->isSetSource(srcinfo.second)) {
        
        if (preferencePtr->loadSource(*id, src)) {
          checkStandaloneSourceType(src);
          m_sources.insert(srcinfo.second, src);
        } else {
          src.id = *id;
          updateDashboardOnError(src, tr("Cannot set handler for %1").arg(*id));
        }
      } else {
        src.id = *id;
        updateDashboardOnError(src, tr("%1 is not set").arg(*id));
      }
    } else {
      Q_EMIT errorOccurred(tr("Cannot handle source (%1)").arg(*id));
    }
  }
  resetInterval();
  computeFirstSrcIndex();
  Q_EMIT settingsLoaded();
}

void DashboardBase::checkStandaloneSourceType(SourceT& src)
{
  if (m_cdata.monitor != MonitorT::Auto) {
    src.mon_type = m_cdata.monitor;
  }
}

void DashboardBase::handleSourceSettingsChanged(QList<qint8> ids)
{
  if (! ids.isEmpty()) {
    Q_FOREACH (const qint8& id, ids) {
      SourceT newsrc;
      m_preferences->loadSource(id, newsrc);
      checkStandaloneSourceType(newsrc);
      SourceListT::Iterator olddata = m_sources.find(id);
      if (olddata != m_sources.end()) {
        m_sources.erase(olddata);
      }
      m_sources.insert(id, newsrc);
    }
    runMonitor();
    Q_EMIT updateSourceUrl();
  }
}

void DashboardBase::computeFirstSrcIndex(void)
{
  m_firstSrcIndex = -1;
  if (! m_cdata.sources.isEmpty()) {
    SourceListT::Iterator cur = m_sources.begin();
    SourceListT::Iterator end = m_sources.end();
    while (cur != end && ! m_cdata.sources.contains(cur->id)) ++cur;
    if (cur != end) {
      m_firstSrcIndex = extractSourceIndex(cur->id);
    }
  }
}

void DashboardBase::finalizeUpdate(const SourceT& src)
{
  for (NodeListIteratorT cnode = m_cdata.cnodes.begin(), end = m_cdata.cnodes.end(); cnode != end; ++cnode) {
    QString srcPrefix = QString("%1:").arg(src.id);
    if (! cnode->monitored && cnode->child_nodes.startsWith(srcPrefix, Qt::CaseInsensitive)) {
      ngrt4n::setCheckOnError(ngrt4n::Unset,
                              tr("Undefined service (%1)").arg(cnode->child_nodes),
                              cnode->check);
      computeStatusInfo(*cnode, src);
      updateDashboard(*cnode);
    }
    cnode->monitored = false;
  }
}

void DashboardBase::resetInterval()
{
  m_interval = 1000 * m_preferences->updateInterval();
  timerIntervalChanged(m_interval);
}

NodeT DashboardBase::rootNode(void)
{
  NodeListT::iterator root = m_cdata.bpnodes.find(ngrt4n::ROOT_ID);
  assert(root != m_cdata.bpnodes.end());
  return *root;
}



void DashboardBase::extractStatsData(CheckStatusCountT& statsData, qint32& count)
{
  count = m_cdata.cnodes.size();
  for (NodeListT::ConstIterator node = m_cdata.cnodes.begin(), end = m_cdata.cnodes.end();
       node != end; ++ node) {
    switch (node->sev) {
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
}
