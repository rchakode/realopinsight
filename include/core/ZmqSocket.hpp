/*
 * ZmqSocket.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
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

#include <string>
#include <sstream>

class ZmqSocket
{
public:
  ZmqSocket(const int & _type);
  ZmqSocket(const std::string& uri, const int & _type);
  ~ZmqSocket();
  bool init();
  bool connect();
  bool connect(const std::string & _uri);
  bool bind(const std::string & _uri);
  void disconnecteFromService();
  void reset();
  void send(const std::string & _msg);
  std::string recv() const;
  void makeHandShake();
  bool isConnected() const {return m_connected2Server; }
  void* getSocket() const {return m_socket;}
  int getServerSerial() const {return m_serverSerial;}
  std::string getErrorMsg() const {return m_errorMsg;}
  void setServerUri (const std::string& uri) { m_serverUri = uri; }


private:
  std::string m_serverUri;
  std::string m_errorMsg;
  void *m_socket;
  void *m_context;
  int m_type;
  bool m_connected2Server;
  int m_serverSerial;
  int convert2ServerSerial(const std::string &versionStr);
};

#endif // ZMQSOCKET_HPP
