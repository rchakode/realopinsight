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


std::pair<QStringList, bool> K8sHelper::parseNamespaces(const QByteArray& data)
{

  QJsonParseError parserError;
  QJsonDocument jdoc= QJsonDocument::fromJson(data, &parserError);
  if (parserError.error != QJsonParseError::NoError) {
    return std::make_pair(QStringList{parserError.errorString()}, false);
  }

  QJsonObject jsonData = jdoc.object();
  QJsonArray items = jsonData["items"].toArray();

  QStringList nses;
  for (auto item: items) {
    auto&& ns = item.toObject();
    auto&& meta = ns["metadata"].toObject();
    nses.push_back(meta["name"].toString());
  }

  return std::make_pair(nses, true);
}


std::pair<QString, bool> K8sHelper::parseNamespacedServices(const QByteArray& in_data,
                                                            const QString& in_macthNamespace,
                                                            QMap<QString, QMap<QString, QString>>& out_selectorMaps,
                                                            NodeListT& out_bpnodes)
{
  QJsonParseError parserError;
  QJsonDocument jdoc= QJsonDocument::fromJson(in_data, &parserError);
  if (parserError.error != QJsonParseError::NoError) {
    return std::make_pair(parserError.errorString(), false);
  }

  QJsonObject jsonData = jdoc.object();
  QJsonArray items = jsonData["items"].toArray();
  for (auto item: items) {
    NodeT serviceNode;
    serviceNode.type = NodeType::BusinessService;
    serviceNode.sev_prule = PropRules::Unchanged;
    serviceNode.sev_crule = CalcRules::Average;
    serviceNode.weight = ngrt4n::WEIGHT_UNIT;
    serviceNode.icon = ngrt4n::DEFAULT_ICON;

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
    serviceNode.id = ngrt4n::md5hash(k8sServiceFqdn);
    serviceNode.parent = ngrt4n::ROOT_ID; // by convention the root node will be created and labeled with the namespace
    serviceNode.description = serviceSpecItem.toString();

    out_bpnodes.insert(serviceNode.id, serviceNode);
  }


  return std::make_pair("", true);
}

std::pair<QString, bool> K8sHelper::parseNamespacedPods(const QByteArray& in_data,
                                                        const QString& in_macthNamespace,
                                                        const QMap<QString, QMap<QString, QString>>& in_serviceSelectorInfos,
                                                        NodeListT& out_bpnodes,
                                                        NodeListT& out_cnodes)
{
  QJsonParseError parserError;
  QJsonDocument jdoc= QJsonDocument::fromJson(in_data, &parserError);
  if (parserError.error != QJsonParseError::NoError) {
    return std::make_pair(parserError.errorString(), false);
  }

  QSet<QString> k8sNamespaces;

  QJsonObject jsonData = jdoc.object();
  QJsonArray items = jsonData["items"].toArray();

  for (auto item: items) {
    NodeT podNode;
    podNode.type = NodeType::BusinessService;
    podNode.sev_prule = PropRules::Unchanged;
    podNode.sev_crule = CalcRules::Average; // pods imply a notion of high availability ?
    podNode.weight = ngrt4n::WEIGHT_UNIT;
    podNode.icon = ngrt4n::DEFAULT_ICON;

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
    auto&& podName = metaData["name"].toString();
    auto&& podUid = metaData["uid"].toString();
    auto&& podCreationTime = metaData["creationTimestamp"].toString();
    auto&& podFqdn = QString("%1.%2").arg(podName, k8sNamespace);

    podNode.name = podName;
    podNode.id = ngrt4n::md5hash(podFqdn);
    podNode.description = QString("uid: %1, creationTimestamp: %2").arg(std::move(podUid), std::move(podCreationTime));
    podNode.parent = ngrt4n::md5hash(serviceFqdn); // match what is set for node in method parseNamespacedServices

    // add pod node as business process service
    out_bpnodes.insert(podNode.id, podNode);

    auto&& containerStatuses = podData["status"].toObject()["containerStatuses"].toArray();
    for (auto containerStatus: containerStatuses) {
      NodeT containerNode;
      containerNode.parent =  podNode.id ;
      containerNode.type = NodeType::ITService;
      containerNode.sev_prule = PropRules::Unchanged;
      containerNode.sev_crule = CalcRules::Worst;
      containerNode.weight = ngrt4n::WEIGHT_UNIT;
      containerNode.icon = ngrt4n::CONTAINER_ICON;

      auto&& containerStatusData = containerStatus.toObject();
      auto&& containerName = containerStatusData["name"].toString();
      auto&& containerId = containerStatusData["containerID"].toString();
      auto&& stateData = parseStateData(containerStatusData["state"].toObject());

      containerNode.name = containerName;
      containerNode.child_nodes = QString("%1/%2").arg(serviceFqdn, containerName); //FIXME: is it correct ?
      containerNode.id = ngrt4n::md5hash(QString("%1/%2").arg(podFqdn, containerId));
      containerNode.sev = stateData.first;
      containerNode.description = stateData.second;
      containerNode.check.host = containerNode.name.toStdString();
      containerNode.check.host_groups = k8sNamespace.toStdString();
      containerNode.check.status = stateData.first;
      containerNode.check.id = QString("%1/%2").arg(serviceFqdn, containerName).toStdString(); //FIXME: is it correct ?
      containerNode.check.last_state_change = "-1"; // FIXME set current time ?

      // add container node as IT service
      out_cnodes.insert(containerNode.id, containerNode);
    }
  }

  // if data match the given namespace
  if (k8sNamespaces.size() == 1) {
    return std::make_pair("success", true);
  }

  // otherwise means invalid data
  out_bpnodes.clear();
  out_cnodes.clear();
  return std::make_pair(QObject::tr("invalid pod data matching namespace %1").arg(in_macthNamespace), false);
}


std::pair<int, QString> K8sHelper::parseStateData(const QJsonObject& state)
{
  QJsonDocument stateDoc(state);
  QString stateString(stateDoc.toJson(QJsonDocument::Compact));

  auto&& keys = state.keys();

  QString key = "running";
  if (keys.contains(key)) {
    return std::make_pair(ngrt4n::Normal, stateString);
  }

  key = "terminated";
  if (keys.contains(key)) {
    return std::make_pair(ngrt4n::Critical, stateString);
  }

  // default state is "waiting" => https://kubernetes.io/docs/reference/generated/kubernetes-api/v1.11/#containerstate-v1-core
  return std::make_pair(ngrt4n::Unknown, stateString);
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


std::pair<QString, bool> K8sHelper::retrieveAndProcessingK8sData(const SourceT& sinfo)
{
  //prepare http request
  QNetworkRequest networkRequest;
  networkRequest.setRawHeader("Accept", "application/json");
  networkRequest.setUrl( QUrl(QString("%1/api/v1/namespaces").arg(sinfo.mon_url)) );

  // make request and conncet to the processing handlers
  QNetworkReply* reply = QNetworkAccessManager::get(networkRequest);
  connect(reply, SIGNAL(finished()), &m_eventLoop, SLOT(quit()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(exitEventLoop(QNetworkReply::NetworkError)));

  // set ssl options
  QSslConfiguration sslConfig;
  if (sinfo.verify_ssl_peer != 0) {
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
  } else {
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
    reply->ignoreSslErrors();
  }
  reply->setSslConfiguration(sslConfig);

  // wait synchronously before continuing
  m_eventLoop.exec();

  if (! reply) {
    return std::make_pair(QObject::tr("Unexpected NULL QNetworkReply"), false);
  }

  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    return std::make_pair(QObject::tr("HTTP call to %1 ended with error: %2").arg(reply->url().toString(), reply->errorString()), false);
  }

  auto outNamespacesParsing = parseNamespaces(reply->readAll());
  if (! outNamespacesParsing.second || outNamespacesParsing.first.size() == 0) {
    return std::make_pair(QObject::tr("Get namespaces returned error or empty list at %1").arg(networkRequest.url().toString()), false);
  }



  return std::make_pair("", true);
}

