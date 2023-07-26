/*
 * K8sHelper.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2018 Rodrigue Chakode (rodrigue.chakode@gmail.com)         #
# Creation Date: July 2018                                                 #
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
#include "K8sHelper.hpp"
#include "Base.hpp"
#include "utilsCore.hpp"
#include <fstream>
#include <string>
#include <utility>
#include <boost/asio.hpp>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

K8sHelper::K8sHelper(const QString& apiUrl, bool verifySslPeer, QString authToken)
  : m_verifySslPeer(verifySslPeer),
    m_userAuthToken(authToken)
{
  m_hostname = QString::fromStdString(boost::asio::ip::host_name());
  if (apiUrl.endsWith("/")) {
    m_apiURL.setUrl(apiUrl + "api/v1");
  } else {
    m_apiURL.setUrl(apiUrl + "/api/v1");
  }
}


std::pair<QString, int> K8sHelper::loadNamespaceView(const QString& in_namespace, CoreDataT& out_cdata)
{
  // process services
  auto resultRequestServicesData = requestNamespacedItemsData(in_namespace, "services");
  if (resultRequestServicesData.second != ngrt4n::RcSuccess) {
    return resultRequestServicesData;
  }

  QMap<QString, QMap<QString, QString>> serviceSelectorMaps;
  NodeListT serviceBpnodes;
  auto resultParseServices = parseNamespacedServices(resultRequestServicesData.first, in_namespace, serviceSelectorMaps, serviceBpnodes);
  if (resultParseServices.second != ngrt4n::RcSuccess) {
    return resultParseServices;
  }

  // process pods
  auto resultRequestPodsData = requestNamespacedItemsData(in_namespace, "pods");
  if (resultRequestPodsData.second != ngrt4n::RcSuccess) {
    return resultRequestPodsData;
  }

  auto resultParsePods = parseNamespacedPods(resultRequestPodsData.first, in_namespace, serviceSelectorMaps, out_cdata.bpnodes, out_cdata.cnodes);
  if (resultParsePods.second != ngrt4n::RcSuccess) {
    out_cdata.clear();
    return resultParsePods;
  }

  // append service nodes
  for(auto&& snode: serviceBpnodes) {
    out_cdata.bpnodes.insert(snode.id, snode);
  }

  // add namespace as root service node
  NodeT rootNode;
  rootNode.id = ngrt4n::ROOT_ID;
  rootNode.parents.clear();
  rootNode.name = in_namespace;
  rootNode.parents = QSet<QString>{""};
  rootNode.type = NodeType::BusinessService;
  rootNode.sev = ngrt4n::Unknown;
  rootNode.sev_prule = PropRules::Unchanged;
  rootNode.sev_crule = CalcRules::Average;
  rootNode.weight = ngrt4n::WEIGHT_UNIT;
  rootNode.icon = ngrt4n::K8S_NS;
  rootNode.description = "Namespace node";

  out_cdata.bpnodes.insert(rootNode.id, rootNode);
  out_cdata.hosts.insert(in_namespace, QStringList{in_namespace});
  out_cdata.monitor = MonitorT::Kubernetes;

  ngrt4n::fixupDependencies(out_cdata);

  return std::make_pair("", ngrt4n::RcSuccess);
}


std::pair<QStringList, int> K8sHelper::listNamespaces(void)
{
  //prepare http request
  QNetworkRequest req;
  req.setUrl(QUrl(QString("%1/namespaces").arg(m_apiURL.url())));
  req.setRawHeader("Accept", "application/json");

  if (m_apiURL.host() != "127.0.0.1" && m_apiURL.host() != "localhost") {
    req.setRawHeader("Host", m_hostname.toUtf8());
    auto authToken = m_userAuthToken;
    if (authToken.isEmpty()) {
      authToken = getAuthTokenFromEnv();
    }
    if (! authToken.isEmpty()) {
      req.setRawHeader("Authorization", QString("Bearer %1").arg(authToken).toUtf8());
    }
  }

  // make request and connect to the processing handlers
  QNetworkReply* reply = QNetworkAccessManager::get(req);
  connect(reply, SIGNAL(finished()), &m_eventLoop, SLOT(quit()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(exitEventLoop(QNetworkReply::NetworkError)));

  setNetworkReplySslOptions(reply, m_verifySslPeer);

  // wait synchronously before continuing
  m_eventLoop.exec();

  if (! reply) {
    auto&& errorMsg = QObject::tr("Unexpected NULL QNetworkReply");
    return std::make_pair(QStringList{errorMsg}, ngrt4n::RcRpcError);
  }

  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    return std::make_pair(QStringList{reply->errorString()}, ngrt4n::RcRpcError);
  }

  return parseNamespaces(reply->readAll());
}


std::pair<QByteArray, int> K8sHelper::requestNamespacedItemsData(const QString& in_namespace, const QString& in_itemType)
{
  //prepare http request
  QNetworkRequest req;
  req.setUrl( QUrl(QString("%1/namespaces/%2/%3").arg(m_apiURL.url(), in_namespace, in_itemType)));
  req.setRawHeader("Accept", "application/json");

  if (m_apiURL.host() != "127.0.0.1" && m_apiURL.host() != "localhost") {
    req.setRawHeader("Host", m_hostname.toUtf8());
    auto authToken = m_userAuthToken;
    if (authToken.isEmpty()) {
      authToken = getAuthTokenFromEnv();
    }
    if (! authToken.isEmpty()) {
      req.setRawHeader("Authorization", QString("Bearer %1").arg(authToken).toUtf8());
    }
  }

  // make request and connect to the processing handlers
  QNetworkReply* reply = QNetworkAccessManager::get(req);
  connect(reply, SIGNAL(finished()), &m_eventLoop, SLOT(quit()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(exitEventLoop(QNetworkReply::NetworkError)));

  setNetworkReplySslOptions(reply, m_verifySslPeer);

  // wait synchronously before continuing
  m_eventLoop.exec();

  if (! reply) {
    auto&& errorMsg = QByteArray("Unexpected NULL QNetworkReply");
    return std::make_pair(errorMsg, ngrt4n::RcRpcError);
  }

  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    return std::make_pair(reply->errorString().toLatin1(), ngrt4n::RcRpcError);
  }

  return std::make_pair(reply->readAll(), ngrt4n::RcSuccess);
}




std::pair<QStringList, int> K8sHelper::parseNamespaces(const QByteArray& data)
{
  QJsonParseError parserError;
  QJsonDocument jdoc= QJsonDocument::fromJson(data, &parserError);
  if (parserError.error != QJsonParseError::NoError) {
    return std::make_pair(QStringList{parserError.errorString()}, ngrt4n::RcParseError);
  }

  QJsonObject jsonData = jdoc.object();
  QJsonArray items = jsonData["items"].toArray();

  QStringList nses;
  for (auto item: items) {
    auto&& ns = item.toObject();
    auto&& meta = ns["metadata"].toObject();
    nses.push_back(meta["name"].toString());
  }

  return std::make_pair(nses, ngrt4n::RcSuccess);
}


std::pair<QString, int> K8sHelper::parseNamespacedServices(const QByteArray& in_data,
                                                           const QString& in_matchNamespace,
                                                           QMap<QString, QMap<QString, QString>>& out_selectorMaps,
                                                           NodeListT& out_bpnodes)
{
  QJsonParseError parserError;
  QJsonDocument jdoc= QJsonDocument::fromJson(in_data, &parserError);
  if (parserError.error != QJsonParseError::NoError) {
    return std::make_pair(parserError.errorString(), ngrt4n::RcParseError);
  }

  QJsonObject jsonData = jdoc.object();
  QJsonArray items = jsonData["items"].toArray();
  for (auto item: items) {
    NodeT serviceNode;
    serviceNode.type = NodeType::BusinessService;
    serviceNode.sev = ngrt4n::Unknown;
    serviceNode.sev_prule = PropRules::Unchanged;
    serviceNode.sev_crule = CalcRules::Average;
    serviceNode.weight = ngrt4n::WEIGHT_UNIT;
    serviceNode.icon = ngrt4n::K8S_SVC;

    auto&& serviceData = item.toObject();
    auto&& metaData = serviceData["metadata"].toObject();
    auto&& k8sNamespace = metaData["namespace"].toString();

    // escape service if not matches the given namespace
    if (k8sNamespace != in_matchNamespace) {
      continue;
    }

    auto&& serviceName = metaData["name"].toString();
    auto&& serviceSpecItem = serviceData["spec"];
    auto&& k8sServiceFqdn = QString("%1.%2").arg(serviceName, k8sNamespace);

    auto selectorData = serviceSpecItem.toObject()["selector"].toObject();
    auto selectorTags = selectorData.keys();
    QMap<QString, QString> curSelectorMap;
    for (auto&& stag: selectorTags) {
      curSelectorMap.insert(stag, selectorData[stag].toString());
    }
    out_selectorMaps.insert(serviceName, curSelectorMap);

    serviceNode.name = serviceName;
    serviceNode.id = ngrt4n::md5IdFromString(k8sServiceFqdn);
    serviceNode.parents = QSet<QString>{ ngrt4n::ROOT_ID }; // by convention the root node will be created and labeled with the namespace
    serviceNode.description = serviceSpecItem.toString();

    out_bpnodes.insert(serviceNode.id, serviceNode);
  }


  return std::make_pair("", ngrt4n::RcSuccess);
}

std::pair<QString, int> K8sHelper::parseNamespacedPods(const QByteArray& in_data,
                                                       const QString& in_matchNamespace,
                                                       const QMap<QString, QMap<QString, QString>>& in_allServicesSelectors,
                                                       NodeListT& out_bpnodes,
                                                       NodeListT& out_cnodes)
{
  QJsonParseError parserError;
  QJsonDocument jdoc= QJsonDocument::fromJson(in_data, &parserError);
  if (parserError.error != QJsonParseError::NoError) {
    return std::make_pair(parserError.errorString(), ngrt4n::RcSuccess);
  }

  QSet<QString> k8sNamespaces;
  QJsonObject jsonData = jdoc.object();
  QJsonArray items = jsonData["items"].toArray();

  for (auto item: items) {
    NodeT podNode;
    podNode.sev = ngrt4n::Unknown;
    podNode.sev_prule = PropRules::Unchanged;
    podNode.sev_crule = CalcRules::Average; // pods induce a notion of high availability
    podNode.weight = ngrt4n::WEIGHT_UNIT;
    podNode.icon = ngrt4n::K8S_POD;

    auto&& podData = item.toObject();
    auto&& metaData = podData["metadata"].toObject();
    auto&& k8sNamespace = metaData["namespace"].toString();

    // escape pod if not matches the given namespace
    if (k8sNamespace != in_matchNamespace) {
      continue;
    }
    k8sNamespaces.insert(k8sNamespace);

    // check whether pod selectors match any service
    auto&& podLabels =  metaData["labels"].toObject().toVariantMap();
    auto&& matchedServices= findMatchingService(in_allServicesSelectors, podLabels);
    if (matchedServices.isEmpty()) {
      continue;
    }

    podNode.parents.clear();
    for (const auto& matchedService: matchedServices) {
      auto&& matchedServiceFqdn = QString("%1.%2").arg(matchedService, k8sNamespace);
      podNode.parents.insert( ngrt4n::md5IdFromString(matchedServiceFqdn) );
    }

    auto&& podName = metaData["name"].toString();
    auto&& podUid = metaData["uid"].toString();
    auto&& podCreationTime = metaData["creationTimestamp"].toString();
    auto&& podFqdn = QString("%1.%2").arg(podName, k8sNamespace);
    podNode.name = podName;
    podNode.id = ngrt4n::md5IdFromString(podFqdn);
    podNode.description = QString("uid -> %1, creationTimestamp -> %2").arg(std::move(podUid), std::move(podCreationTime));

    // add pod node
    auto&& podStatusData = podData["status"].toObject();
    auto&& podPhaseStatus = podStatusData["phase"].toString();
    auto podStatusConditions = podStatusData["conditions"].toArray();
    const auto StatusPhase = convertToPodPhaseStatusEnum(podPhaseStatus);


    switch (StatusPhase) {
    // handle Failed and CrashLoopBackoff pods as IT services
    case ngrt4n::K8sPodPhaseFailed:
    case ngrt4n::K8sPodPhaseCrashLoopBackoff:
      podNode.type = NodeType::ITService;
      podNode.child_nodes = podFqdn;
      podNode.check.id = podNode.child_nodes.toStdString();
      podNode.check.host = podFqdn.toStdString();
      podNode.check.host_groups = podFqdn.toStdString();
      podNode.check.status = ngrt4n::K8sFailed;
      podNode.check.last_state_change = ngrt4n::convertToTimet(podCreationTime, "yyyy-MM-ddThh:mm:ssZ");
      podNode.check.alarm_msg = QString("pod is %1 because %2 (%3)").arg(podPhaseStatus, podStatusData["reason"].toString(), podStatusData["message"].toString()).toLower().toStdString();
      out_cnodes.insert(podNode.id, podNode);
      continue; // since there is not containerStatuses object to process
      break;
      // handle Pending, Running and Succeeded pods as business process nodes
    case ngrt4n::K8sPodPhasePending:
      podNode.type = NodeType::ITService;
      podNode.child_nodes = podFqdn;
      podNode.check.id = podNode.child_nodes.toStdString();
      podNode.check.host = podFqdn.toStdString();
      podNode.check.host_groups = podFqdn.toStdString();
      podNode.check.status = ngrt4n::K8sFailed;

      if (! podStatusConditions.empty()) {
        auto lastStatusCondition = podStatusConditions[0].toObject();
        podNode.check.last_state_change = ngrt4n::convertToTimet(lastStatusCondition["lastTransitionTime"].toString(), "yyyy-MM-ddThh:mm:ssZ");
        podNode.check.alarm_msg = QString("pod is %1 because %2 (%3)").arg(podPhaseStatus, lastStatusCondition["reason"].toString(), lastStatusCondition["message"].toString()).toLower().toStdString();
      } else { // unexpected situation
        podNode.check.last_state_change = "0";
        podNode.check.alarm_msg = "cannot get condition for pending state";
      }

      out_cnodes.insert(podNode.id, podNode);
      break;
    case ngrt4n::K8sPodPhaseRunning:
    case ngrt4n::K8sPodPhaseSucceeded:
      podNode.type = NodeType::BusinessService;
      out_bpnodes.insert(podNode.id, podNode);
      break;
    default:
      qDebug() << QObject::tr("Unknown pod phase: %1").arg(podPhaseStatus);
      continue;
      break;
    }

    auto&& containerStatuses = podStatusData["containerStatuses"].toArray();
    for (auto containerStatus: containerStatuses) {
      NodeT containerNode;
      containerNode.parents =  QSet<QString>{ podNode.id };
      containerNode.type = NodeType::ITService;
      containerNode.sev = ngrt4n::Unknown;
      containerNode.sev_prule = PropRules::Unchanged;
      containerNode.sev_crule = CalcRules::Worst; // all items composing a pod are typically required to have the pod operational
      containerNode.weight = ngrt4n::WEIGHT_UNIT;
      containerNode.icon = ngrt4n::CONTAINER_ICON;

      auto&& containerStatusData = containerStatus.toObject();
      auto&& containerName = containerStatusData["name"].toString();
      auto&& containerId = containerStatusData["containerID"].toString();
      auto&& ready = containerStatusData["ready"].toBool();
      auto&& restartCount = containerStatusData["restartCount"].toInt();

      containerNode.id = ngrt4n::md5IdFromString(QString("%1/%2").arg(podFqdn, containerId));
      containerNode.name = containerName;
      containerNode.description = QString("Ready -> %1, restartCount -> %2").arg(ready ? "true" : "false").arg(restartCount);
      containerNode.child_nodes = QString("%1/%2").arg(podFqdn, containerName);
      containerNode.check.id = containerNode.child_nodes.toStdString();
      containerNode.check.host = containerName.toStdString();
      containerNode.check.host_groups = podFqdn.toStdString();

      std::tie(containerNode.check.status,
               containerNode.check.last_state_change,
               containerNode.check.alarm_msg) = extractStateInfo(containerStatusData["state"].toObject());

      // format timestamp as seconds since epoch
      containerNode.check.last_state_change = ngrt4n::convertToTimet(
            (containerNode.check.last_state_change.empty()? podCreationTime : containerNode.check.last_state_change.c_str()),
            "yyyy-MM-ddThh:mm:ssZ");

      // add container node as IT service
      out_cnodes.insert(containerNode.id, containerNode);
    }
  }

  // if data match the given namespace
  if (k8sNamespaces.size() == 1) {
    return std::make_pair("", ngrt4n::RcSuccess);
  }

  // otherwise means no pod data
  out_bpnodes.clear();
  out_cnodes.clear();

  return std::make_pair(QObject::tr("no pod data matching namespace %1").arg(in_matchNamespace), ngrt4n::RcSuccess);
}


std::tuple<int,  std::string, std::string> K8sHelper::extractStateInfo(const QJsonObject& state)
{
  QJsonDocument stateDoc(state);

  auto&& stateKeys = state.keys();
  QString&& runningKey = "running";
  if (stateKeys.contains(runningKey)) {
    auto && stateTimestamp = state[runningKey].toObject()["startedAt"].toString().toStdString();
    return std::make_tuple(ngrt4n::K8sRunning, stateTimestamp, QObject::tr("container is running").toStdString());
  }

  QString&& terminatedKey  = "terminated";
  if (stateKeys.contains(terminatedKey)) {
    auto&& stateTerminated = state[terminatedKey].toObject();
    auto&& stateTimestamp = stateTerminated["finishedAt"].toString().toStdString();
    int stateCode = ngrt4n::K8sSucceeded;
    if (stateTerminated["exitCode"].toInt() != 0) {
      stateCode = ngrt4n::K8sFailed;
    }
    return std::make_tuple(stateCode, stateTimestamp, QObject::tr("container is terminated").toStdString());
  }

  // default state is "waiting"
  // See Kubernetes documentation: https://kubernetes.io/docs/reference/generated/kubernetes-api/v1.11/#containerstate-v1-core
  return std::make_tuple(ngrt4n::K8sPending, "", QObject::tr("container is wating").toStdString());
}


QSet<QString> K8sHelper::findMatchingService(const QMap<QString, QMap<QString, QString>>& allServicesSelectors,
                                             const QMap<QString, QVariant>& podLabels)
{
  QSet<QString>&& out{};
  auto&& labelsList = podLabels.keys();
  QSet<QString> podLabelsTags (labelsList.begin(), labelsList.end());
  for (auto&& currentServiceSelectors: allServicesSelectors.toStdMap()) {
    auto selectorTags = currentServiceSelectors.second.keys();
    if (podLabelsTags.contains(QSet<QString>(selectorTags.begin(), selectorTags.end()))) {
      bool matched = true;
      for (auto&& curTag: selectorTags) {
        if (currentServiceSelectors.second[curTag] != podLabels[curTag]) {
          matched = false;
          break;
        }
      }
      if (matched) {
        out.insert(currentServiceSelectors.first);
      }
    }
  }
  return out;
}

void K8sHelper::setNetworkReplySslOptions(QNetworkReply* reply, bool verifyPeerOption)
{
  if (! reply) {
    return ;
  }
  QSslConfiguration sslConfig;
  if (verifyPeerOption) {
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
  } else {
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    reply->ignoreSslErrors();
  }

  reply->setSslConfiguration(sslConfig);
}


int K8sHelper::convertToPodPhaseStatusEnum(const QString& podPhaseStatusText)
{
  auto&& podPhaseStatusUpper = podPhaseStatusText.toUpper();

  if (podPhaseStatusUpper == "RUNNING") {
    return ngrt4n::K8sPodPhaseRunning;
  }

  if (podPhaseStatusUpper == "SUCCEEDED") {
    return ngrt4n::K8sPodPhaseSucceeded;
  }

  if (podPhaseStatusUpper == "PENDING") {
    return ngrt4n::K8sPodPhasePending;
  }

  if (podPhaseStatusUpper == "FAILED") {
    return ngrt4n::K8sPodPhaseFailed;
  }

  if (podPhaseStatusUpper == "CRASHLOOPBACKOFF") {
    return ngrt4n::K8sPodPhaseCrashLoopBackoff;
  }

  return ngrt4n::K8sPodPhaseUndefined;
}


QString K8sHelper::getAuthTokenFromEnv()
{
  QString tokenFile = QString::fromLocal8Bit(qgetenv("REALOPINSIGHT_K8S_SA_TOKEN"));
  if (tokenFile.isEmpty()) {
    tokenFile = "/var/run/secrets/kubernetes.io/serviceaccount/token";
  }
  std::ifstream ifs(tokenFile.toStdString());
  std::string token(std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>())));
  if (! token.empty()) {
    return QString::fromStdString(token);
  }
  return "";
}
