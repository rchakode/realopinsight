/*
 * K8sHelper.hpp
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

#ifndef K8SHELPER_H
#define K8SHELPER_H
#include "core/src/Base.hpp"
#include <QStringList>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>


class K8sHelper : public QNetworkAccessManager
{
  Q_OBJECT

public:
  K8sHelper(void);
  std::pair<QString, int> loadNamespaceView(const SourceT& in_sinfo, const QString& in_namespace, CoreDataT& out_cdata);
  std::pair<QStringList, int> listNamespaces(const SourceT& sinfo);
  std::pair<QByteArray, int> requestNamespacedItemsData(const SourceT& sinfo, const QString& k8sNamespace, const QString& itemType);
  std::pair<int, QString> parseStateData(const QJsonObject& state);
  std::pair<QStringList, int> parseNamespaces(const QByteArray& data);

  std::pair<QString, int> parseNamespacedServices(const QByteArray& in_data,
                                                  const QString& in_macthNamespace,
                                                  QMap<QString, QMap<QString, QString>>& out_selectorMaps,
                                                  NodeListT& out_bpnodes);

  std::pair<QString, int> parseNamespacedPods(const QByteArray& in_data,
                                              const QString& in_sacthNamespace,
                                              const QMap<QString, QMap<QString, QString>>& in_serviceSelectorInfos,
                                              NodeListT& out_bpnodes,
                                              NodeListT& out_cnodes);


public Q_SLOTS:
  void exitEventLoop(const QNetworkReply::NetworkError& code) { m_eventLoop.exit(code);}

private:
  QEventLoop m_eventLoop;
  void setNetworkReplySslOptions(QNetworkReply* reply, int verifyPeerOption);
  std::pair<QString, bool> findMatchingService(const QMap<QString, QMap<QString, QString>>& serviceSelectorInfos, const QMap<QString, QVariant>& podLabels);
};

#endif // K8SHELPER_H
