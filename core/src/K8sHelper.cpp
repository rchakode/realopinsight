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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <utility>

K8sHelper::K8sHelper(void)
{

}


std::pair<QString, int> K8sHelper::loadNamespaceView(const QString& in_k8sProxyUrl, bool in_verifySslPeer, const QString& in_namespace, CoreDataT& out_cdata)
{
  // process services
  auto resultRequestServicesData = requestNamespacedItemsData(in_k8sProxyUrl, in_verifySslPeer, in_namespace, "services");
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
  auto resultRequestPodsData = requestNamespacedItemsData(in_k8sProxyUrl, in_verifySslPeer, in_namespace, "pods");
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
  NodeT rnode;
  rnode.id = ngrt4n::ROOT_ID;
  rnode.parent = "";
  rnode.name = in_namespace;
  rnode.type = NodeType::BusinessService;
  rnode.sev_prule = PropRules::Unchanged;
  rnode.sev_crule = CalcRules::Worst;
  rnode.weight = ngrt4n::WEIGHT_UNIT;
  rnode.icon = ngrt4n::K8S_NS;
  rnode.description = "Namespace node";

  out_cdata.bpnodes.insert(rnode.id, rnode);
  ngrt4n::fixupParentChildrenDependencies(out_cdata);

  return std::make_pair("", ngrt4n::RcSuccess);
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
                                                           const QString& in_macthNamespace,
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
    serviceNode.sev_prule = PropRules::Unchanged;
    serviceNode.sev_crule = CalcRules::Average;
    serviceNode.weight = ngrt4n::WEIGHT_UNIT;
    serviceNode.icon = ngrt4n::K8S_SVC;

    auto&& serviceData = item.toObject();
    auto&& metaData = serviceData["metadata"].toObject();
    auto&& k8sNamespace = metaData["namespace"].toString();

    // escape service if not matches the given namespac
    if (k8sNamespace != in_macthNamespace) {
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
    serviceNode.parent = ngrt4n::ROOT_ID; // by convention the root node will be created and labeled with the namespace
    serviceNode.description = serviceSpecItem.toString();

    out_bpnodes.insert(serviceNode.id, serviceNode);
  }


  return std::make_pair("", ngrt4n::RcSuccess);
}

std::pair<QString, int> K8sHelper::parseNamespacedPods(const QByteArray& in_data,
                                                       const QString& in_macthNamespace,
                                                       const QMap<QString, QMap<QString, QString>>& in_serviceSelectorInfos,
                                                       NodeListT& outPodNodes,
                                                       NodeListT& outContainerNodes)
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
    podNode.type = NodeType::BusinessService;
    podNode.sev_prule = PropRules::Unchanged;
    podNode.sev_crule = CalcRules::Average; // pods induce a notion of high availability
    podNode.weight = ngrt4n::WEIGHT_UNIT;
    podNode.icon = ngrt4n::K8S_POD;

    auto&& podData = item.toObject();
    auto&& metaData = podData["metadata"].toObject();
    auto&& k8sNamespace = metaData["namespace"].toString();

    // escape pod if not matches the given namespace
    if (k8sNamespace != in_macthNamespace) {
      continue;
    }

    // check if pod selector match any service
    auto&& podLabels =  metaData["labels"].toObject().toVariantMap();
    auto serviceMatch = findMatchingService(in_serviceSelectorInfos, podLabels);
    if (! serviceMatch.second) {
      continue;
    }

    // start processing pod matching a service selector
    k8sNamespaces.insert(k8sNamespace);
    auto&& serviceFqdn = QString("%1.%2").arg(serviceMatch.first, k8sNamespace);
    auto podName = metaData["name"].toString();
    auto&& podUid = metaData["uid"].toString();
    auto&& podCreationTime = metaData["creationTimestamp"].toString();
    auto&& podFqdn = QString("%1.%2").arg(podName, k8sNamespace);

    podNode.name = podName;
    podNode.id = ngrt4n::md5IdFromString(podFqdn);
    podNode.description = QString("uid: %1, creationTimestamp: %2").arg(std::move(podUid), std::move(podCreationTime));
    podNode.parent = ngrt4n::md5IdFromString(serviceFqdn); // shall match what is set for node in method parseNamespacedServices

    // add pod node as business process service
    outPodNodes.insert(podNode.id, podNode);

    auto&& containerStatuses = podData["status"].toObject()["containerStatuses"].toArray();
    for (auto containerStatus: containerStatuses) {
      NodeT containerNode;
      containerNode.parent =  podNode.id ;
      containerNode.type = NodeType::ITService;
      containerNode.sev_prule = PropRules::Unchanged;
      containerNode.sev_crule = CalcRules::Worst; // all items composing a pod are typically required to have the pod operational
      containerNode.weight = ngrt4n::WEIGHT_UNIT;
      containerNode.icon = ngrt4n::CONTAINER_ICON;

      auto&& containerStatusData = containerStatus.toObject();
      auto&& containerName = containerStatusData["name"].toString();
      auto&& containerId = containerStatusData["containerID"].toString();
      auto&& restartCount = containerStatusData["restartCount"].toInt();
      auto&& stateData = parseStateData(containerStatusData["state"].toObject());

      containerNode.id = ngrt4n::md5IdFromString(QString("%1/%2").arg(podFqdn, containerId));
      containerNode.name = containerName;
      containerNode.description = QString("containerID: %1, restartCount: %2").arg(containerId, restartCount);
      containerNode.child_nodes = QString("%1/%2").arg(podFqdn, containerName); //FIXME: is it a good as assumption ?
      containerNode.check.id = containerNode.child_nodes.toStdString();
      containerNode.check.host = containerNode.name.toStdString();
      containerNode.check.host_groups = podName.toStdString();

      containerNode.check.status = stateData.first;
      containerNode.check.last_state_change = ngrt4n::convertToTimet(
                                                (stateData.second.isEmpty()? podCreationTime : stateData.second),
                                                "yyyy-MM-ddThh:mm:ssZ");

      // add container node as IT service
      outContainerNodes.insert(containerNode.id, containerNode);
    }
  }

  // if data match the given namespace
  if (k8sNamespaces.size() == 1) {
    return std::make_pair("", ngrt4n::RcSuccess);
  }

  // otherwise means no pod data
  outPodNodes.clear();
  outContainerNodes.clear();

  return std::make_pair(QObject::tr("no pod data matching namespace %1").arg(in_macthNamespace), ngrt4n::RcSuccess);
}


std::pair<int, QString> K8sHelper::parseStateData(const QJsonObject& state)
{
  QJsonDocument stateDoc(state);

  auto&& stateKeys = state.keys();
  QString&& runningKey = "running";
  if (stateKeys.contains(runningKey)) {
    auto && stateTimestamp = state[runningKey].toObject()["startedAt"].toString();
    return std::make_pair(ngrt4n::Normal, stateTimestamp);
  }

  QString&& terminatedKey  = "terminated";
  if (stateKeys.contains(terminatedKey)) {
    auto&& stateTerminated = state[terminatedKey].toObject();
    auto&& stateTimestamp = stateTerminated["finishedAt"].toString();
    int severity = ngrt4n::Normal;
    if (stateTerminated["exitCode"].toInt() != 0) {
      severity = ngrt4n::Critical;
    }
    return std::make_pair(severity, stateTimestamp);
  }

  // default state is "waiting"
  // See Kubernetes documentation: https://kubernetes.io/docs/reference/generated/kubernetes-api/v1.11/#containerstate-v1-core
  return std::make_pair(ngrt4n::Unknown, "");
}


std::pair<QString, bool> K8sHelper::findMatchingService(const QMap<QString, QMap<QString, QString>>& serviceSelectorInfos,
                                                        const QMap<QString, QVariant>& podLabels)
{
  QString outMatchedService = "";
  bool outSelectorMatched = false;

  auto&& podLabelsTags = QSet<QString>::fromList(podLabels.keys());
  for (auto&& curSelectorInfo: serviceSelectorInfos.toStdMap()) {
    auto&& selectorTags = curSelectorInfo.second.keys();
    if (podLabelsTags.contains(QSet<QString>::fromList(selectorTags))) {
      bool matched = true;
      for (auto&& curTag: selectorTags) {
        if (curSelectorInfo.second[curTag] != podLabels[curTag]) {
          matched = false;
          break;
        }
      }
      if (matched) {
        outSelectorMatched = true;
        outMatchedService = curSelectorInfo.first;
        break;
      }
    }
  }
  return std::make_pair(outMatchedService, outSelectorMatched);
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

std::pair<QStringList, int> K8sHelper::listNamespaces(const QString& in_k8sProxyUrl, bool in_verifySslPeer)
{
  //prepare http request
  QNetworkRequest networkRequest;
  networkRequest.setRawHeader("Accept", "application/json");

  if (in_k8sProxyUrl.endsWith("/")) {
    networkRequest.setUrl( QUrl(QString("%1api/v1/namespaces").arg(in_k8sProxyUrl)) );
  } else {
    networkRequest.setUrl( QUrl(QString("%1/api/v1/namespaces").arg(in_k8sProxyUrl)) );
  }

  // make request and conncet to the processing handlers
  QNetworkReply* reply = QNetworkAccessManager::get(networkRequest);
  connect(reply, SIGNAL(finished()), &m_eventLoop, SLOT(quit()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(exitEventLoop(QNetworkReply::NetworkError)));

  setNetworkReplySslOptions(reply, in_verifySslPeer);

  // wait synchronously before continuing
  m_eventLoop.exec();

  if (! reply) {
    auto&& errorMsg = QObject::tr("Unexpected NULL QNetworkReply");
    return std::make_pair(QStringList{errorMsg}, ngrt4n::RcRpcError);
  }

  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    auto&& errorMsg = QObject::tr("HTTP call to %1 ended with error: %2").arg(reply->url().toString(), reply->errorString());
    return std::make_pair(QStringList{errorMsg}, ngrt4n::RcRpcError);
  }

  return parseNamespaces(reply->readAll());
}

std::pair<QByteArray, int> K8sHelper::requestNamespacedItemsData(const QString& in_k8sProxyUrl, bool in_verifySslPeer, const QString& k8sNamespace, const QString& itemType)
{
  //prepare http request
  QNetworkRequest networkRequest;
  networkRequest.setRawHeader("Accept", "application/json");

  if (in_k8sProxyUrl.endsWith("/")) {
    networkRequest.setUrl( QUrl(QString("%1api/v1/namespaces/%2/%3").arg(in_k8sProxyUrl, k8sNamespace, itemType)));
  } else {
    networkRequest.setUrl( QUrl(QString("%1/api/v1/namespaces/%2/%3").arg(in_k8sProxyUrl, k8sNamespace, itemType)));
  }

  // make request and conncet to the processing handlers
  QNetworkReply* reply = QNetworkAccessManager::get(networkRequest);
  connect(reply, SIGNAL(finished()), &m_eventLoop, SLOT(quit()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(exitEventLoop(QNetworkReply::NetworkError)));

  setNetworkReplySslOptions(reply, in_verifySslPeer);

  // wait synchronously before continuing
  m_eventLoop.exec();

  if (! reply) {
    auto&& errorMsg = QByteArray("Unexpected NULL QNetworkReply");
    return std::make_pair(errorMsg, ngrt4n::RcRpcError);
  }

  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    auto&& errorMsg = QObject::tr("HTTP call to %1 ended with error: %2").arg(reply->url().toString(), reply->errorString()).toLatin1();
    return std::make_pair(errorMsg, ngrt4n::RcRpcError);
  }

  return std::make_pair(reply->readAll(), ngrt4n::RcSuccess);
}



