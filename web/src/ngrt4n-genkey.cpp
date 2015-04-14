/*
 * ngrt4n-genkey.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Creation: 12-04-2015                                                     #
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


#include "WebLicenseActivation.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QString>
#include <QDebug>
#include <unistd.h>


int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  QString hostId;
  QString hostName;
  QString appVersion;
  int opt;
  bool printHelp = false;
  while ((opt = getopt (argc, argv, "m:n:v:h")) != -1)
    switch (opt) {
      case 'm':
        hostId = QString::fromUtf8(optarg, strlen(optarg));
        break;
      case 'n':
        hostName = QString::fromUtf8(optarg, strlen(optarg));
        break;
      case 'v':
        appVersion = QString::fromUtf8(optarg, strlen(optarg));
        break;
      case 'h':
      default:
        printHelp = true;
        break;
    }

  if (printHelp
      || hostId.isEmpty()
      || hostName.isEmpty()
      || appVersion.isEmpty()) {
    qDebug() << "Usage:\n    " << basename(argv[0]) << "-m <hostid> -n <hostname> -v <appversion>";
  } else {
    qDebug() << WebLicenseActivation::genKey(hostId, hostName, appVersion);
  }

  return a.exec();
}
