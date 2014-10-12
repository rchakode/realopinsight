/*
 * ngrt4n-manager.cpp
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

#include "utilsCore.hpp"
#include "client/src/Auth.hpp"
#include "client/src/MainWindow.hpp"
#include "client/src/SvConfigCreator.hpp"
#include "client/src/GuiPreferences.hpp"
#include <iostream>
#include <sstream>
#include <getopt.h>
#include <QTranslator>
#include <QObject>

QString  usage = "usage: %1 [OPTION] [description_file]\n"
    "Options: \n"
    "	-c\n"
    "	   Launches the Configuration Manager\n"
    "	-e [view_config]\n"
    "	   Runs the Editor and loads the given description file specified\n"
    "	-d view_config\n"
    "	   Runs the Operations Console and loads the given description file specified\n"
    "	-v\n"
    "	  Prints version and licenses information.\n"
    "	-h \n"
    "	   Prints this help.\n";


int main(int argc, char **argv)
{
  QApplication* app = new QApplication(argc, argv);
  Q_INIT_RESOURCE(ngrt4n);
  INIT_TRANSLATION;
  app->setWindowIcon(QIcon(":images/built-in/icon.png"));
  app->setApplicationName(APP_NAME.toUpper());
  app->setStyleSheet(GuiPreferences::style());
  QString cmdName = ngrt4n::basename(argv[0]);
  QString versionMsg = ngrt4n::getWelcomeMsg("Workstation");
  QString module = "config";
  QString file = (argc >= 2)? argv[1] : "";
  int opt;
  if ((opt = getopt(argc, argv, "chvd:e:")) != -1) {
    switch (opt) {
      case 'c':
        module = "config";
        break;
      case 'd':
        module = "dashboard";
        file = optarg;
        break;
      case 'e':
        module = "editor";
        file = optarg;
        break;
      case 'v':
        std::cout << versionMsg.toStdString()<<"\n";
        exit(0);
      case 'h': {
        std::cout << usage.arg(cmdName).toStdString();
        exit(0);
      }
      default:
        std::cout << usage.arg(cmdName).toStdString();
        exit(1);
        break;
    }
  }
  std::clog << versionMsg.toStdString()<<"\n";
  Auth authentication;
  int userRole = authentication.exec();
  if (userRole != Auth::AdmUserRole && userRole != Auth::OpUserRole) exit(1);
  if (module == "dashboard") {
    QSplashScreen* info = ngrt4n::infoScreen(versionMsg);
    ngrt4n::delay(1);
    if (file == "") {
      info->clearMessage();
      info->showMessage(QObject::tr("You need to select a description file!"), Qt::AlignCenter|Qt::AlignCenter);
      ngrt4n::delay(1); info->finish(0);
      file = QFileDialog::getOpenFileName(0,
                                          QObject::tr("%1 | Select a description file").arg(APP_NAME),
                                          ".",
                                          QObject::tr("Xml files (*.xml);;All files (*)"));

      if (!file.length()){
        ngrt4n::alert(QObject::tr("No description file has been selected and the program will exit!"));
        exit (1);
      }

    }
    info->finish(0);
    MainWindow* console= new MainWindow(userRole, file);
    console->render();
  } else if (module == "editor") {
    SvCreator* editor = new SvCreator(userRole);
    editor->load(file);
  } else if (module == "config") {
    GuiPreferences* monPref = new GuiPreferences(userRole, Preferences::ChangeMonitoringSettings);
    monPref->exec();
    exit(0);
  }
  return app->exec();
}
