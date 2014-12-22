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
#include <QMessageBox>
#include <QSslConfiguration>


const RequestListT ZbxHelper::ReqPatterns = ZbxHelper::requestsPatterns();


ZbxHelper::ZbxHelper(const QString & baseUrl)
  : QNetworkAccessManager(),
    m_apiUri(baseUrl%ZBX_API_CONTEXT),
    m_reqHandler(new QNetworkRequest()),
    m_trid(-1),
    m_evlHandler(new QEventLoop(this)),
    m_isLogged(false)
{
  m_reqHandler->setRawHeader("Content-Type", "application/json");
  m_reqHandler->setUrl(QUrl(m_apiUri));

}

ZbxHelper::~ZbxHelper()
{
  delete m_reqHandler;
  delete m_evlHandler;
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
                            \"auth\": \"%1\", \
                            \"id\": %9}";
  patterns[GetTriggersByHostGroup] = "{\"jsonrpc\": \"2.0\", \
                                     \"auth\": \"%1\", \
                                     \"method\": \"trigger.get\", \
                                     \"params\": { \
                                     \"filter\": {%2}, \
                                     \"selectGroups\": [\"name\"], \
                                     \"selectHosts\": [\"host\"], \
                                     \"selectItems\": [\"key_\",\"name\",\"lastclock\"], \
                                     \"output\": [\"description\",\"value\",\"error\",\"comments\",\"priority\"], \
                                     \"limit\": -1}, \
                                     \"id\": %9}";
  patterns[GetTriggersByHostGroupV18] = "{\"jsonrpc\": \"2.0\", \
                                        \"auth\": \"%1\", \
                                        \"method\": \"trigger.get\", \
                                        \"params\": { \
                                        \"filter\": {%2}, \
                                        \"select_hosts\": [\"host\"], \
                                        \"output\":  \"extend\", \
                                        \"limit\": -1}, \
                                        \"id\": %9}";
  patterns[GetITServiceTriggersByIds] = "{\"jsonrpc\": \"2.0\", \
                                        \"auth\": \"%1\", \
                                        \"method\": \"trigger.get\", \
                                        \"params\": { \
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
                            \"params\": { \
                            \"output\": \"extend\", \
                            \"selectDependencies\": \"extend\" \
                            \"}, \
                            \"id\": %9}";

  return patterns;
}


bool ZbxHelper::checkLogin(void)
{
  if (! m_isLogged && openSession() != 0)
    return -1;

  return m_isLogged;
}

int
ZbxHelper::postRequest(qint32 reqId, const QStringList& params)
{
  QString request;

  if (reqId == GetLogin) {
    request = ReqPatterns[reqId];
  } else {
    request = ReqPatterns[reqId].arg(m_auth);
  }

  Q_FOREACH(const QString &param, params) {
    request = request.arg(param);
  }

  QNetworkReply* reply = QNetworkAccessManager::post(*m_reqHandler, ngrt4n::toByteArray(request));
  setSslReplyErrorHandlingOptions(reply);

  connect(reply, SIGNAL(finished()), m_evlHandler, SLOT(quit()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));

  m_evlHandler->exec();

  if (! reply || parseReply(reply) !=0)
    return -1;

  return 0;
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
ZbxHelper::setTrid(const QString& apiv)
{
  qint32 vnum = apiv.mid(0, 3).remove(".").toInt();
  if (vnum < 14) {
    m_trid = GetTriggersByHostGroupV18;
  } else {
    m_trid = GetTriggersByHostGroup;
  }
}

int
ZbxHelper::parseReply(QNetworkReply* reply)
{
  reply->deleteLater();

  // check for error in network communication
  if (reply->error() != QNetworkReply::NoError) {
    m_lastError = tr("%1 (%2)").arg(reply->errorString(), reply->url().toString()) ;
    return -1;
  }

  // now read data
  m_replyJsonData.setData( QString( reply->readAll() )) ;
  return 0;
}

bool
ZbxHelper::backendReturnedSuccessResult(void)
{
  QString errData = m_replyJsonData.getProperty("error").property("data").toString();
  QString errMsg = m_replyJsonData.getProperty("error").property("message").toString();

  if (errData.isEmpty() && errMsg.isEmpty())
    return true;

  m_lastError = QString("%1%2%3").arg(errData, (errData.isEmpty()? "" : "; "), errMsg);

  return false;
}

int
ZbxHelper::openSession(void)
{
  setBaseUrl(m_sourceInfo.mon_url);
  QStringList params = ngrt4n::getAuthInfo(m_sourceInfo.auth);
  if (params.size() != 2) {
    m_lastError = tr("Bad auth string, should be in the form of login:password");
    return -1;
  }

  params.push_back(QString::number(GetLogin));
  setSslPeerVerification(m_sourceInfo.verify_ssl_peer != 0);

  if (postRequest(GetLogin, params) != 0)
    return -1;

  if (processLoginReply() !=0)
    return -1;

  // Get the API version
  if (fecthApiVersion() != 0)
    return -1;

  return 0;
}

int
ZbxHelper::processLoginReply(void)
{
  qint32 tid = m_replyJsonData.getProperty("id").toInt32();
  QString result = m_replyJsonData.getProperty("result").toString();
  if (tid == ZbxHelper::GetLogin && ! result.isEmpty()) {
    m_auth = result;
    m_isLogged = true;
    return 0;
  }
  m_lastError = tr("Login failed");

  return -1;
}

int
ZbxHelper::fecthApiVersion(void)
{
  QStringList params;

  params.push_back(QString::number(GetApiVersion));
  setSslPeerVerification(m_sourceInfo.verify_ssl_peer);

  if (postRequest(ZbxHelper::GetApiVersion, params) != 0)
    return -1;

  if (processGetApiVersionReply() !=0)
    return -1;

  return 0;
}

int
ZbxHelper::processGetApiVersionReply(void)
{

  qint32 tid = m_replyJsonData.getProperty("id").toInt32();

  if (tid != ZbxHelper::GetApiVersion) {
    m_lastError = tr("the transaction id does not correspond to getApiVersion");
    return -1;
  }

  setTrid( m_replyJsonData.getProperty("result").toString() );

  return 0;
}

int
ZbxHelper::processTriggerData(ChecksT& checks)
{
  // check weird reponset
  qint32 tid = m_replyJsonData.getProperty("id").toInt32();
  if (tid != GetTriggersByHostGroup && GetTriggersByHostGroupV18) {
    m_lastError = tr("Unexpected transaction id returned: %1").arg(QString::number(tid));
    return -1;
  }

  // now treat successful result
  QScriptValueIterator trigger(m_replyJsonData.getProperty("result"));
  while (trigger.hasNext()) {

    trigger.next();
    if (trigger.flags() & QScriptValue::SkipInEnumeration) continue;

    QScriptValue triggerJsonData = trigger.value();
    QString triggerId = triggerJsonData.property("triggerid").toString();
    QString triggerName = triggerJsonData.property("description").toString();

    CheckT check;
    check.host = processHostJsonValue(triggerJsonData.property("hosts"));
    check.host_groups = processHostGroupsJsonValue(triggerJsonData.property("groups"));
    check.check_command = triggerName.toStdString();
    check.status = triggerJsonData.property("value").toInt32();
    if (check.status == ngrt4n::ZabbixClear) {
      check.alarm_msg = "OK ("+triggerName.toStdString()+")";
    } else {
      check.alarm_msg = triggerJsonData.property("error").toString().toStdString();
      check.status = triggerJsonData.property("priority").toInteger();
    }

    if (tid == GetTriggersByHostGroupV18) {
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
    check.id = ID_PATTERN.arg(check.host.c_str(), triggerName).toStdString();
    checks.insert(triggerId.toStdString(), check);
  }
  return 0;
}

int
ZbxHelper::loadChecks(const SourceT& srcInfo, ChecksT& checks, const QString& filterValue,
                      ngrt4n::RequestFilterT filterType)
{
  m_sourceInfo = srcInfo;

  checks.clear();

  if (! checkLogin())
    return -1;

  QStringList params;
  QString filter = "";
  if (! filterValue.isEmpty()) {
    if (filterType == ngrt4n::GroupFilter) {
      filter = QString("\"group\":[\"%1\"]").arg(filterValue);
    } else {
      filter = QString("\"host\":[\"%1\"]").arg(filterValue);
    }
  }
  params.push_back(filter);
  params.push_back(QString::number(m_trid));

  if (postRequest(m_trid, params) != 0)
    return -1;

  if (! backendReturnedSuccessResult())
    return -1;

  if (processTriggerData(checks) !=0)
    return -1;

  return 0;
}


int
ZbxHelper::loadITServices(const SourceT& srcInfo, CoreDataT& cdata)
{
  m_sourceInfo = srcInfo;

  ngrt4n::clearCoreData(cdata);

  cdata.monitor = ngrt4n::Zabbix;

  if (! checkLogin())
    return -1;

  QStringList params(QString::number(GetITServices));
  if (postRequest(GetITServices, params) != 0)
    return -1;

  if (! backendReturnedSuccessResult())
    return -1;

  ZabbixServiceTriggerIdsT triggerIds;
  ZabbixParentChildsDependenciesMapT parentChildsDependencies;
  ZabbixChildParentDependenciesMapT childParentDependencies;

  if (processZabbixServiceData(cdata, parentChildsDependencies, childParentDependencies, triggerIds))
    return -1;

  if (setBusinessServiceDependencies(cdata.bpnodes, parentChildsDependencies) != 0)
    return -1;

  if (setITServiceDataPoint(cdata.cnodes, triggerIds) != 0)
    return -1;

  return 0;
}


int ZbxHelper::processZabbixServiceData(CoreDataT& cdata,
                                        ZabbixParentChildsDependenciesMapT& parentChildsDependencies,
                                        ZabbixChildParentDependenciesMapT& childParentDependencies,
                                        ZabbixServiceTriggerIdsT& triggerIds)
{
  qint32 tid = m_replyJsonData.getProperty("id").toInt32();
  if (tid != GetITServices) {
    m_lastError = tr("Unexpected transaction id returned: %1, expected: %2").arg(QString::number(tid), QString::number(GetITServices));
    return -1;
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

    int triggerId = serviceJsonData.property("triggerid").toInt32();
    if (triggerId != 0) {
      triggerIds.insert(triggerId);
      node.type = NodeType::ITService;
      cdata.cnodes.insert(node.id, node);
    } else {
      node.type = NodeType::BusinessService;
      cdata.bpnodes.insert(node.id, node);
    }

    processAppendDependenciesJsonValue(serviceJsonData.property("dependencies"), parentChildsDependencies, childParentDependencies);
  }

  return 0;
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
  ZabbixParentChildsDependenciesMapT::ConstIterator relationIter = parentChildsDependencies.begin();
  while (relationIter != parentChildsDependencies.end()) {

    NodeListT::Iterator bpnode = bpnodes.find(relationIter.key());
    if (bpnode != bpnodes.end()) {
      bpnode->child_nodes = QStringList( relationIter.value().values() ).join(ngrt4n::CHILD_SEP.c_str());
    }

    ++relationIter;
  }
  return 0;
}

int ZbxHelper::setITServiceDataPoint(NodeListT& cnodes, const ZabbixServiceTriggerIdsT& triggerIds)
{
  QStringList params(getTriggersIdsJsonList(triggerIds));
  params.push_back(QString::number(GetITServiceTriggersByIds));

  if (postRequest(GetITServices, params) != 0)
    return -1;

  if (! backendReturnedSuccessResult())
    return -1;

  ChecksT checks;
  if (processTriggerData(checks) != 0)
    return -1;

  return 0;
}


QString ZbxHelper::getTriggersIdsJsonList(const ZabbixServiceTriggerIdsT& triggerIds)
{
  QString result = "";
  if (! triggerIds.isEmpty()) {
    ZabbixServiceTriggerIdsT::ConstIterator idIter = triggerIds.begin();
    result = QString("\"%1\"").arg(*idIter);
    ++idIter;
    while (idIter != triggerIds.end()) {
      result.append(QString(",\"%1\"").arg( QString("\"%1\"").arg(*idIter)));
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
    result = entryIter.value().property("host").toString().toStdString();
    break;
  }

  return result;
}



