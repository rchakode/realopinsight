/*
 * Utils.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 11-08-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */

#ifndef UTILS_CLIENT_HPP
#define UTILS_CLIENT_HPP

#include "Base.hpp"
#include <QString>
#include <QIcon>

namespace utils {
  inline void delay(const qint32& d){ sleep(d);}
  inline std::string getCtime(const QString& dt, const QString& format){
    return QDateTime::fromString(dt, format).toString().toStdString();}
  inline std::string getCtime(const quint32& tt){
    return QDateTime::fromTime_t(tt).toString().toStdString();}
  inline void alert(const QString & msg) {
    QMessageBox::warning(0, QObject::tr("%1 - Warning").arg(appName), msg, QMessageBox::Yes);}
  QString criticityToText(const qint32& _status);
  void clear(CoreDataT& data);
  QString getAbsolutePath(const QString& _path);
  MonitorBroker::CriticityT computeCriticity(const int& _monitor, const int& _statusOrSeverity);
  int computePropCriticity(const qint8& _criticity, const qint8& propRule);
  QColor computeColor(const int &_criticity);
  QIcon computeCriticityIcon(const int &_criticity);
  bool findNode(NodeListT& bpnodes, NodeListT& cnodes, const QString& nodeId, NodeListT::iterator& node);
  bool findNode(CoreDataT* coreData, const QString& nodeId, NodeListT::iterator& node);
} //NAMESPACE

#endif // UTILS_CLIENT_HPP
