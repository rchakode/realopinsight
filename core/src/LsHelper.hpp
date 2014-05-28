/*
 * LsHelper.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-03-2014                                                  #
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

#ifndef MKLSHELPER_HPP
#define MKLSHELPER_HPP

#include "Base.hpp"
#include <QTcpSocket>

class LsHelper : public QTcpSocket
{
  Q_OBJECT
public:
  enum ReqTypeT{
    Host = 0,
    Service = 1
  };
  LsHelper(const QString& host, const int& port);
  ~LsHelper();

  bool connectToService(void);
  void disconnectFromService(void);
  bool requestData(const QString& host, const ReqTypeT& reqType);
  bool recvData(const ReqTypeT& reqType);
  bool fecthHostChecks(const QString& host);
  bool findCheck(const QString& id, CheckListCstIterT& check);
  void clearData(void) {m_checks.clear();}
  void setHost(const QString& host) {m_host = host;}
  void setPort(const int& port) {m_port = port;}
  bool isConnected() const {return state() == QAbstractSocket::ConnectedState;}

private:
  const static int DefaultTimeout = 50000; /* 5 seconds */
  QString m_host;
  qint32 m_port;
  RequestListT mrequestMap;
  CheckListT m_checks;
  QString m_errorMsg;
  void setRequestPatterns();
  void handleNetworkFailure() {handleNetworkFailure(QAbstractSocket::error());}
  void handleNetworkFailure(QAbstractSocket::SocketError error);
};

#endif // MKLSHELPER_HPP
