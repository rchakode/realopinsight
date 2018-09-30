/*
 * RawSocket.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Creation : 20-07-2014                                                    #
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


#ifndef RAWSOCKET_HPP
#define RAWSOCKET_HPP

#ifdef WIN32

#include <winsock2.h>

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#endif

#include <QString>
#include <QObject>


const size_t BUFFER_SIZE = 1024 * 1024; // 1 Mo

class RawSocket
{
public:
  RawSocket(const QString& host, uint16_t port);
  ~RawSocket();
  int setupSocket();
  int makeRequest(const QByteArray& data);
  QString& lastResult(void) {return m_lastResult;}
  QString lastError(void) const {return m_lastError;}
  QString socketAddr(void) const {return QString("%1:%2").arg(m_host, QString::number(m_port));}

private:
  QString m_lastError;
  QString m_lastResult;
  QString m_host;
  uint16_t m_port;
  SOCKADDR_IN m_sockAddr;

  void buildErrorString(void);
};

#endif // RAWSOCKET_HPP
