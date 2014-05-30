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
#include "global.hpp"
#include "utilsCore.hpp"
#include "JsonHelper.hpp"
#include "LsHelper.hpp"
#include <QScriptValueIterator>
#include <QNetworkCookieJar>
#include <QSystemTrayIcon>
#include <sstream>
#include <QStatusBar>
#include <QObject>
#include <QNetworkCookie>
#include <iostream>
#include <algorithm>
#include <zmq.h>
#include <cassert>


#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#   include <QUrlQuery>
#endif


namespace {
const QString DEFAULT_TIP_PATTERN(QObject::tr("Service: %1\nDescription: %2\nSeverity: %3\n   Calc. Rule: %4\n   Prop. Rule: %5"));
const QString ALARM_SPECIFIC_TIP_PATTERN(QObject::tr("\nTarget Host: %6\nData Point: %7\nRaw Output: %8\nOther Details: %9"));
const QString SERVICE_OFFLINE_MSG(QObject::tr("Failed to connect to %1 (%2)"));
const QString JSON_ERROR_MSG("{\"return_code\": \"-1\", \"message\": \""%SERVICE_OFFLINE_MSG%"\"}");
}

StringMapT DashboardBase::propRules() {
  StringMapT map;
  map.insert(PropRules::label(PropRules::Unchanged), PropRules::toString(PropRules::Unchanged));
  map.insert(PropRules::label(PropRules::Decreased), PropRules::toString(PropRules::Decreased));
  map.insert(PropRules::label(PropRules::Increased), PropRules::toString(PropRules::Increased));
  return map;
}

StringMapT DashboardBase::calcRules() {
  StringMapT map;
  map.insert(CalcRules::label(CalcRules::HighCriticity),
             CalcRules::toString(CalcRules::HighCriticity));
  map.insert(CalcRules::label(CalcRules::WeightedCriticity),
             CalcRules::toString(CalcRules::WeightedCriticity));
  return map;
}

DashboardBase::DashboardBase(const QString& descriptionFile)
  : m_descriptionFile(ngrt4n::getAbsolutePath(descriptionFile)),
    m_cdata (new CoreDataT()),
    m_updateCounter(0),
    m_showOnlyTroubles(false),
    m_lastErrorState(false)
{
  resetStatData();
}

DashboardBase::~DashboardBase()
{
  delete m_cdata;
}

void DashboardBase::initialize(Preferences* preferencePtr)
{
  m_preferences = preferencePtr;
  m_lastErrorState = false;
  if (! m_descriptionFile.isEmpty()) {
    Parser parser(m_descriptionFile, m_cdata);
    connect(&parser, SIGNAL(errorOccurred(QString)), this, SLOT(handleErrorOccurred(QString)));
    if (parser.process(true)) {
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
  resetStatData();
  if (m_cdata->monitor == ngrt4n::Auto) {
    for (SourceListT::Iterator src = m_sources.begin(), end = m_sources.end(); src!=end; ++src) {
      runMonitor(*src);
    }
  } else {
    SourceListT::Iterator src = m_sources.find(0);
    if (src != m_sources.end()) {
      runMonitor(*src);
    } else {
      Q_EMIT errorOccurred(tr("The default source is not yet set"));
    }
  }
  ++m_updateCounter;
}

void DashboardBase::runMonitor(SourceT& src)
{
  prepareUpdate(src);
  openRpcSession(src);
  switch(src.mon_type) {
  case ngrt4n::Zenoss:
    Q_FOREACH (const QString& hitem, m_cdata->hosts.keys()) {
      StringPairT info = ngrt4n::splitSourceHostInfo(hitem);
      if (info.first != src.id) continue;
      ChecksT checks;
      if (src.zns_handler->loadChecks(src, info.second, checks) == 0) {
        updateCNodesWithChecks(checks, src);
      } else {
        updateDashboardOnError(src, src.zns_handler->lastError());
      }
    }
    break;
  case ngrt4n::Zabbix:
    Q_FOREACH (const QString& hostItem, m_cdata->hosts.keys()) {
      StringPairT info = ngrt4n::splitSourceHostInfo(hostItem);

      if (info.first != src.id) continue;

      ChecksT checks;
      if (src.zbx_handler->loadChecks(src, info.second, checks) == 0) {
        updateCNodesWithChecks(checks, src);
      } else {
        updateDashboardOnError(src, src.zbx_handler->lastError());
      }
    }
    break;
  case ngrt4n::Nagios:
  default:
    src.use_ngrt4nd? runNgrt4ndUpdate(src) : runLivestatusUpdate(src);
    break;
  }
  updateChart();
  finalizeUpdate(src);
}

void DashboardBase::runNgrt4ndUpdate(int srcId)
{
  SourceListT::Iterator src = m_sources.find(srcId);
  if (src != m_sources.end()) {
    runNgrt4ndUpdate(*src);
  } else {
    updateDashboardOnError(*src, tr("Undefined source (%1)").arg(ngrt4n::sourceId(srcId)));
  }
}

void DashboardBase::runNgrt4ndUpdate(const SourceT& src)
{
  CheckT invalidCheck;
  ngrt4n::setCheckOnError(ngrt4n::Unknown, "", invalidCheck);

  /* Check if the handler is connected */
  if (src.d4n_handler->isConnected()) {
    if (src.d4n_handler->getServerSerial() < 110) {
      QString errmsg = tr("The server serial %1 is not supported").arg(src.d4n_handler->getServerSerial());
      updateDashboardOnError(src, errmsg);
      return;
    }
  } else {
    QString errmsg(src.d4n_handler->getErrorMsg().c_str());
    updateDashboardOnError(src, errmsg);
    return;
  }

  /* Now start doing the job */
  for (NodeListIteratorT cnode=m_cdata->cnodes.begin(),
       end=m_cdata->cnodes.end();
       cnode!=end;
       ++cnode)
  {
    if (cnode->child_nodes.isEmpty()) {
      cnode->severity = ngrt4n::Unknown;
      m_cdata->check_status_count[cnode->severity]+=1;
      continue;
    }

    QPair<QString, QString> info = ngrt4n::splitSourceHostInfo(cnode->child_nodes);
    if (info.first != src.id) {
      continue;
    }

    // Retrieve data
    QString msg = src.auth%":"%info.second;
    src.d4n_handler->send(msg.toStdString());
    JsonHelper jsHelper(src.d4n_handler->recv().c_str());

    // Treat data
    qint32 ret = jsHelper.getProperty("return_code").toInt32();
    cnode->check.status = (ret!=0)? ngrt4n::NagiosUnknown : jsHelper.getProperty("status").toInt32();
    cnode->check.host = jsHelper.getProperty("host").toString().toStdString();
    cnode->check.last_state_change = jsHelper.getProperty("lastchange").toString().toStdString();
    cnode->check.check_command = jsHelper.getProperty("command").toString().toStdString();
    cnode->check.alarm_msg = jsHelper.getProperty("message").toString().toStdString();

    computeStatusInfo(*cnode, src);
    updateDashboard(*cnode);
    m_cdata->check_status_count[cnode->severity]+=1;
    cnode->monitored = true;
  }
}


void DashboardBase::runLivestatusUpdate(int srcId)
{
  SourceListT::Iterator src = m_sources.find(srcId);
  if (src != m_sources.end()) {
    runLivestatusUpdate(*src);
  } else {
    updateDashboardOnError(*src, tr("Undefined source (%1)").arg(ngrt4n::sourceId(srcId)));
  }
}

void DashboardBase::runLivestatusUpdate(const SourceT& src)
{
  if (!src.ls_handler->isConnected()) {
    updateDashboardOnError(src, src.ls_handler->errorString());
    return;
  }

  CheckT invalidCheck;
  ngrt4n::setCheckOnError(ngrt4n::Unknown, "", invalidCheck);

  QHashIterator<QString, QStringList> hostit(m_cdata->hosts);
  while (hostit.hasNext()) {
    hostit.next();
    QPair<QString, QString> info = ngrt4n::splitSourceHostInfo(hostit.key());

    if (info.first != src.id || ! src.ls_handler->fecthHostChecks(info.second)) continue;

    Q_FOREACH (const QString& value, hostit.value()) {
      QString key;
      if (value != "ping") {
        key = ID_PATTERN.arg(info.second).arg(value);
      } else {
        key = info.second;
      }
      CheckListCstIterT chkit;
      if (src.ls_handler->findCheck(key, chkit)) {
        updateCNodesWithCheck(*chkit, src);
      } else {
        invalidCheck.id = key.toStdString(); //FIXME: invalidCheck.id = key.toStdString();
        invalidCheck.alarm_msg = tr("Undefined service (%1)").arg(key).toStdString();
        updateCNodesWithCheck(invalidCheck, src);
      }
    }
  }
}


void DashboardBase::resetStatData(void)
{
  m_cdata->check_status_count[ngrt4n::Normal] = 0;
  m_cdata->check_status_count[ngrt4n::Minor] = 0;
  m_cdata->check_status_count[ngrt4n::Major] = 0;
  m_cdata->check_status_count[ngrt4n::Critical] = 0;
  m_cdata->check_status_count[ngrt4n::Unknown] = m_cdata->cnodes.size();
}


void DashboardBase::prepareUpdate(const SourceT& src)
{
  QString msg = QObject::tr("updating %1 (%2)...");
  switch(src.mon_type) {
  case ngrt4n::Nagios:
    msg = msg.arg(src.id, QString("tcp://%1:%2").arg(src.ls_addr, QString::number(src.ls_port)));
    break;
  case ngrt4n::Zabbix:
  case ngrt4n::Zenoss:
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
  QString nodeToolTip = ngrt4n::getNodeToolTip(_node);
  updateTree(_node, nodeToolTip);
  updateMap(_node, nodeToolTip);
  updateChart();
  updateMsgConsole(_node);
  updateBpNode(_node.parent);
  updateEventFeeds(_node);
}

void DashboardBase::updateCNodesWithCheck(const CheckT& check, const SourceT& src)
{
  for (NodeListIteratorT cnode=m_cdata->cnodes.begin(), end = m_cdata->cnodes.end(); cnode!=end; ++cnode) {
    if (cnode->child_nodes.toLower() == ngrt4n::realCheckId(src.id, QString::fromStdString(check.id)).toLower()) {
      cnode->check = check;
      computeStatusInfo(*cnode, src);
      ++(m_cdata->check_status_count[cnode->severity]);
      updateDashboard(*cnode);
      cnode->monitored = true;
    }
  }
}

void DashboardBase::updateCNodesWithChecks(const ChecksT& checks, const SourceT& src)
{
  for (ChecksT::const_iterator check=checks.begin(), end = checks.end(); check!=end; ++check) {
    updateCNodesWithCheck(check->second, src);
  }
}

void DashboardBase::computeStatusInfo(NodeT& _node, const SourceT& src)
{
  QRegExp regexp;
  _node.severity = ngrt4n::computeSeverity(src.mon_type, _node.check.status);
  _node.prop_sev = ngrt4n::computeSeverity2Propagate(_node.severity, _node.sev_prule);
  _node.actual_msg = QString::fromStdString(_node.check.alarm_msg);

  if (_node.check.host == "-") {
    return;
  }

  if (m_cdata->monitor == ngrt4n::Zabbix)
  {
    regexp.setPattern(ngrt4n::TAG_ZABBIX_HOSTNAME.c_str());
    _node.actual_msg.replace(regexp, _node.check.host.c_str());
    regexp.setPattern(ngrt4n::TAG_ZABBIX_HOSTNAME2.c_str());
    _node.actual_msg.replace(regexp, _node.check.host.c_str());
  }

  if (_node.severity == ngrt4n::Normal)
  {
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
  if (m_cdata->monitor == ngrt4n::Nagios) {
    info = QString(_node.check.check_command.c_str()).split("!");
    if (info.length() >= 3) {
      regexp.setPattern(ngrt4n::TAG_THERESHOLD.c_str());
      _node.actual_msg.replace(regexp, info[1]);
      if (_node.severity == ngrt4n::Major)
        _node.actual_msg.replace(regexp, info[2]);
    }
  }
}

void DashboardBase::updateBpNode(const QString& _nodeId)
{
  Criticity criticity(ngrt4n::Normal);

  NodeListT::iterator node;
  if (! ngrt4n::findNode(m_cdata, _nodeId, node)) return;

  QStringList nodeIds = node->child_nodes.split(ngrt4n::CHILD_SEP.c_str());
  Q_FOREACH (const QString& nodeId, nodeIds) {
    NodeListT::iterator child;
    if (!ngrt4n::findNode(m_cdata, nodeId, child)) continue;
    Criticity cst(static_cast<ngrt4n::SeverityT>(child->prop_sev));
    if (node->sev_crule == CalcRules::WeightedCriticity) {
      criticity = criticity / cst;
    } else {
      criticity = criticity * cst;
    }
  }

  node->severity = criticity.getValue();
  switch(node->sev_prule) {
  case PropRules::Increased:
    node->prop_sev = (criticity++).getValue();
    break;
  case PropRules::Decreased:
    node->prop_sev = (criticity--).getValue();
    break;
  case PropRules::Unchanged:
  default:
    node->prop_sev = node->severity;
    break;
  }

  QString toolTip = getNodeToolTip(*node);
  updateMap(*node, toolTip);
  updateTree(*node, toolTip);
  if (node->id != ngrt4n::ROOT_ID && ! node->parent.isEmpty()) {
    updateBpNode(node->parent);
  }
}

QStringList DashboardBase::getAuthInfo(int srcId)
{
  SourceListT::Iterator source = m_sources.find(srcId);
  if (source != m_sources.end()) {
    return ngrt4n::getAuthInfo(source->auth);
  }
  return QStringList();
}


void DashboardBase::openRpcSessions(void)
{
  for (SourceListT::Iterator src = m_sources.begin(), end = m_sources.end();
       src != end; ++src) {
    openRpcSession(*src);
  }
}

void DashboardBase::openRpcSession(int srcId)
{
  SourceListT::Iterator src = m_sources.find(srcId);
  if (src != m_sources.end()) {
    switch (src->mon_type) {
    case ngrt4n::Zabbix:
      src->zbx_handler->setIsLogged(false);
      break;
    case ngrt4n::Zenoss:
      src->zns_handler->setIsLogged(false);
      break;
    default:
      break;
    }
  }
  openRpcSession(*src);
}


void DashboardBase::openRpcSession(SourceT& src)
{
  QStringList authParams = ngrt4n::getAuthInfo(src.auth);
  if (authParams.size() != 2 && src.mon_type != ngrt4n::Nagios) {
    updateDashboardOnError(src, tr("Invalid authentication chain!\n"
                                   "Must follow the pattern login:password"));
    return;
  }

  switch(src.mon_type) {
  case ngrt4n::Nagios:
    if (src.use_ngrt4nd) {
      if (src.d4n_handler->isConnected())
        src.d4n_handler->disconnectFromService();

      if(src.d4n_handler->connect())
        src.d4n_handler->makeHandShake();
    } else {
      if (src.ls_handler->isConnected())
        src.ls_handler->disconnectFromService();

      src.ls_handler->connectToService();
    }
    break;
  case ngrt4n::Zabbix:
    src.zbx_handler->openSession(src);
    break;
  case ngrt4n::Zenoss:
    src.zns_handler->openSession(src);
    break;
  default:
    break;
  }
}


void DashboardBase::updateDashboardOnError(const SourceT& src, const QString& msg)
{
  if (! msg.isEmpty()) {
    Q_EMIT updateStatusBar(msg);
  }
  for (NodeListIteratorT cnode = m_cdata->cnodes.begin(); cnode != m_cdata->cnodes.end(); ++cnode) {

    StringPairT info = ngrt4n::splitSourceHostInfo(cnode->child_nodes);
    if (info.first != src.id) continue;

    ngrt4n::setCheckOnError(-1, msg, cnode->check);
    computeStatusInfo(*cnode, src);
    ++(m_cdata->check_status_count[ngrt4n::Unknown]);  //FIXME: reset stat data
    cnode->monitored = true;
    updateDashboard(*cnode);
  }
}

void DashboardBase::initSettings(Preferences* preferencePtr)
{
  m_sources.clear();
  SourceT src;
  for (auto id=m_cdata->sources.begin(), end = m_cdata->sources.end(); id != end; ++id)
  {
    QPair<bool, int> srcinfo = ngrt4n::checkSourceId(*id);
    if (srcinfo.first) {
      if (preferencePtr->isSetSource(srcinfo.second)) {
        if (preferencePtr->loadSource(*id, src) && allocSourceHandler(src)) {
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

bool DashboardBase::allocSourceHandler(SourceT& src)
{
  bool allocated = false;
  if (m_cdata->monitor != ngrt4n::Auto) {
    src.mon_type = m_cdata->monitor;
  }

  switch (src.mon_type) {
  case ngrt4n::Nagios:
    if (src.use_ngrt4nd) {
      QString uri = QString("tcp://%1:%2").arg(src.ls_addr, QString::number(src.ls_port));
      src.d4n_handler = std::make_shared<ZmqSocket>(uri.toStdString(), ZMQ_REQ);
    } else {
      src.ls_handler = std::make_shared<LsHelper>(src.ls_addr, src.ls_port);
    }
    allocated = true;
    break;
  case ngrt4n::Zabbix:
    src.zbx_handler = std::make_shared<ZbxHelper>();
    allocated = true;
    break;
  case ngrt4n::Zenoss:
    src.zns_handler = std::make_shared<ZnsHelper>();
    allocated = true;
    break;
  default:
    updateDashboardOnError(src, tr("%1: undefined monitor (%2)").arg(src.id, src.mon_type));
    allocated = false;
    break;
  }

  return allocated;
}

void DashboardBase::handleSourceSettingsChanged(QList<qint8> ids)
{
  if (ids.size() > 0)
  {
    Q_FOREACH (const qint8& id, ids) {
      SourceT newsrc;
      m_preferences->loadSource(id, newsrc);
      SourceListT::Iterator olddata = m_sources.find(id);
      if (olddata != m_sources.end()) {
        switch (olddata->mon_type) {
        case ngrt4n::Nagios:
          if (olddata->use_ngrt4nd) {
            olddata->ls_handler.reset();
          } else {
            olddata->d4n_handler.reset();
          }
          break;
        case ngrt4n::Zabbix:
          olddata->zbx_handler.reset();
          break;
        case ngrt4n::Zenoss:
          olddata->zns_handler.reset();
          break;
        default:
          Q_EMIT errorOccurred(tr("Unknown monitor type (%1)").arg(olddata->mon_type));
          break;
        }
      }
      allocSourceHandler(newsrc);
      m_sources[id] = newsrc;
    }
    runMonitor();
    Q_EMIT updateSourceUrl();
  }
}

void DashboardBase::computeFirstSrcIndex(void)
{
  m_firstSrcIndex = -1;
  if (! m_cdata->sources.isEmpty()) {
    SourceListT::Iterator cur = m_sources.begin();
    SourceListT::Iterator end = m_sources.end();
    while (cur != end && ! m_cdata->sources.contains(cur->id)) ++cur;
    if (cur != end) {
      m_firstSrcIndex = extractSourceIndex(cur->id);
    }
  }
}

QString DashboardBase::getNodeToolTip(const NodeT& _node)
{
  QString toolTip = DEFAULT_TIP_PATTERN.arg(_node.name,
                                            const_cast<QString&>(_node.description).replace("\n", " "),
                                            ngrt4n::severityText(_node.severity),
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


void DashboardBase::finalizeUpdate(const SourceT& src)
{
  for (NodeListIteratorT cnode=m_cdata->cnodes.begin(), end=m_cdata->cnodes.end(); cnode!=end; ++cnode) {
    QString prefix = QString("%1:").arg(src.id);
    if (! cnode->monitored && cnode->child_nodes.startsWith(prefix, Qt::CaseInsensitive)) {
      ngrt4n::setCheckOnError(ngrt4n::Unset, tr("Undefined service (%1)").arg(cnode->child_nodes), cnode->check);
      computeStatusInfo(*cnode, src);
      m_cdata->check_status_count[cnode->severity]+=1;
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
  NodeListT::iterator root = m_cdata->bpnodes.find(ngrt4n::ROOT_ID);
  assert(root != m_cdata->bpnodes.end());
  return *root;
}
