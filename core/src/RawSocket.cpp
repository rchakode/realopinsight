/*
 * RawSocket.cpp
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


#include "Base.hpp"
#include "RawSocket.hpp"
#include <cerrno>
#include <QDebug>

RawSocket::RawSocket(const QString& host, uint16_t port)
  : m_host(host),
    m_port(port)
{
}

RawSocket::~RawSocket()
{
}


int RawSocket::setupSocket(void)
{
  m_sockAddr.sin_addr.s_addr = inet_addr(m_host.toStdString().c_str());
  m_sockAddr.sin_family = AF_INET;
  m_sockAddr.sin_port = (htons)( m_port );
  return ngrt4n::RcSuccess;
}


int RawSocket::makeRequest(const QByteArray& data)
{
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

  if (connect(sock, (SOCKADDR *)&m_sockAddr, sizeof(m_sockAddr)) == SOCKET_ERROR) {
    buildErrorString();
    return ngrt4n::RcRpcError;
  }

  if (send(sock, data.data(), static_cast<size_t>(data.size()), 0) < data.size()) {
    buildErrorString();
    return ngrt4n::RcRpcError;
  }

  char buffer[BUFFER_SIZE];
  ssize_t count = 0;
  m_lastResult.clear();
  while ((count = recv(sock, buffer, static_cast<size_t>(BUFFER_SIZE - 1), 0)) > 0) {
    m_lastResult.append(QString::fromUtf8(buffer, static_cast<int>(count)));
  }

  if (count < 0) {
    m_lastError = QObject::tr("Failed receiving data");
    return ngrt4n::RcRpcError;
  }

  closesocket(sock);
  return ngrt4n::RcSuccess;
}

void RawSocket::buildErrorString(void)
{
  switch (errno) {
    case EHOSTUNREACH:
      m_lastError = QObject::tr("%1: host down or unreachable %1").arg(socketAddr());
      break;
    case ETIMEDOUT:
      m_lastError = QObject::tr("%1: connection failed due to timeout %1:%2").arg(socketAddr());
      break;
    case EADDRNOTAVAIL:
      m_lastError = QObject::tr("%1: cannot assign requested address").arg(socketAddr());
      break;
    case ENETDOWN:
      m_lastError = QObject::tr("%1: network is down (%1)").arg(socketAddr());
      break;
    case ECONNRESET:
      m_lastError = QObject::tr("%1: connection reset by peer").arg(socketAddr());
      break;
    case ECONNREFUSED:
      m_lastError = QObject::tr("%1: connection refused").arg(socketAddr());
      break;
    default:
      m_lastError = QObject::tr("Socket operation failed with error %1").arg(errno);
      break;
  }
}
