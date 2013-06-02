/*
 * ngrt4n.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */

#include "core/ns.hpp"
#include "client/Auth.hpp"
#include "client/SvNavigator.hpp"
#include "client/SvConfigCreator.hpp"
#include "client/utilsClient.hpp"
#include <sstream>
#include <getopt.h>
#include <QObject>

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
  app->setStyleSheet(Preferences::style());
  QString cmdName = basename(argv[0]);
  QString versionMsg = utils::getWelcomeMsg(QObject::tr("Operations Console"));
  bool runConfig = false;
  int opt;
  if ((opt = getopt(argc, argv, "chv")) != -1) {
      switch (opt) {
        case 'c':
          runConfig = true;
          break;
        case 'v':
          cout<<versionMsg.toStdString()<<"\n";
          exit(0);
          break;
        case 'h':
          cout<<usage.arg(cmdName).toStdString();
          exit(0);
          break;
        default:
          cout<<usage.arg(cmdName).toStdString();
          exit(1);
          break;
        }
    }
  std::clog<<versionMsg.toStdString()<<"\n";
  Auth authentication;
  int userRole = authentication.exec();
  if ( userRole != Auth::AdmUserRole && userRole != Auth::OpUserRole) exit(1);

  if (runConfig) {
      Preferences* update_settings = new Preferences(userRole, Preferences::ChangeMonitoringSettings);
      Preferences* change_passwd = new Preferences(userRole, Preferences::ChangePassword);
      update_settings->exec();
      change_passwd->exec();
      exit(0);
    }
  QSplashScreen* info = utils::infoScreen(versionMsg);
  utils::delay(2);
  QString file = (argc >= 2)? argv[1] : "";
  if (file.isEmpty()) {
      info->clearMessage();
      info->showMessage(QObject::tr("You need to select a configuration file!"),
                        Qt::AlignCenter|Qt::AlignCenter);
      utils::delay(1); info->finish(0);
      file = QFileDialog::getOpenFileName(0,
                                          QObject::tr("%1 | Select a configuration file").arg(APP_NAME),
                                          ".",
                                          QObject::tr("Xml files (*.xml);;All files (*)"));

      if (!file.length()) {
          utils::alert(QObject::tr("No configuration file selected, the program will exit!"));
          exit(1);
        }
    }
  info->finish(0);
  SvNavigator *console= new SvNavigator(userRole);
  console->load(file, 0);
  console->startMonitor();

  return app->exec();
}
