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

namespace {
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
  const double SCALIN_FACTOR = 1.1;
  const double SCALOUT_FACTOR = 1/SCALIN_FACTOR;
  const qint32 CHART_WIDTH=200;
  const qint32 CHART_HEIGHT=150;

  inline void delay(const qint32& d)
  { sleep(d); }

  inline std::string convertToTimet(const QString& dt, const QString& format)
  { return QString::number(QDateTime::fromString(dt, format).toTime_t()).toStdString(); }

  inline std::string humanTimeText(const std::string& mytime_t)
  { return QDateTime::fromTime_t(QString(mytime_t.c_str()).toUInt()).toString().toStdString(); }

  inline QString genNodeId()
  {
    static int i = 0;
    return QString("%1%2")
        .arg(QUuid::createUuid().toString().replace("{", "").replace("}", "").replace("-",""))
        .arg(QString::number(++i));
  }

  inline QString sourceId(const qint32& idx)
  { return QString("%1%2").arg(SRC_BASENAME, QString::number(idx));}

  inline QString sourceKey(const qint32& idx)
  { return QString("/Sources/%1").arg(sourceId(idx));}

  inline QString sourceKey(const QString& id)
  { return QString("/Sources/%1").arg(id);}

  inline QString realCheckId(const QString& srcid, const QString& chkid)
  { return QString("%1:%2").arg(srcid, chkid);}

  inline QString libVersion(void)
  { return PKG_VERSION; }

  QString getAbsolutePath(const QString& _path);
  qint8 severityFromProbeStatus(const int& monitorType, const int& statusValue);
  QString getIconPath(int _severity);
  bool findNode(CoreDataT* coreData, const QString& nodeId, NodeListT::iterator& node);
  bool findNode(NodeListT& bpnodes, NodeListT& cnodes,const QString& nodeId, NodeListT::iterator& node);
  bool findNode(const NodeListT& bpnodes, const NodeListT& cnodes, const QString& nodeId, NodeListT::const_iterator& node);
  bool findNode(const NodeListT& nodes, const QString& nodeId, NodeListT::const_iterator& node);
  QString sourceData2Json(const SourceT& src);
  qint32 convertToSourceType(const QString& str);
  void setCheckOnError(int status, const QString& msg, CheckT& invalidCheck);
  QStringList sourceTypes(void);
  QStringList sourceIndexes(void);
  StringPairT splitDataPointInfo(const QString& info); /* return <[sourcei:]hostaddr, checkid> */
  StringPairT splitSourceDataPointInfo(const QString& info); /* return <source, hostaddr> */
  QString getSourceIdFromStr(const QString& str);
  QPair<bool, int> checkSourceId(const QString& id);
  IconMapT nodeIcons();
  inline QByteArray toByteArray(const QString& str) { return QByteArray(str.toStdString().c_str(), str.length()); }
  QStringList getAuthInfo(const QString& authString);
  QString basename(const QString& path);

} //NAMESPACE

#endif // UTILS_CLIENT_HPP
