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
  QString statusToString(const qint32 & _status);
  void clear(CoreDataT& data);
  void alert(const QString  & msg);
  QString getAbsolutePath(const QString & _path);
  void delay(const qint32 & d);
  MonitorBroker::CriticityT getCriticity(const int& _monitor, const int & _statusOrSeverity);
  QColor getColor(const int &_criticity);
  QIcon getTreeIcon(const int &_criticity);
  bool findNode(CoreDataT* coreData, const QString& nodeId, NodeListT::iterator& node);
} //NAMESPACE

#endif // UTILS_CLIENT_HPP
