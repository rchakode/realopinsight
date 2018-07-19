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

  QJsonObject json = jdoc.object();
  QJsonArray items = json["items"].toArray();

  for (auto&& item: items) {
    auto&& ns = item.toObject();
    auto&& meta = ns["metadata"].toObject();
    nses.push_back(meta["name"].toString());
  }

  return std::make_pair(nses, true);
}


std::pair<QString, bool> K8sHelper::parsePods(const QByteArray& data, CoreDataT& cdata)
{
  QJsonParseError parserError;
  QJsonDocument jdoc= QJsonDocument::fromJson(data, &parserError);
  if (parserError.error != QJsonParseError::NoError) {
    return std::make_pair(parserError.errorString(), false);
  }

  QStringList serviceLabels;

  QJsonObject json = jdoc.object();
  QJsonArray items = json["items"].toArray();

  for (auto&& podRef: items) {
    NodeT pnode;

    pnode.type = NodeType::BusinessService;
    pnode.sev_prule = PropRules::Unchanged;
    pnode.sev_crule = CalcRules::Average; // pods imply a notion of high availability ?
    pnode.weight = ngrt4n::WEIGHT_UNIT;
    pnode.icon = ngrt4n::DEFAULT_ICON;

    auto&& pod = podRef.toObject();
    auto&& meta = pod["metadata"].toObject();
    pnode.description = QObject::tr("uid: %1, creationTimestamp: %2").arg(meta["uid"].toString(), meta["creationTimestamp"].toString());

    pnode.name = meta["name"].toString();
    pnode.id = ngrt4n::md5hash( pnode.name );

    auto&& srvLabel = meta["labels"].toObject()["app"].toString();
    pnode.parent = ngrt4n::md5hash( srvLabel );
    serviceLabels.push_back(srvLabel);

    cdata.bpnodes.insert(pnode.id, pnode);

    auto&& podStatus = pod["status"].toObject();
    auto&& containerStatuses = podStatus["containerStatuses"].toArray();

    for (auto&& containerStatusRef: containerStatuses) {
      NodeT cnode;

      cnode.parent =  pnode.id ;
      cnode.type = NodeType::ITService;
      cnode.sev_prule = PropRules::Unchanged;
      cnode.sev_crule = CalcRules::Worst;
      cnode.weight = ngrt4n::WEIGHT_UNIT;
      cnode.icon = ngrt4n::CONTAINER_ICON; // TODO add docker icon

      auto&& containerStatus = containerStatusRef.toObject();
      cnode.name = containerStatus["name"].toString();
      cnode.id = ngrt4n::md5hash( containerStatus["containerID"].toString() );

      auto&& stateParsed = parseState(containerStatus["state"].toObject());
      cnode.sev = stateParsed.first;
      cnode.description = stateParsed.second;

      cdata.cnodes.insert(cnode.id, cnode);
    }
  }

  // add service nodes
  for (auto && slabel: serviceLabels) {
    NodeT pnode;

    pnode.name = slabel;
    pnode.id = ngrt4n::md5hash(slabel);
    pnode.parent = ngrt4n::ROOT_ID; // namespace level ?
    pnode.type = NodeType::BusinessService;
    pnode.sev_prule = PropRules::Unchanged;
    pnode.sev_crule = CalcRules::Worst;
    pnode.weight = ngrt4n::WEIGHT_UNIT;
    pnode.icon = ngrt4n::DEFAULT_ICON;
    pnode.description = "";

    cdata.bpnodes.insert(pnode.id, pnode);
  }


  return std::make_pair("", true);
}


std::pair<int, QString> K8sHelper::parseState(const QJsonObject& state)
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

