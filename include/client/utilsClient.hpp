/*
 * Utils.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 11-08-2012                                                 #
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
#include <QIcon>
#include <unistd.h>

namespace {
  const QString SRC_BASENAME = "Source";
}

namespace utils {

  inline void delay(const qint32& d)
  { sleep(d); }

  inline std::string getCtime(const QString& dt, const QString& format)
  { return QDateTime::fromString(dt, format).toString().toStdString(); }

  inline std::string getCtime(const quint32& tt)
  { return QDateTime::fromTime_t(tt).toString().toStdString(); }

  inline void alert(const QString & msg)
  { QMessageBox::warning(0, QObject::tr("%1 - Warning").arg(APP_NAME), msg, QMessageBox::Yes); }

  inline QString genNodeId()
  { return "ngrt4nsrv"+QDateTime::currentDateTime().toString("yyyymmddHHmmsszzz"); }

  inline QString sourceId(const qint32& idx)
  { return QString("%1%2").arg(SRC_BASENAME, QString::number(idx));}

  inline QString sourceKey(const qint32& idx)
  { return QString("/Sources/%1").arg(sourceId(idx));}

  inline QString sourceKey(const QString& id)
  { return QString("/Sources/%1").arg(id);}

  QString criticityToText(const qint32& _status);
  void clear(CoreDataT& data);
  QString getAbsolutePath(const QString& _path);
  MonitorBroker::SeverityT computeCriticity(const int& _monitor, const int& _statusOrSeverity);
  int computePropCriticity(const qint8& _criticity, const qint8& propRule);
  QColor computeColor(const int &_criticity);
  QIcon computeCriticityIcon(const int &_criticity);
  bool findNode(CoreDataT* coreData, const QString& nodeId, NodeListT::iterator& node);
  bool findNode(NodeListT& bpnodes,
                NodeListT& cnodes,
                const QString& nodeId,
                NodeListT::iterator& node);
  bool findNode(const NodeListT& bpnodes,
                const NodeListT& cnodes,
                const QString& nodeId,
                NodeListT::const_iterator& node);
  QSplashScreen* infoScreen(const QString & msg="");
  QString getWelcomeMsg(const QString& utility);
  QString source2Str(const SourceT& src);
  qint32 convert2ApiType(const QString& str);
  CheckT getUnknownService(int status, const QString& sid);
  QStringList sourceTypes(void);
  QStringList sourceIndexes(void);
  QPair<QString, QString> splitCheckInfo(const QString& info); /* return <[sourcei:]hostaddr, checkid> */
  QPair<QString, QString> splitSourceHostInfo(const QString& info); /* return <source, hostaddr> */
  QString getHostFromSourceStr(const QString& str);
  QString getSourceIdFromStr(const QString& str);
  QPair<bool, int> checkSourceId(const QString& id);
} //NAMESPACE

#endif // UTILS_CLIENT_HPP
