/*
 * ngrt4n-oc.cpp
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

#include "global.hpp"
#include "Auth.hpp"
#include "MainWindow.hpp"
#include "SvConfigCreator.hpp"
#include "utilsClient.hpp"
#include <sstream>
#include <getopt.h>
#include <QObject>
#include <iostream>

QString  usage = "usage: %1 [OPTION] [view_config]\n"
    "Options: \n"
    "	-c\n"
    "	   Launch the configuration utility\n"
    "	-v\n"
    "	  Print the version and license information.\n"
    "	-h \n"
    "	   Print this help.\n";

int main(int argc, char **argv)
{
  QApplication* app = new QApplication(argc, argv);
  INIT_TRANSLATION;
  app->setWindowIcon(QIcon (":images/built-in/icon.png"));
  app->setApplicationName(APP_NAME);
  app->setStyleSheet(GuiPreferences::style());
  QString cmdName = basename(argv[0]);
  QString versionMsg = ngrt4n::getWelcomeMsg(QObject::tr("Operations Console"));
  bool runConfig = false;
  int opt;
  if ((opt = getopt(argc, argv, "chv")) != -1) {
    switch (opt) {
      case 'c':
        runConfig = true;
        break;
      case 'v':
        std::cout<<versionMsg.toStdString()<<"\n";
        exit(0);
        break;
      case 'h':
        std::cout<<usage.arg(cmdName).toStdString();
        exit(0);
        break;
      default:
        std::cout<<usage.arg(cmdName).toStdString();
        exit(1);
        break;
    }
  }
  std::clog<<versionMsg.toStdString()<<"\n";
  Auth authentication;
  int userRole = authentication.exec();
  if ( userRole != Auth::AdmUserRole && userRole != Auth::OpUserRole) exit(1);

  if (runConfig) {
    GuiPreferences* preferenceWindow = new GuiPreferences(userRole, Preferences::ChangeMonitoringSettings);
    GuiPreferences* changePasswdWindow = new GuiPreferences(userRole, Preferences::ChangePassword);
    preferenceWindow->exec();
    changePasswdWindow->exec();
    exit(0);
  }
  QSplashScreen* info = ngrt4n::infoScreen(versionMsg);
  ngrt4n::delay(2);
  QString file = (argc >= 2)? argv[1] : "";
  if (file.isEmpty()) {
    info->clearMessage();
    info->showMessage(QObject::tr("You need to select a configuration file!"),
                      Qt::AlignCenter|Qt::AlignCenter);
    ngrt4n::delay(1); info->finish(0);
    file = QFileDialog::getOpenFileName(0,
                                        QObject::tr("%1 | Select a configuration file").arg(APP_NAME),
                                        ".",
                                        QObject::tr("Xml files (*.xml);;All files (*)"));

    if (file.isNull() || file.isEmpty()) {
      ngrt4n::alert(QObject::tr("No configuration file selected, the program will exit!"));
      exit(1);
    }
  }
  info->finish(0);
  MainWindow *console= new MainWindow(userRole, file);
  console->render();

  return app->exec();
}
