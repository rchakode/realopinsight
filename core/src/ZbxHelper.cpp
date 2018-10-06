/*
* ZbxHelper.cpp
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

#include "ZbxHelper.hpp"
#include "utilsCore.hpp"
#include "JsonHelper.hpp"
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QDebug>
#include <QSslConfiguration>


const RequestListT ZbxHelper::ReqPatterns = ZbxHelper::requestsPatterns();


ZbxHelper::ZbxHelper(const QString & baseUrl)
  : QNetworkAccessManager(),
    m_apiUri(baseUrl%ZBX_API_CONTEXT),
    m_getTriggersByHostOrGroupApiVersion(-1),
    m_isLogged(false)
{
  m_reqHandler.setRawHeader("Content-Type", "application/json");
  m_reqHandler.setUrl(QUrl(m_apiUri));

}

ZbxHelper::~ZbxHelper()
{
}

RequestListT
ZbxHelper::requestsPatterns()
{
  RequestListT patterns;
  patterns[GetLogin] = "{\"jsonrpc\": \"2.0\", \
                       \"auth\": null, \
                       \"method\": \"user.login\", \
                       \"params\": {\"user\": \"%1\",\"password\": \"%2\"}, \
                       \"id\": %9}";
  patterns[GetApiVersion] = "{\"jsonrpc\": \"2.0\", \
                            \"method\": \"apiinfo.version\", \
                            \"params\": [], \
                            \"id\": %9}";
  patterns[GetTriggersByHostOrGroup] = "{\"jsonrpc\": \"2.0\", \
                                       \"auth\": \"%1\", \
                                       \"method\": \"trigger.get\", \
                                       \"params\": { \
                                       \"active\": true, \
                                       \%2 \
                                       \"selectGroups\": [\"name\"], \
                                        \"selectHosts\": [\"host\"], \
                                        \"selectItems\": [\"key_\",\"name\",\"lastclock\"], \
                                        \"output\": [\"description\",\"value\",\"error\",\"comments\",\"priority\"], \
                                        \"limit\": -1}, \
                                        \"id\": %9}";
  patterns[GetTriggersByHostOrGroupV18] = "{\"jsonrpc\": \"2.0\", \
                                          \"auth\": \"%1\", \
                                          \"method\": \"trigger.get\", \
                                          \"params\": { \
                                          \"active\": true, \
                                          \%2 \
                                          \"select_hosts\": [\"host\"], \
                                           \"output\":  \"extend\", \
                                           \"limit\": -1}, \
                                           \"id\": %9}";
  patterns[GetTriggersByIds] = "{\"jsonrpc\": \"2.0\", \
                               \"auth\": \"%1\", \
                               \"method\": \"trigger.get\", \
                               \"params\": { \
                               \"active\": true, \
                               \"triggerids\": %2, \
                               \"selectGroups\": [\"name\"], \
                               \"selectHosts\": [\"host\"], \
                               \"selectItems\": [\"key_\",\"name\",\"lastclock\"], \
                               \"output\": [\"triggerid\",\"description\",\"value\",\"error\",\"comments\",\"priority\"], \
                               \"limit\": -1}, \
                               \"id\": %9}";
  patterns[GetITServices] = "{\"jsonrpc\": \"2.0\", \
                            \"auth\": \"%1\", \
                            \"method\": \"service.get\", \
                            \"params\": {\"output\": \"extend\", \"selectDependencies\": \"extend\"}, \
                            \"id\": %9}";

  return patterns;
}


bool ZbxHelper::checkLogin(void)
{
  if (! m_isLogged && openSession() != ngrt4n::RcSuccess) {
    m_isLogged = false;
  }

  return m_isLogged;
}

int
ZbxHelper::postRequest(qint32 reqId, const QStringList& params)
{
  QString request = "";
  if (reqId == GetLogin || reqId == GetApiVersion) {
    request = ReqPatterns[reqId] ;
  } else {
    request = ReqPatterns[reqId].arg(m_auth);
  }

  Q_FOREACH(const QString& myparam, params) {
    request = request.arg(myparam);
  }

  QNetworkReply* reply = QNetworkAccessManager::post(m_reqHandler, ngrt4n::toByteArray(request));
  setSslReplyErrorHandlingOptions(reply);

  connect(reply, SIGNAL(finished()), &m_evlHandler, SLOT(quit()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));

  m_evlHandler.exec();

  if (! reply) {
    return ngrt4n::RcGenericFailure;
  }

  return parseReply(reply);
}


void
ZbxHelper::setSslPeerVerification(bool verifyPeer)
{
  if (verifyPeer) {
    m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
  } else {
    m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
  }
}

void
ZbxHelper::setApiVersion(const QString& apiv)
{
  qint32 vnum = apiv.mid(0, 3).remove(".").toInt();
  if (vnum < 14) {
    m_getTriggersByHostOrGroupApiVersion = GetTriggersByHostOrGroupV18;
  } else {
    m_getTriggersByHostOrGroupApiVersion = GetTriggersByHostOrGroup;
  }
}

int
ZbxHelper::parseReply(QNetworkReply* reply)
{
  reply->deleteLater();

  // check for error in network communication
  if (reply->error() != QNetworkReply::NoError) {
    m_lastError = tr("%1 (%2)").arg(reply->errorString(), reply->url().toString()) ;
    return ngrt4n::RcGenericFailure;
  }

  // now read data
  QString data = QString(reply->readAll());
  m_replyJsonData.setData( data ) ;
  return ngrt4n::RcSuccess;
}

bool
ZbxHelper::checkBackendSuccessfulResult(void)
{
  QString errMsg = m_replyJsonData.getProperty("error").property("message").toString();
  QString errData = m_replyJsonData.getProperty("error").property("data").toString();

  if (errData.isEmpty() && errMsg.isEmpty())
    return true;

  m_lastError = QString("%1: %2").arg(errMsg, errData);

  return false;
}

int
ZbxHelper::openSession(void)
{
  setBaseUrl(m_sourceInfo.mon_url);
  QStringList params = ngrt4n::getAuthInfo(m_sourceInfo.auth);
  if (params.size() != 2) {
    m_lastError = tr("Bad auth string, should be in the form of login:password");
    return ngrt4n::RcGenericFailure;
  }

  params.push_back(QString::number(GetLogin));
  setSslPeerVerification(m_sourceInfo.verify_ssl_peer);

  if (postRequest(GetLogin, params) != ngrt4n::RcSuccess) {
    return ngrt4n::RcGenericFailure;
  }

  if (processLoginReply() != ngrt4n::RcSuccess) {
    return ngrt4n::RcGenericFailure;
  }

  // Get the API version and return
  return fecthApiVersion();
}

int
ZbxHelper::processLoginReply(void)
{
  qint32 tid = m_replyJsonData.getProperty("id").toInt32();
  QString result = m_replyJsonData.getProperty("result").toString();
  if (tid == ZbxHelper::GetLogin && ! result.isEmpty()) {
    m_auth = result;
    m_isLogged = true;
    return ngrt4n::RcSuccess;
  }
  m_lastError = tr("Login failed");

  return ngrt4n::RcGenericFailure;
}

int
ZbxHelper::fecthApiVersion(void)
{
  setSslPeerVerification(m_sourceInfo.verify_ssl_peer);

  QStringList params {QString::number(GetApiVersion)};
  if (postRequest(ZbxHelper::GetApiVersion, params) != ngrt4n::RcSuccess) {
    return ngrt4n::RcGenericFailure;
  }

  return processGetApiVersionReply();
}

int
ZbxHelper::processGetApiVersionReply(void)
{
  qint32 tid = m_replyJsonData.getProperty("id").toInt32();
  if (tid != ZbxHelper::GetApiVersion) {
    m_lastError = tr("the transaction id does not correspond to getApiVersion");
    return ngrt4n::RcGenericFailure;
  }

  if (! checkBackendSuccessfulResult())
    return ngrt4n::RcGenericFailure;

  setApiVersion( m_replyJsonData.getProperty("result").toString() );
  return ngrt4n::RcSuccess;
}

int
ZbxHelper::processTriggerData(ChecksT& checks)
{
  // check weird reponset
  qint32 tid = m_replyJsonData.getProperty("id").toInt32();

  if (tid != GetTriggersByHostOrGroup
      && tid != GetTriggersByHostOrGroupV18
      && tid != GetTriggersByIds) {
    m_lastError = tr("Unexpected transaction id: %1").arg(QString::number(tid));
    return ngrt4n::RcGenericFailure;
  }

  // now treat successful result
  QScriptValueIterator trigger(m_replyJsonData.getProperty("result"));
  while (trigger.hasNext()) {

    trigger.next();
    if (trigger.flags() & QScriptValue::SkipInEnumeration) continue;

    QScriptValue triggerJsonData = trigger.value();
    QString triggerId = triggerJsonData.property("triggerid").toString();
    QString triggerName = triggerJsonData.property("description").toString().trimmed();

    CheckT check;
    check.host = processHostJsonValue(triggerJsonData.property("hosts"));
    check.host_groups = processHostGroupsJsonValue(triggerJsonData.property("groups"));
    check.check_command = triggerName.toStdString();
    check.status = triggerJsonData.property("value").toInt32();
    if (check.status == ngrt4n::ZabbixClear) {
      check.alarm_msg = "OK ("+QString(triggerName).replace("{HOST.NAME}", check.host.c_str()).toStdString()+")";
    } else {
      check.alarm_msg = triggerJsonData.property("error").toString().toStdString();
      check.status = static_cast<int>(triggerJsonData.property("priority").toInteger());
    }

    if (tid == GetTriggersByHostOrGroupV18) {
      check.last_state_change = triggerJsonData.property("lastchange").toString().toStdString();
    } else {
      QScriptValueIterator item(triggerJsonData.property("items"));
      if (item.hasNext()) {
        item.next();
        if (item.flags()&QScriptValue::SkipInEnumeration) continue;
        QScriptValue itemData = item.value();
        check.last_state_change = itemData.property("lastclock").toString().toStdString();
      }
    }
    if (std::stoi(check.last_state_change, nullptr, 10) == 0) {
      check.last_state_change = QString::number(time(nullptr)).toStdString();
    }
    check.id = ID_PATTERN.arg(check.host.c_str(), triggerName).toStdString();
    checks.insert(triggerId.toStdString(), check);
  }
  return ngrt4n::RcSuccess;
}

int
ZbxHelper::loadChecks(const SourceT& srcInfo, ChecksT& checks, const QString& filterValue,
                      ngrt4n::RequestFilterT filterType)
{
  m_sourceInfo = srcInfo;

  checks.clear();

  if (! checkLogin()) {
    return ngrt4n::RcGenericFailure;
  }

  QStringList params;
  if (! filterValue.isEmpty()) {
    if (filterType == ngrt4n::GroupFilter) {
      params.push_back( QString("\"group\": \"%1\",").arg(filterValue) );
    } else {
      params.push_back( QString("\"filter\": { \"host\":[\"%1\"]},").arg(filterValue) );
    }
  } else {
    params.push_back("");
  }
  //params.push_back(filter);
  params.push_back(QString::number(m_getTriggersByHostOrGroupApiVersion));

  if (postRequest(m_getTriggersByHostOrGroupApiVersion, params) != ngrt4n::RcSuccess) {
    return ngrt4n::RcGenericFailure;
  }

  if (! checkBackendSuccessfulResult()) {
    return ngrt4n::RcGenericFailure;
  }

  return processTriggerData(checks);
}


std::pair<int, QString>
ZbxHelper::loadITServices(const SourceT& srcInfo, CoreDataT& cdata)
{
  m_sourceInfo = srcInfo;
  cdata.clear();
  cdata.monitor = MonitorT::Any;

  if (! checkLogin()) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("login failed: %s").arg(m_lastError));
  }

  QStringList params(QString::number(GetITServices));
  if (postRequest(GetITServices, params) != ngrt4n::RcSuccess) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("failed to post request: %s").arg(m_lastError));
  }

  if (! checkBackendSuccessfulResult()) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("failed to parse backend request: %s").arg(m_lastError));
  }

  ZabbixParentChildsDependenciesMapT parentChildsDependencies;
  ZabbixChildParentDependenciesMapT childParentDependencies;
  ZabbixServiceTriggerDependenciesMapT serviceTriggerDependencies;

  if (processZabbixITServiceData(cdata, parentChildsDependencies, childParentDependencies, serviceTriggerDependencies) != ngrt4n::RcSuccess) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("failed to process output: %s").arg(m_lastError));
  }

  if (setBusinessServiceDependencies(cdata.bpnodes, parentChildsDependencies) != ngrt4n::RcSuccess) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("failed to build dependencies for bpnodes: %s").arg(m_lastError));
  }

  if (setITServiceDataPoint(cdata.cnodes, serviceTriggerDependencies) != ngrt4n::RcSuccess) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("failed to build dependencies for cnodes: %s").arg(m_lastError));
  }

  NodeT rootService;
  rootService.id = ngrt4n::ROOT_ID;
  rootService.name = tr("Zabbix IT Services");
  rootService.icon = "Zabbix";
  rootService.type = NodeType::BusinessService;
  rootService.child_nodes = extractTopParentServices(cdata.bpnodes, childParentDependencies);
  cdata.bpnodes.insert(ngrt4n::ROOT_ID, rootService);

  return std::make_pair(ngrt4n::RcSuccess, "");
}

QString ZbxHelper::extractTopParentServices(const NodeListT& bpnodes, const ZabbixChildParentDependenciesMapT& childParentDependencies)
{
  QString result = "";
  NodeListT::ConstIterator node = bpnodes.begin();
  while (node != bpnodes.end()) {
    ZabbixChildParentDependenciesMapT::ConstIterator dependency = childParentDependencies.find(node->id);
    if (dependency == childParentDependencies.end()) {
      result += result.isEmpty() ? node->id : QString(",%1").arg(node->id);
    }
    ++node;
  }
  return result;
}

int ZbxHelper::processZabbixITServiceData(CoreDataT& cdata,
                                          ZabbixParentChildsDependenciesMapT& parentChildsDependencies,
                                          ZabbixChildParentDependenciesMapT& childParentDependencies,
                                          ZabbixServiceTriggerDependenciesMapT& serviceTriggerDependencies)
{
  qint32 tid = m_replyJsonData.getProperty("id").toInt32();
  if (tid != GetITServices) {
    m_lastError = tr("Unexpected transaction id: %1, expected: %2").arg(QString::number(tid), QString::number(GetITServices));
    return ngrt4n::RcGenericFailure;
  }

  QScriptValueIterator serviceIter(m_replyJsonData.getProperty("result"));
  while (serviceIter.hasNext()) {
    serviceIter.next();
    if (serviceIter.flags() & QScriptValue::SkipInEnumeration) continue;

    QScriptValue serviceJsonData = serviceIter.value();

    NodeT node;
    node.id = serviceJsonData.property("serviceid").toString();
    node.name = serviceJsonData.property("name").toString();

    int zabbixCalcRule = serviceJsonData.property("algorithm").toInt32();
    ngrt4n::AggregatedSeverityT aggregationRule = aggregationRuleFromZabbixCalcRule(zabbixCalcRule);

    node.sev_crule = aggregationRule.sev;
    node.weight =  aggregationRule.weight;
    node.sev_prule = PropRules::Unchanged;

    QString triggerId = serviceJsonData.property("triggerid").toString();
    if (triggerId.toInt() != 0) {
      node.type = NodeType::ITService;
      cdata.cnodes.insert(node.id, node);
      serviceTriggerDependencies.insert(node.id, triggerId);
    } else {
      node.type = NodeType::BusinessService;
      cdata.bpnodes.insert(node.id, node);
    }
    processAppendDependenciesJsonValue(serviceJsonData.property("dependencies"), parentChildsDependencies, childParentDependencies);
  }

  setServicesParent(cdata.bpnodes, childParentDependencies);
  setServicesParent(cdata.cnodes, childParentDependencies);

  return ngrt4n::RcSuccess;
}

void ZbxHelper::setServicesParent(NodeListT& nodes, const ZabbixChildParentDependenciesMapT& childParentDependencies)
{
  //FIXME: refactoring required
  // set parent info for bpnodes
  NodeListT::Iterator node = nodes.begin();
  while (node != nodes.end()) {
    ZabbixChildParentDependenciesMapT::ConstIterator parentIdIter = childParentDependencies.find(node->id);
    if (parentIdIter != childParentDependencies.end()) {
      node->parents = QSet<QString>{ *parentIdIter };
    } else {
      node->parents = QSet<QString>{ ngrt4n::ROOT_ID };
    }
    ++node;
  }
}

ngrt4n::AggregatedSeverityT ZbxHelper::aggregationRuleFromZabbixCalcRule(int zabbixCalcRule)
{
  ngrt4n::AggregatedSeverityT result;
  result.sev = CalcRules::Average;
  result.weight = ngrt4n::WEIGHT_UNIT;
  switch (zabbixCalcRule) {
    case 0:
      result.weight = 0;
      break;
    case 1:
      result.sev = CalcRules::Worst;
      break;
    case 2: // default
    default:
      result.sev = CalcRules::Average;
      break;
  }
  return result;
}



void ZbxHelper::processAppendDependenciesJsonValue(const QScriptValue& dependenviesJsonValue,
                                                   ZabbixParentChildsDependenciesMapT& parentChildsDependencies,
                                                   ZabbixChildParentDependenciesMapT& childParentDependencies)
{
  QScriptValueIterator entryIter(dependenviesJsonValue);
  while (entryIter.hasNext()) {
    entryIter.next();
    if (entryIter.flags() & QScriptValue::SkipInEnumeration) continue;
    QScriptValue entryContent = entryIter.value();
    QString parent = entryContent.property("serviceupid").toString();
    QString child = entryContent.property("servicedownid").toString();
    parentChildsDependencies[parent].insert(child);
    childParentDependencies.insert(child, parent);
  }
}

int ZbxHelper::setBusinessServiceDependencies(NodeListT& bpnodes, const ZabbixParentChildsDependenciesMapT& parentChildsDependencies)
{
  ZabbixParentChildsDependenciesMapT::ConstIterator dependency = parentChildsDependencies.begin();
  while (dependency != parentChildsDependencies.end()) {
    NodeListT::Iterator bpnode = bpnodes.find(dependency.key());
    if (bpnode != bpnodes.end()) {
      bpnode->child_nodes = QStringList( dependency.value().values() ).join(ngrt4n::CHILD_Q_SEP);
    }
    ++dependency;
  }
  return ngrt4n::RcSuccess;
}

int ZbxHelper::setITServiceDataPoint(NodeListT& cnodes, const ZabbixServiceTriggerDependenciesMapT& serviceTriggerDependencies)
{
  QStringList params;

  QSet<QString> uniqueTriggerIds = serviceTriggerDependencies.values().toSet();
  params.push_back( getTriggersIdsJsonList(uniqueTriggerIds) );
  params.push_back(QString::number(GetTriggersByIds));

  if (postRequest(GetTriggersByIds, params) != ngrt4n::RcSuccess) {
    return ngrt4n::RcGenericFailure;
  }

  if (! checkBackendSuccessfulResult()) {
    return ngrt4n::RcGenericFailure;
  }

  ChecksT dataPoints;
  if (processTriggerData(dataPoints) != ngrt4n::RcSuccess) {
    return ngrt4n::RcGenericFailure;
  }

  ZabbixServiceTriggerDependenciesMapT::ConstIterator serviceTriggerLink = serviceTriggerDependencies.begin();
  while (serviceTriggerLink != serviceTriggerDependencies.end()) {
    NodeListT::Iterator cnode = cnodes.find( serviceTriggerLink.key() );
    if (cnode != cnodes.end()) {
      ChecksT::ConstIterator dataPoint = dataPoints.find(serviceTriggerLink.value().toStdString());
      if (dataPoint != dataPoints.end()) {
        cnode->child_nodes = QString("%1:%2").arg(m_sourceInfo.id.trimmed(),
                                                  QString::fromStdString(dataPoint.value().id).trimmed());
      } else {
        cnode->child_nodes = "";
        qDebug()<< "Not trigger associated to the service: "<< cnodes[serviceTriggerLink.key()].name;
      }
    }
    ++serviceTriggerLink;
  }

  return ngrt4n::RcSuccess;
}


QString ZbxHelper::getTriggersIdsJsonList(const QSet<QString>& triggerIds)
{
  QString result = "";
  if (! triggerIds.isEmpty()) {
    QSet<QString>::ConstIterator idIter = triggerIds.begin();
    result = QString("\"%1\"").arg(*idIter);
    ++idIter;
    while (idIter != triggerIds.end()) {
      result.append(QString(",\"%1\"").arg(*idIter));
      ++ idIter;
    }
  }
  return QString("[%1]").arg(result);
}

void
ZbxHelper::setSslReplyErrorHandlingOptions(QNetworkReply* reply)
{
  reply->setSslConfiguration(m_sslConfig);
  if (m_sslConfig.peerVerifyMode() == QSslSocket::VerifyNone)
    reply->ignoreSslErrors();
}


std::string
ZbxHelper::processHostGroupsJsonValue(const QScriptValue& hostGroupJsonValue)
{
  std::string result("");
  QScriptValueIterator entryIter(hostGroupJsonValue);
  while (entryIter.hasNext()) {
    entryIter.next();
    if (entryIter.flags() & QScriptValue::SkipInEnumeration)
      continue;
    std::string name = entryIter.value().property("name").toString().toStdString();

    if (result.empty())
      result = name;
    else
      result.append(ngrt4n::CHILD_SEP).append(name);
  }

  return result;
}


std::string
ZbxHelper::processHostJsonValue(const QScriptValue& hostJsonValue)
{
  std::string result("");
  QScriptValueIterator entryIter(hostJsonValue);
  while (entryIter.hasNext()) {
    entryIter.next();
    if (entryIter.flags() & QScriptValue::SkipInEnumeration)
      continue;
    result = entryIter.value().property("host").toString().trimmed().toStdString();
    break;
  }

  return result;
}



