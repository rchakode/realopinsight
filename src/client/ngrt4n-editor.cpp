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

#include "utilsClient.hpp"
#include "core/ns.hpp"
#include "client/Auth.hpp"
#include "client/SvConfigCreator.hpp"
#include <sstream>
#include <getopt.h>
#include "Base.hpp"
#include <memory>

QString  usage = "usage: %1 [OPTION] [view_config]\n"
    "Options: \n"
    "	-v\n"
    "	  Print the version and license information.\n"
    "	-h \n"
    "	   Print this help.\n";

int main(int argc, char **argv)
{
  QApplication* app = new QApplication(argc, argv);
  INIT_TRANSLATION;
  app->setWindowIcon(QIcon(":images/built-in/icon.png"));
  app->setApplicationName(APP_NAME);
  app->setStyleSheet(Preferences::style());

  QString cmdName= basename(argv[0]);
  QString versionMsg = utils::getWelcomeMsg(QObject::tr("Editor"));
  QString file = (argc >= 2)? argv[1] : "";
  int opt;
  if ((opt = getopt(argc, argv, "hv")) != -1) {
      switch (opt) {
        case 'v':
          cout << versionMsg.toStdString() << endl;
          exit(0);
          break;
        case 'h':
          cout << usage.arg(cmdName).toStdString();
          exit(0);
          break;
        default:
          cout << usage.arg(cmdName).toStdString();
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
