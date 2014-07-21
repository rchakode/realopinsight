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


#include "RawSocket.hpp"
#include <cerrno>
#include <QDebug>

RawSocket::RawSocket(const QString& host, int port)
  : m_host(host),
    m_port(port)
{
}

RawSocket::~RawSocket()
{
  cleanUp();
}


int RawSocket::setupSocket(void)
{
#ifdef WIN32
  WSADATA wsa;
  int err = WSAStartup(MAKEWORD(2, 2), &wsa);
  if(err < 0) {
    m_lastError = QObject::tr("WSAStartup failed ");
    return -1;
  }
#endif

  m_sockAddr.sin_addr.s_addr = inet_addr(m_host.toStdString().c_str());
  m_sockAddr.sin_family = AF_INET;
  m_sockAddr.sin_port = htons(m_port);

  return 0;
}

void RawSocket::cleanUp(void)
{
#ifdef WIN32
  WSACleanup();
#endif
}

int RawSocket::makeRequest(const QByteArray& data)
{
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

  if (connect(sock, (SOCKADDR *)&m_sockAddr, sizeof(m_sockAddr)) == SOCKET_ERROR) {
    buildErrorString();
    return -1;
  }

  if (send(sock, data.data(), data.size(), 0) < data.size()) {
    buildErrorString();
    return -1;
  }

  if (recv(sock, m_lastResult, BUFFER_SIZE -1, 0) < 0) {
    m_lastError = QObject::tr("Failed receiving data");
    return -1;
  }

  closesocket(sock);
  return 0;
}

QString RawSocket::lastResult(void)
{
  return QString(m_lastResult);
}


void RawSocket::buildErrorString(void)
{
  int errorCode = -1;
#ifdef WIN32
  errorCode = WSAGetLastError();
#else
  errorCode = errno;
#define WSAEHOSTDOWN EHOSTUNREACH
#define WSAETIMEDOUT ETIMEDOUT
#define WSAEADDRNOTAVAIL EADDRNOTAVAIL
#define WSAENETDOWN ENETDOWN
#define WSAECONNRESET ECONNRESET
#define WSAECONNREFUSED ECONNREFUSED
#endif

  switch (errorCode) {
  case WSAEHOSTDOWN:
    m_lastError = QObject::tr("Host down or unreachable %1").arg(m_host);
    break;
  case WSAETIMEDOUT:
    m_lastError = QObject::tr("Connection failed due to timeout %1:%2").arg(m_host, QString::number(m_port));
    break;
  case WSAEADDRNOTAVAIL:
  case EADDRNOTAVAIL:
    m_lastError = QObject::tr("Cannot assign requested address %1:%2").arg(m_host, QString::number(m_port));
    break;
  case WSAENETDOWN:
    m_lastError = QObject::tr("Network is down");
    break;
  case WSAECONNRESET:
    m_lastError = QObject::tr("Connection reset by peer");
    break;
  case WSAECONNREFUSED:
    m_lastError = QObject::tr("Connection refused");
    break;
  default:
    m_lastError = QObject::tr("Socket operation failed with error %1").arg(errorCode);
    break;
  }
}
