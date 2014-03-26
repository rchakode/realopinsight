/*
 * ngrt4n-editor.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
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

#include "utilsClient.hpp"
#include "global.hpp"
#include "Auth.hpp"
#include "Base.hpp"
#include "SvConfigCreator.hpp"
#include "GuiPreferences.hpp"
#include <iostream>
#include <getopt.h>

QString  usage = "usage: %1 [OPTION] [view_config]\n"
    "Options: \n"
    "	-v\n"
    "	  Print the version and license information.\n"
    "	-h \n"
    "	   Print this help.\n";

int main(int argc, char **argv)
{
  QApplication* app = new QApplication(argc, argv);
  Q_INIT_RESOURCE(ngrt4n);
  INIT_TRANSLATION;
  app->setWindowIcon(QIcon(":images/built-in/icon.png"));
  app->setApplicationName(APP_NAME);
  app->setStyleSheet(GuiPreferences::style());

  QString cmdName= ngrt4n::basename(argv[0]);
  QString versionMsg = ngrt4n::getWelcomeMsg(QObject::tr("Editor"));
  QString file = (argc >= 2)? argv[1] : "";
  int opt;
  if ((opt = getopt(argc, argv, "hv")) != -1) {
    switch (opt) {
      case 'v':
        std::cout << versionMsg.toStdString() <<"\n";
        exit(0);
        break;
      case 'h':
        std::cout << usage.arg(cmdName).toStdString();
        exit(0);
        break;
      default:
        std::cout << usage.arg(cmdName).toStdString();
        exit (1);
        break;
    }
  }
  std::clog <<versionMsg.toStdString()<<"\n";
  Auth authentication;
  int userRole = authentication.exec();
  if (userRole != Auth::AdmUserRole && userRole != Auth::OpUserRole) exit(1);
  SvCreator* svc = new SvCreator(userRole);
  svc->load(file);
  return app->exec();
}
