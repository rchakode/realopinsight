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

K8sHelper::K8sHelper()
{

}


std::pair<QStringList, bool> K8sHelper::parseNamespaces(const QByteArray& data)
{
  QStringList nses;

  QJsonParseError parserError;
  QJsonDocument jdoc= QJsonDocument::fromJson(data, &parserError);
  if (parserError.error != QJsonParseError::NoError) {
    nses.push_back(parserError.errorString());
    return std::make_pair(nses, false);
  }

  QJsonObject jsonData = jdoc.object();
  QJsonArray items = jsonData["items"].toArray();

  for (auto item: items) {
    auto&& ns = item.toObject();
    auto&& meta = ns["metadata"].toObject();
    nses.push_back(meta["name"].toString());
  }

  return std::make_pair(nses, true);
}


std::pair<QString, bool> K8sHelper::parseNamespacedServices(const QByteArray& data,
                                                            const QString& macthNamespace,
                                                            QMap<QString, QMap<QString, QString>>& selectorMaps,
                                                            NodeListT& bpnodes)
{
  QJsonParseError parserError;
  QJsonDocument jdoc= QJsonDocument::fromJson(data, &parserError);
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

    // espace service if not matches the given namespac
    if (k8sNamespace != macthNamespace) {
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
    selectorMaps.insert(serviceName, curSelectorMap);

    serviceNode.name = serviceName;
    serviceNode.id = ngrt4n::md5hash(k8sServiceFqdn);
    serviceNode.description = serviceSpecItem.toString();

    bpnodes.insert(serviceNode.id, serviceNode);
  }


  return std::make_pair("", true);
}

std::pair<QString, bool> K8sHelper::parseNamespacedPods(const QByteArray& data,
                                                        const QString& macthNamespace,
                                                        CoreDataT& cdata)
{
  QJsonParseError parserError;
  QJsonDocument jdoc= QJsonDocument::fromJson(data, &parserError);
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

    // espace pod if not matches the given namespace
    if (k8sNamespace != macthNamespace) {
      continue;
    }
    k8sNamespaces.insert(k8sNamespace);

    auto&& podName = metaData["name"].toString();
    auto&& podUid = metaData["uid"].toString();
    auto&& podCreationTime = metaData["creationTimestamp"].toString();
    auto&& k8sAppLabel = metaData["labels"].toObject()["app"].toString();
    auto&& k8sPodFqdn = QString("%1.%2").arg(podName, k8sNamespace);
    auto&& k8sAppFqdn = QString("%1.%2").arg(k8sAppLabel, k8sNamespace);

    podNode.name = podName;
    podNode.id = ngrt4n::md5hash(k8sPodFqdn);
    podNode.description = QString("uid: %1, creationTimestamp: %2").arg(std::move(podUid), std::move(podCreationTime));
    podNode.parent = ngrt4n::md5hash(k8sAppFqdn);
    cdata.bpnodes.insert(podNode.id, podNode);

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
      containerNode.child_nodes = QString("%1/%2").arg(k8sAppFqdn, containerName);
      containerNode.id = ngrt4n::md5hash(QString("%1/%2").arg(k8sPodFqdn, containerId));
      containerNode.sev = stateData.first;
      containerNode.description = stateData.second;
      cdata.cnodes.insert(containerNode.id, containerNode);
    }
  }

  // if data match the given namespace
  if (k8sNamespaces.size() == 1) {
    NodeT pnode;
    pnode.id =ngrt4n::ROOT_ID;
    pnode.parent = "";
    pnode.name = *k8sNamespaces.begin();
    pnode.type = NodeType::BusinessService;
    pnode.sev_prule = PropRules::Unchanged;
    pnode.sev_crule = CalcRules::Worst;
    pnode.weight = ngrt4n::WEIGHT_UNIT;
    pnode.icon = ngrt4n::DEFAULT_ICON;
    pnode.description = "";
    cdata.bpnodes.insert(pnode.id, pnode);

    return std::make_pair("", true);
  }

  cdata.clear();
  return std::make_pair(QObject::tr("invalid namespaced data match for namespace %1").arg(macthNamespace), false);
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

