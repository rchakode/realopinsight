/*
 * utilsCore.hpp
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

#ifndef UTILS_CLIENT_HPP
#define UTILS_CLIENT_HPP

#include "Base.hpp"
#include <QString>
#include <unistd.h>

namespace
{
const QString SRC_BASENAME = "Source";
}

namespace ngrt4n
{
const int DefaultPort = 1983;
const int DefaultUpdateInterval = 300;
const int MaxMsg = 512;

const QString ROOT_ID = "root";
const QString PLUS = "plus";
const QString MINUS = "minus";
const QString DEFAULT_ICON = "Business Process";
const QString CONTAINER_ICON = "Container";
const QString APPLICATION_ICON = "Application";
const QString GENERIC_CHECK_ICON = "Other Check";
const QString K8S_POD = "Kubernetes Pod";
const QString K8S_SVC = "Kubernetes Service";
const QString K8S_NS = "Kubernetes Namespace";
const double SCALIN_FACTOR = 1.1;
const double SCALOUT_FACTOR = 1 / SCALIN_FACTOR;
const qint32 CHART_WIDTH = 200;
const qint32 CHART_HEIGHT = 150;

const IconMapT NodeIcons = {
    {DEFAULT_ICON, "images/business-process.png"},
    {CONTAINER_ICON, "images/docker-container.png"},
    {K8S_POD, "images/k8s-pod.png"},
    {K8S_NS, "images/k8s-ns.png"},
    {K8S_SVC, "images/k8s-svc.png"},
    {PLUS, "images/built-in/nav-plus.png"},
    {MINUS, "images/built-in/nav-minus.png"},
    {"Other Check", "images/check.png"},
    {"Server", "images/server.png"},
    {"Firewall", "images/firewall.png"},
    {"Router", "images/network.png"},
    {"Network", "images/network.png"},
    {"Switch", "images/switch.png"},
    {"Filer", "images/filer.png"},
    {"Hard disk", "images/harddisk.png"},
    {"Storage Area", "images/storage.png"},
    {"Linux", "images/linux.png"},
    {"Cloud", "images/cloud.png"},
    {"Hypervisor", "images/hypervisor.png"},
    {"Application", "images/application.png"},
    {"Web Access", "images/web.png"},
    {"Web server", "images/web-server.png"},
    {"Database Engine", "images/db.png"},
    {"Database Server", "images/db-server.png"},
    {"Process", "images/process.png"},
    {"Logfile", "images/log.png"},
    {"Network Bandwidth", "images/network-usage.png"},
    {"CPU", "images/cpu.png"},
    {"CPU Load", "images/performance-level.png"},
    {"Memory", "images/memory.png"},
    {"Memory Usage", "images/memory-usage.png"},
    {"Resource Utilization", "images/resource-usage.png"},
    {"Performance", "images/performance.png"},
    {"Nagios", "images/nagios-logo-n.png"},
    {"Zabbix", "images/zabbix-logo-z.png"},
    {"Zenoss", "images/zenoss-logo-o.png"},
    {"Tree", "images/hierarchy.png"}};

const QStringList DataSourceIndices = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
const QStringList MonitorSourceTypes = {
    MonitorT::toString(MonitorT::Kubernetes),
    MonitorT::toString(MonitorT::Nagios),
    MonitorT::toString(MonitorT::Zabbix)};

inline void delay(unsigned int d)
{
  sleep(d);
}

inline std::string convertToTimet(const QString &dt, const QString &format)
{
  return QString::number(QDateTime::fromString(dt, format).toTime_t()).toStdString();
}

inline std::string humanTimeText(const std::string &mytime_t)
{
  return QDateTime::fromTime_t(QString(mytime_t.c_str()).toUInt()).toString().toStdString();
}

inline QString generateId()
{
  static int i = 0;
  return QString("roi_%1%2")
      .arg(QUuid::createUuid().toString().replace("{", "").replace("}", "").replace("-", ""))
      .arg(QString::number(++i));
}

inline QString md5IdFromString(const QString &str)
{
  return QString("roi_%1").arg(QString(QCryptographicHash::hash((str.toUtf8()), QCryptographicHash::Md5).toHex()));
}

inline QString sourceId(const qint32 &idx)
{
  return QString("%1%2").arg(SRC_BASENAME, QString::number(idx));
}

inline QString sourceKey(const qint32 &sourceIndex)
{
  return QString("/Sources/%1").arg(sourceId(sourceIndex));
}

inline QString sourceKey(const QString &sourceId)
{
  return QString("/Sources/%1").arg(sourceId);
}

inline QString realCheckId(const QString &srcid, const QString &chkid)
{
  return QString("%1:%2").arg(srcid, chkid);
}

QString getAbsolutePath(const QString &_path);

qint8 severityFromProbeStatus(const int &monitorType, const int &statusValue);

QString getIconPath(int _severity);

bool findNode(CoreDataT *coreData, const QString &nodeId, NodeListT::iterator &node);

bool findNode(NodeListT &bpnodes, NodeListT &cnodes, const QString &nodeId, NodeListT::iterator &node);

bool findNode(const NodeListT &bpnodes, const NodeListT &cnodes, const QString &nodeId, NodeListT::const_iterator &node);

bool findNode(const NodeListT &nodes, const QString &nodeId, NodeListT::const_iterator &node);

void setCheckOnError(int status, const QString &msg, CheckT &invalidCheck);

StringPairT splitDataPointInfo(const QString &info); /* return <[source:]hostaddr, checkid> */

StringPairT splitSourceDataPointInfo(const QString &info); /* return <source, hostaddr> */

QString getSourceIdFromStr(const QString &str);

inline QByteArray toByteArray(const QString &str) { return QByteArray(str.toStdString().c_str(), str.length()); }

QStringList getAuthInfo(const QString &authString);

QString basename(const QString &path);

std::pair<int, QString> loadDynamicViewByGroup(const SourceT &sinfo, const QString &filter, CoreDataT &cdata);

std::pair<int, QString> loadDataItems(const SourceT &sinfo, const QString &filter, ChecksT &checks);

std::pair<int, QString> saveViewDataToPath(const CoreDataT &cdata, const QString &path);

QString generateNodeXml(const NodeT &node);

void fixupDependencies(CoreDataT &cdata);

void setParentChildDependency(const QString &childId, const QString &parentId, NodeListT &pnodes);

QString encodeXml(const QString &data);

QString decodeXml(const QString &data);

} // namespace ngrt4n

#endif // UTILS_CLIENT_HPP
