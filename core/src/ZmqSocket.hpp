/*
 * ZmqSocket.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
#	                                                                         #
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

#ifndef ZMQSOCKET_HPP
#define ZMQSOCKET_HPP
#include "global.hpp"
#include <QtCore/QtGlobal>
#include <string>
#include <sstream>
#include <QString>

class ZmqSocket
{
public:
  ZmqSocket(const int & _type);
  ZmqSocket(const std::string& uri, const int& _type);
  ~ZmqSocket();
  int init();
  int setupSocket();
  void releaseSocket();
  void reset();
  void send(const std::string & data);
  std::string recv() const;
  void makeHandShake();
  bool isReady() const {return m_connected2Server; }
  void* getSocket() const {return m_socket;}
  int getServerSerial() const {return m_serverSerial;}
  QString lastError() const {return m_lastError;}
  void setServerUri (const std::string& uri) { m_serverUri = uri; }


private:
  std::string m_serverUri;
  QString m_lastError;
  void *m_socket;
  void *m_context;
  int m_type;
  bool m_connected2Server;
  int m_serverSerial;
  int convert2ServerSerial(const std::string &versionStr);
};

#endif // ZMQSOCKET_HPP
