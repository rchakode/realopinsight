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
  K8sHelper(const QString& apiUrl, bool verifySslPeer, QString authToken);
  std::pair<QString, int> loadNamespaceView(const QString& in_namespace, CoreDataT& out_cdata);
  std::pair<QStringList, int> listNamespaces();
  std::pair<QByteArray, int> requestNamespacedItemsData(const QString& in_namespace, const QString& in_itemType);
  std::tuple<int,  std::string, std::string> extractStateInfo(const QJsonObject& state);
  std::pair<QStringList, int> parseNamespaces(const QByteArray& data);

  std::pair<QString, int> parseNamespacedServices(const QByteArray& in_data,
                                                  const QString& in_matchNamespace,
                                                  QMap<QString, QMap<QString, QString>>& out_selectorMaps,
                                                  NodeListT& out_bpnodes);

  std::pair<QString, int> parseNamespacedPods(const QByteArray& in_data,
                                              const QString& in_matchNamespace,
                                              const QMap<QString, QMap<QString, QString>>& in_allServicesSelectors,
                                              NodeListT& out_bpnodes,
                                              NodeListT& out_cnodes);


public Q_SLOTS:
  void exitEventLoop(const QNetworkReply::NetworkError& code) { m_eventLoop.exit(code);}

private:
  QUrl m_apiURL;
  QString m_hostname;
  bool m_verifySslPeer;
  QString m_userAuthToken;
  QEventLoop m_eventLoop;
  void setNetworkReplySslOptions(QNetworkReply* reply, bool verifyPeerOption);
  QSet<QString> findMatchingService(const QMap<QString, QMap<QString, QString>>& allServicesSelectors, const QMap<QString, QVariant>& podLabels);
  int convertToPodPhaseStatusEnum(const QString& podPhaseStatusText);
  QString getAuthTokenFromEnv();
};

#endif // K8SHELPER_H
