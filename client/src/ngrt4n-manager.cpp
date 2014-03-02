/*
 * ngrt4n.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 24-05-2012                                                  #
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

#include "ns.hpp"
#include "utilsClient.hpp"
#include "client/Auth.hpp"
#include "client/MainWindow.hpp"
#include "client/SvConfigCreator.hpp"
#include <sstream>
#include <getopt.h>
#include <QTranslator>
#include <QObject>

QString  usage = "usage: %1 [OPTION] [view_config]\n"
    "Options: \n"
    "	-c\n"
    "	   Launch the configuration utility\n"
    "	-e [view_config]\n"
    "	   Run the VE utility and load the file view_config if specified\n"
    "	-d view_config\n"
    "	   Run the OC utility and load the file view_config\n"
    "	-v\n"
    "	  Print the version and license information.\n"
    "	-h \n"
    "	   Print this help.\n";


int main(int argc, char **argv)
{
  QApplication* app = new QApplication(argc, argv);
  INIT_TRANSLATION;
  app->setWindowIcon(QIcon(":images/built-in/icon.png"));
  app->setApplicationName(APP_NAME.toUpper());
  app->setStyleSheet(Preferences::style());
  QString cmdName = basename(argv[0]);
  QString versionMsg = ngrt4n::getWelcomeMsg("");
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
      info->showMessage(QObject::tr("You need to select a configuration file!"), Qt::AlignCenter|Qt::AlignCenter);
      ngrt4n::delay(1); info->finish(0);
      file = QFileDialog::getOpenFileName(0,
                                          QObject::tr("%1 | Select a configuration file").arg(APP_NAME),
                                          ".",
                                          QObject::tr("Xml files (*.xml);;All files (*)"));

      if (!file.length()){
        ngrt4n::alert(QObject::tr("No configuration file has been selected and the program will exit!"));
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
    Preferences* monPref = new Preferences(userRole, Preferences::ChangeMonitoringSettings);
    monPref->exec();
    exit(0);
  }
  return app->exec();
}
