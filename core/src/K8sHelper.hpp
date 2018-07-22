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


class K8sHelper : public QNetworkAccessManager
{
public:
  K8sHelper();
  std::pair<QStringList, bool> parseNamespaces(const QByteArray& data);
  std::pair<QString, bool> parseNamespacedServices(const QByteArray& data,
                                                   const QString& macthNamespace,
                                                   QMap<QString, QMap<QString, QString>>& selectorMaps,
                                                   NodeListT& bpnodes);
  std::pair<QString, bool> parseNamespacedPods(const QByteArray& data,
                                               const QString& macthNamespace,
                                               const QMap<QString, QMap<QString, QString>>& serviceSelectorInfos,
                                               NodeListT& bpnodes,
                                               NodeListT& cnodes);
  std::pair<int, QString> parseStateData(const QJsonObject& state);
  std::pair<QString, bool> findMatchingService(const QMap<QString, QMap<QString, QString>>& serviceSelectorInfos,
                                               const QMap<QString, QVariant>& podLabels);
};

#endif // K8SHELPER_H
