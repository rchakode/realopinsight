/*
 * SvNavigator.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 24-05-2012                                                  #
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
#include "MonitorBroker.hpp"
#include "ns.hpp"
#include "utilsClient.hpp"
#include "JsHelper.hpp"
#include "LsHelper.hpp"
#include <QScriptValueIterator>
#include <QNetworkCookieJar>
#include <QSystemTrayIcon>
#include <sstream>
#include <QStatusBar>
#include <QObject>
#include <zmq.h>
#include <iostream>
#include <algorithm>


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

DashboardBase::DashboardBase(const QString& _config)
  : m_config(utils::getAbsolutePath(_config)),
    m_cdata (new CoreDataT()),
    m_updateCounter(0),
    m_settings (new Settings()),
    m_showOnlyTroubles(false),
    m_errorState(false)
{
}

DashboardBase::~DashboardBase()
{
  delete m_cdata;
}

void DashboardBase::initialize(Preferences* preferencePtr)
{
  m_errorState = false;
  if (! m_config.isEmpty()) {
    Parser parser(m_config, m_cdata);
    connect(&parser, SIGNAL(errorOccurred(QString)), this, SLOT(handleErrorOccurred(QString)));
    if (parser.process(true)) {
      buildTree();
      buildMap();
      initSettings(preferencePtr);
    } else {
      m_errorState = true;
      m_lastError = parser.dotFile();
    }
  }
}

void DashboardBase::runMonitor()
{
  resetStatData();
  if (m_cdata->monitor == MonitorBroker::Auto) {
    for (SourceListT::Iterator src=m_sources.begin(), end=m_sources.end(); src!=end; ++src)
    {
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
  updateChart();
}

void DashboardBase::runMonitor(SourceT& src)
{
  prepareUpdate(src);
  openRpcSession(src);
  switch(src.mon_type) {
    case MonitorBroker::Zenoss:
    case MonitorBroker::Zabbix:
      requestZbxZnsData(src);
      break;
    case MonitorBroker::Nagios:
    default:
      src.use_ngrt4nd? runNgrt4ndUpdate(src) : runLivestatusUpdate(src);
      break;
  }
  finalizeUpdate(src);
}

void DashboardBase::runNgrt4ndUpdate(int srcId)
{
  SourceListT::Iterator src = m_sources.find(srcId);
  if (src != m_sources.end()) {
    runNgrt4ndUpdate(*src);
  } else {
    updateDashboardOnError(*src, tr("Undefined source (%1)").arg(utils::sourceId(srcId)));
  }
}

void DashboardBase::runNgrt4ndUpdate(const SourceT& src)
{
  CheckT invalidCheck;
  utils::setCheckOnError(MonitorBroker::Unknown, "", invalidCheck);

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
       end=m_cdata->cnodes.end(); cnode!=end; ++cnode)
  {
    if (cnode->child_nodes.isEmpty()) {
      cnode->severity = MonitorBroker::Unknown;
      m_cdata->check_status_count[cnode->severity]+=1;
      continue;
    }

    QPair<QString, QString> info = utils::splitSourceHostInfo(cnode->child_nodes);
    if (info.first != src.id) {
      continue;
    }

    // Retrieve data
    QString msg = src.auth%":"%info.second;
    src.d4n_handler->send(msg.toStdString());
    JsonHelper jsHelper(src.d4n_handler->recv());

    // Treat data
    qint32 ret = jsHelper.getProperty("return_code").toInt32();
    cnode->check.status = (ret!=0)? MonitorBroker::NagiosUnknown : jsHelper.getProperty("status").toInt32();
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
    updateDashboardOnError(*src, tr("Undefined source (%1)").arg(utils::sourceId(srcId)));
  }
}

void DashboardBase::runLivestatusUpdate(const SourceT& src)
{
  if (!src.ls_handler->isConnected()) {
    updateDashboardOnError(src, src.ls_handler->errorString());
    return;
  }

  CheckT invalidCheck;
  utils::setCheckOnError(MonitorBroker::Unknown, "", invalidCheck);

  QHashIterator<QString, QStringList> hostit(m_cdata->hosts);
  while (hostit.hasNext()) {
    hostit.next();
    QPair<QString, QString> info = utils::splitSourceHostInfo(hostit.key());

    if (info.first != src.id || ! src.ls_handler->loadHostData(info.second)) continue;

    Q_FOREACH (const QString& value, hostit.value()) {
      QString key;
      if (value != "ping") {
        key = ID_PATTERN.arg(info.second).arg(value);
      } else {
        key = info.second;
      }
      CheckListCstIterT chkit;
      if (src.ls_handler->findCheck(key, chkit)) {
        updateCNodes(*chkit, src);
      } else {
        invalidCheck.id = key.toStdString(); //FIXME: invalidCheck.id = key.toStdString();
        invalidCheck.alarm_msg = tr("Undefined service (%1)").arg(key).toStdString();
        updateCNodes(invalidCheck, src);
      }
    }
  }
}


void DashboardBase::resetStatData(void)
{
  m_cdata->check_status_count[MonitorBroker::Normal] = 0;
  m_cdata->check_status_count[MonitorBroker::Minor] = 0;
  m_cdata->check_status_count[MonitorBroker::Major] = 0;
  m_cdata->check_status_count[MonitorBroker::Critical] = 0;
  m_cdata->check_status_count[MonitorBroker::Unknown] = 0;
}


void DashboardBase::prepareUpdate(const SourceT& src)
{
  QString msg = QObject::tr("updating %1 (%2)...");
  switch(src.mon_type) {
    case MonitorBroker::Nagios:
      msg = msg.arg(src.id, QString("tcp://%1:%2").arg(src.ls_addr, QString::number(src.ls_port)));
      break;
    case MonitorBroker::Zabbix:
      msg = msg.arg(src.id, src.zbx_handler->getApiEndpoint());
      break;
    case MonitorBroker::Zenoss:
      msg = msg.arg(src.id, src.zns_handler->getApiBaseEndpoint());
      break;
    default:
      msg = msg.arg(src.id, "undefined source type");
      break;
  }
  Q_EMIT updateStatusBar(msg);
}

void DashboardBase::updateDashboard(const NodeT& _node)
{
  QString toolTip = utils::getNodeToolTip(_node);
  updateTree(_node, toolTip);
  updateMap(_node, toolTip);
  updateMsgConsole(_node);
  updateBpNode(_node.parent);
  updateEventFeeds(_node);
}

void DashboardBase::updateCNodes(const CheckT& check, const SourceT& src)
{
  for (NodeListIteratorT cnode=m_cdata->cnodes.begin(); cnode!=m_cdata->cnodes.end(); ++cnode)
  {
    if (cnode->child_nodes.toLower()==utils::realCheckId(src.id, QString::fromStdString(check.id)).toLower())
    {
      cnode->check = check;
      computeStatusInfo(*cnode, src);
      ++(m_cdata->check_status_count[cnode->severity]);
      updateDashboard(*cnode);
      cnode->monitored = true;
    }
  }
}

void DashboardBase::computeStatusInfo(NodeT& _node, const SourceT& src)
{
  QRegExp regexp;
  _node.severity = utils::computeSeverity(src.mon_type, _node.check.status);
  _node.prop_sev = utils::computeSeverity2Propagate(_node.severity, _node.sev_prule);
  _node.actual_msg = QString::fromStdString(_node.check.alarm_msg);

  if (_node.check.host == "-") {
    return;
  }

  if (m_cdata->monitor == MonitorBroker::Zabbix)
  {
    regexp.setPattern(ngrt4n::TAG_ZABBIX_HOSTNAME.c_str());
    _node.actual_msg.replace(regexp, _node.check.host.c_str());
    regexp.setPattern(ngrt4n::TAG_ZABBIX_HOSTNAME2.c_str());
    _node.actual_msg.replace(regexp, _node.check.host.c_str());
  }

  if (_node.severity == MonitorBroker::Normal)
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
  if (m_cdata->monitor == MonitorBroker::Nagios) {
    info = QString(_node.check.check_command.c_str()).split("!");
    if (info.length() >= 3) {
      regexp.setPattern(ngrt4n::TAG_THERESHOLD.c_str());
      _node.actual_msg.replace(regexp, info[1]);
      if (_node.severity == MonitorBroker::Major)
        _node.actual_msg.replace(regexp, info[2]);
    }
  }
}

void DashboardBase::updateBpNode(const QString& _nodeId)
{
  Criticity criticity(MonitorBroker::Normal);

  NodeListT::iterator node;
  if (! utils::findNode(m_cdata, _nodeId, node)) return;

  QStringList nodeIds = node->child_nodes.split(ngrt4n::CHILD_SEP.c_str());
  Q_FOREACH (const QString& nodeId, nodeIds) {
    NodeListT::iterator child;
    if (!utils::findNode(m_cdata, nodeId, child)) continue;
    Criticity cst(static_cast<MonitorBroker::SeverityT>(child->prop_sev));
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
  if (node->id != utils::ROOT_ID) {
    updateBpNode(node->parent);
    // FIXME: Q_EMIT hasToBeUpdate(node->parent);
    // Q_EMIT hasToBeUpdate(node->parent);
  }

}

void DashboardBase::processZbxReply(QNetworkReply* _reply, SourceT& src)
{
  _reply->deleteLater();
  QNetworkReply::NetworkError errcode = _reply->error();
  if (errcode != QNetworkReply::NoError) {
    processRpcError(errcode, src);
    return;
  }
  QString data = _reply->readAll();
  JsonHelper jsHelper(data);
  QString errmsg = jsHelper.getProperty("error").property("data").toString();
  if (errmsg.isEmpty()) errmsg = jsHelper.getProperty("error").property("message").toString();
  if (!errmsg.isEmpty()) {
    updateDashboardOnError(src, errmsg);
    return;
  }
  qint32 tid = jsHelper.getProperty("id").toInt32();
  switch(tid) {
    case ZbxHelper::Login: {
      QString auth = jsHelper.getProperty("result").toString();
      if (!auth.isEmpty()) {
        src.zbx_handler->setAuth(auth);
        src.zbx_handler->setIsLogged(true);
      }
      break;
    }
    case ZbxHelper::ApiVersion: {
      src.zbx_handler->setTrid(jsHelper.getProperty("result").toString());
      break;
    }
    case ZbxHelper::Trigger:
    case ZbxHelper::TriggerV18: {

      QScriptValueIterator trigger(jsHelper.getProperty("result"));
      while (trigger.hasNext()) {

        trigger.next();
        if (trigger.flags()&QScriptValue::SkipInEnumeration) continue;

        QScriptValue triggerData = trigger.value();
        QString triggerName = triggerData.property("description").toString();

        CheckT check;
        check.check_command = triggerName.toStdString();
        check.status = triggerData.property("value").toInt32();
        if (check.status == MonitorBroker::ZabbixClear) {
          check.alarm_msg = "OK ("+triggerName.toStdString()+")";
        } else {
          check.alarm_msg = triggerData.property("error").toString().toStdString();
          check.status = triggerData.property("priority").toInteger();
        }
        QString targetHost = "";
        QScriptValueIterator host(triggerData.property("hosts"));
        if (host.hasNext())
        {
          host.next(); if (host.flags()&QScriptValue::SkipInEnumeration) continue;
          QScriptValue hostData = host.value();
          targetHost = hostData.property("host").toString();
          check.host = targetHost.toStdString();
        }
        if (tid == ZbxHelper::TriggerV18) {
          check.last_state_change = triggerData.property("lastchange").toString().toStdString();
        } else {
          QScriptValueIterator item(triggerData.property("items"));
          if (item.hasNext()) {
            item.next(); if (item.flags()&QScriptValue::SkipInEnumeration) continue;
            QScriptValue itemData = item.value();
            check.last_state_change = itemData.property("lastclock").toString().toStdString();
          }
        }
        QString key = ID_PATTERN.arg(targetHost, triggerName);
        check.id = key.toStdString();
        updateCNodes(check, src);
      }
      break;
    }
    default :
      updateDashboardOnError(src, tr("Weird response received from the server"));
      qDebug() << data;
      break;
  }
}

void DashboardBase::processZnsReply(QNetworkReply* _reply, SourceT& src)
{
  _reply->deleteLater();

  if (_reply->error() != QNetworkReply::NoError) {
    processRpcError(_reply->error(), src);
    return;
  }

  QVariant cookiesContainer = _reply->header(QNetworkRequest::SetCookieHeader);
  QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(cookiesContainer);
  QString data = _reply->readAll();
  if (data.endsWith("submitted=true")) {
    src.zns_handler->cookieJar()->setCookiesFromUrl(cookies, src.zns_handler->getApiBaseEndpoint());
    src.zns_handler->setIsLogged(true);
  } else {
    JsonHelper jsonHelper(data);
    qint32 tid = jsonHelper.getProperty("tid").toInt32();
    QScriptValue result = jsonHelper.getProperty("result");
    bool reqSucceed = result.property("success").toBool();
    if (!reqSucceed) {
      updateDashboardOnError(src, tr("Authentication failed: %1").arg(result.property("msg").toString()));
      return;
    }
    if (tid == ZnsHelper::Device) {

      QScriptValueIterator devices(result.property("devices"));

      while (devices.hasNext()) {

        devices.next();
        if (devices.flags()&QScriptValue::SkipInEnumeration) continue;

        QScriptValue ditem = devices.value();
        QString duid = ditem.property("uid").toString();
        QNetworkReply* reply = src.zns_handler->postRequest(ZnsHelper::Component,
                                                            ZnsHelper::ReqPatterns[ZnsHelper::Component]
            .arg(duid, QString::number(ZnsHelper::Component))
            .toAscii());
        processZnsReply(reply, src);

        QString did = utils::realCheckId(src.id, ditem.property("name").toString());
        if (m_cdata->hosts[did].contains("ping", Qt::CaseInsensitive))
        {
          reply = src.zns_handler->postRequest(ZnsHelper::Device,
                                               ZnsHelper::ReqPatterns[ZnsHelper::DeviceInfo]
              .arg(duid, QString::number(ZnsHelper::DeviceInfo))
              .toAscii());
          processZnsReply(reply, src);
        }
      }
    } else {
      CheckT check;
      if (tid == ZnsHelper::Component) {
        QScriptValueIterator components(result.property("data"));
        while (components.hasNext()) {
          components.next(); if (components.flags()&QScriptValue::SkipInEnumeration) continue;
          QScriptValue citem = components.value();
          QString cname = citem.property("name").toString();
          QScriptValue device = citem.property("device");
          QString duid = device.property("uid").toString();
          QString dname = ZnsHelper::getDeviceName(duid);
          QString chkid = ID_PATTERN.arg(dname, cname);
          check.id = chkid.toStdString();
          check.host = dname.toStdString();
          check.last_state_change = utils::convertToTimet(device.property("lastChanged").toString(),
                                                          "yyyy/MM/dd hh:mm:ss");
          QString severity =citem.property("severity").toString();
          if (!severity.compare("clear", Qt::CaseInsensitive)) {
            check.status = MonitorBroker::ZenossClear;
            check.alarm_msg = tr("The %1 component is Up").arg(cname).toStdString();
          } else {
            check.status = citem.property("failSeverity").toInt32();
            check.alarm_msg = citem.property("status").toString().toStdString();
          }
          updateCNodes(check, src);
        }
      } else if (tid == ZnsHelper::DeviceInfo) {
        QScriptValue devInfo(result.property("data"));
        QString dname = devInfo.property("name").toString();
        check.id = check.host = dname.toStdString();
        check.status = devInfo.property("status").toBool();
        check.last_state_change = utils::convertToTimet(devInfo.property("lastChanged").toString(),
                                                        "yyyy/MM/dd hh:mm:ss");
        if (check.status) {
          check.status = MonitorBroker::ZenossClear;
          check.alarm_msg = tr("The host '%1' is Up").arg(dname).toStdString();
        } else {
          check.status = MonitorBroker::ZenossCritical;
          check.alarm_msg = tr("The host '%1' is Down").arg(dname).toStdString();
        }
        updateCNodes(check, src);
      } else {
        updateDashboardOnError(src, tr("Weird response received from the server"));
      }
    }
  }
}

QStringList DashboardBase::getAuthInfo(int srcId)
{
  SourceListT::Iterator source = m_sources.find(srcId);
  if (source != m_sources.end()) {
    return getAuthInfo(source->auth);
  }
  return QStringList();
}

QStringList DashboardBase::getAuthInfo(const QString& authString)
{
  QStringList authInfo;
  int pos = authString.indexOf(":");
  if (pos != -1) {
    authInfo.push_back(authString.left(pos));
    authInfo.push_back(authString.mid(pos+1, -1));
  }
  return authInfo;
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
      case MonitorBroker::Zabbix:
        src->zbx_handler->setIsLogged(false);
        break;
      case MonitorBroker::Zenoss:
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
  QStringList authParams = getAuthInfo(src.auth);
  if (authParams.size() != 2 && src.mon_type != MonitorBroker::Nagios) {
    updateDashboardOnError(src, tr("Invalid authentication chain!\n"
                                   "Must follow the pattern login:password"));
    return;
  }

  QUrl znsUrlParams;
  switch(src.mon_type) {
    case MonitorBroker::Nagios:
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
    case MonitorBroker::Zabbix: {
      src.zbx_handler->setBaseUrl(src.mon_url);
      authParams.push_back(QString::number(ZbxHelper::Login));
      src.zbx_handler->setSslConfig(src.verify_ssl_peer);
      QNetworkReply* reply = src.zbx_handler->postRequest(ZbxHelper::Login, authParams);
      processZbxReply(reply, src);
      if (src.zbx_handler->getIsLogged()) {
        // The get API version
        QStringList params;
        params.push_back(QString::number(ZbxHelper::ApiVersion));
        src.zbx_handler->setSslConfig(src.verify_ssl_peer);
        reply = src.zbx_handler->postRequest(ZbxHelper::ApiVersion, params);
        processZbxReply(reply, src);
      }
      break;
    }
    case MonitorBroker::Zenoss: {
      src.zns_handler->setBaseUrl(src.mon_url);
      znsUrlParams.addQueryItem("__ac_name", authParams[0]);
      znsUrlParams.addQueryItem("__ac_password", authParams[1]);
      znsUrlParams.addQueryItem("submitted", "true");
      znsUrlParams.addQueryItem("came_from", src.zns_handler->getApiContextEndpoint());
      src.zns_handler->setSslConfig(src.verify_ssl_peer);
      QNetworkReply* reply = src.zns_handler->postRequest(ZnsHelper::Login, znsUrlParams.encodedQuery());
      processZnsReply(reply, src);
    }
      break;
    default:
      break;
  }
}


void DashboardBase::requestZbxZnsData(SourceT& src)
{
  switch(src.mon_type) {
    case MonitorBroker::Zabbix: {
      if (src.zbx_handler->getIsLogged()) {
        int trid = src.zbx_handler->getTrid();
        Q_FOREACH (const QString& hitem, m_cdata->hosts.keys()) {

          StringPairT info = utils::splitSourceHostInfo(hitem);

          if (info.first != src.id) continue;

          QStringList params;
          params.push_back(info.second);
          params.push_back(QString::number(trid));
          QNetworkReply* reply = src.zbx_handler->postRequest(trid, params);
          processZbxReply(reply, src);
        }
      }
      break;
    }
    case MonitorBroker::Zenoss: {
      if (src.zns_handler->getIsLogged()) {
        src.zns_handler->setRouterEndpoint(ZnsHelper::Device);
        Q_FOREACH (const QString& hitem, m_cdata->hosts.keys()) {

          StringPairT info = utils::splitSourceHostInfo(hitem);

          if (info.first != src.id) continue;

          QNetworkReply* reply = src.zns_handler->postRequest(ZnsHelper::Device,
                                                              ZnsHelper::ReqPatterns[ZnsHelper::Device]
              .arg(info.second, QString::number(ZnsHelper::Device))
              .toAscii());
          processZnsReply(reply, src);
        }
      }
      break;
    }
    default:
      break;
  }
}

void DashboardBase::processRpcError(QNetworkReply::NetworkError _code, const SourceT& src)
{
  QString apiUrl = "";
  if (src.mon_type == MonitorBroker::Zabbix) {
    apiUrl = src.zbx_handler->getApiEndpoint();
  } else if (src.mon_type == MonitorBroker::Zenoss) {
    apiUrl =  src.zns_handler->getRequestEndpoint();
  }

  switch (_code) {
    case QNetworkReply::RemoteHostClosedError:
      updateDashboardOnError(src, tr("The connection has been closed by the remote host"));
      break;
    case QNetworkReply::HostNotFoundError:
      updateDashboardOnError(src, tr("Host not found"));
      break;
    case QNetworkReply::ConnectionRefusedError:
      updateDashboardOnError(src, tr("Connection refused"));
      break;
    case QNetworkReply::SslHandshakeFailedError:
      updateDashboardOnError(src, tr("SSL Handshake failed"));
      break;
    case QNetworkReply::TimeoutError:
      updateDashboardOnError(src, tr("Timeout exceeded"));
      break;
    default:
      updateDashboardOnError(src, SERVICE_OFFLINE_MSG.arg(apiUrl, tr("error %1").arg(_code)));
  }
}


void DashboardBase::updateDashboardOnError(const SourceT& src, const QString& msg)
{
  if (!msg.isEmpty()) {
    Q_EMIT updateStatusBar(msg);
  }
  for (NodeListIteratorT cnode = m_cdata->cnodes.begin(); cnode != m_cdata->cnodes.end(); ++cnode)
  {
    StringPairT info = utils::splitSourceHostInfo(cnode->child_nodes);
    if (info.first != src.id) continue;

    utils::setCheckOnError(-1, msg, cnode->check);
    computeStatusInfo(*cnode, src);
    updateDashboard(*cnode);
    m_cdata->check_status_count[MonitorBroker::Unknown]+=1;
    cnode->monitored = true;
  }
}


void DashboardBase::initSettings(Preferences* preferencePtr)
{
  m_sources.clear();
  SourceT src;
  for (auto id=m_cdata->sources.begin(),end = m_cdata->sources.end(); id!=end; ++id)
  {
    QPair<bool, int> srcinfo = utils::checkSourceId(*id);
    if (srcinfo.first) {
      if (preferencePtr->isSetSource(srcinfo.second) &&
          m_settings->loadSource(*id, src) &&
          allocSourceHandler(src)) {
        m_sources.insert(srcinfo.second, src);
      } else {
        src.id = *id;
        updateDashboardOnError(src, tr("%1 is not set").arg(*id));
      }
    } else {
      Q_EMIT errorOccurred(tr("Could not handle this source (%1)").arg(*id));
    }
  }
  resetInterval();
  computeFirstSrcIndex();
  Q_EMIT settingsLoaded();
}

bool DashboardBase::allocSourceHandler(SourceT& src)
{
  bool allocated = false;
  if (src.mon_type == MonitorBroker::Auto) {
    src.mon_type = m_cdata->monitor;
  }

  switch (src.mon_type) {
    case MonitorBroker::Nagios:
      if (src.use_ngrt4nd) {
        QString uri = QString("tcp://%1:%2").arg(src.ls_addr, QString::number(src.ls_port));
        src.d4n_handler = std::make_shared<ZmqSocket>(uri.toStdString(), ZMQ_REQ);
      } else {
        src.ls_handler = std::make_shared<LsHelper>(src.ls_addr, src.ls_port);
      }
      allocated = true;
      break;
    case MonitorBroker::Zabbix:
      src.zbx_handler = std::make_shared<ZbxHelper>();
      allocated = true;
      break;
    case MonitorBroker::Zenoss:
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
  Q_FOREACH (const qint8& id, ids) {
    SourceT newsrc;
    m_settings->loadSource(id, newsrc);
    SourceListT::Iterator olddata = m_sources.find(id);
    if (olddata != m_sources.end()) {
      switch (olddata->mon_type) {
        case MonitorBroker::Nagios:
          if (olddata->use_ngrt4nd) {
            olddata->ls_handler.reset();
          } else {
            olddata->d4n_handler.reset();
          }
          break;
        case MonitorBroker::Zabbix:
          olddata->zbx_handler.reset();
          break;
        case MonitorBroker::Zenoss:
          olddata->zns_handler.reset();
          break;
        default:
          Q_EMIT errorOccurred(tr("Unknown monitor type (%1)").arg(olddata->mon_type));
          break;
      }
    }
    allocSourceHandler(newsrc);
    m_sources[id] = newsrc;
    runMonitor(newsrc);
  }
  Q_EMIT updateSourceUrl();
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
                                            utils::severityText(_node.severity),
                                            CalcRules::label(_node.sev_crule),
                                            PropRules::label(_node.sev_prule));
  if (_node.type == NodeType::AlarmNode)
  {
    toolTip += ALARM_SPECIFIC_TIP_PATTERN.arg(QString::fromStdString(_node.check.host).replace("\n", " "),
                                              _node.child_nodes,
                                              QString::fromStdString(_node.check.alarm_msg),
                                              _node.actual_msg);
  }
  return toolTip;
}


void DashboardBase::finalizeUpdate(const SourceT& src)
{
  if (m_cdata->cnodes.isEmpty()) return;

  for (NodeListIteratorT cnode = m_cdata->cnodes.begin(),
       end = m_cdata->cnodes.end(); cnode != end; ++cnode)
  {
    if (! cnode->monitored &&
        cnode->child_nodes.toLower()==utils::realCheckId(src.id,
                                                         QString::fromStdString(cnode->check.id)).toLower())
    {
      utils::setCheckOnError(MonitorBroker::Unknown,
                             tr("Undefined service (%1)").arg(cnode->child_nodes),
                             cnode->check);
      computeStatusInfo(*cnode, src);
      m_cdata->check_status_count[cnode->severity]+=1;
      updateDashboard(*cnode);
    }
    cnode->monitored = false;
  }
}

void DashboardBase::resetInterval()
{
  m_interval = 1000 * m_settings->updateInterval();
  timerIntervalChanged(m_interval);
}
