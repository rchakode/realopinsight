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
#include "client/SvConfigCreator.hpp"
#include <sstream>
#include <getopt.h>
#include "Base.hpp"

QString  usage = "usage: %1 [OPTION] [view_config]\n"
    "Options: \n"
    "	-v\n"
    "	  Print the version and license information.\n"
    "	-h \n"
    "	   Print this help.\n";

int main(int argc, char **argv)
{
  //FIXME: test ngrt4n-oc
  QApplication* app = new QApplication(argc, argv);
  app->setWindowIcon(QIcon(":images/built-in/icon.png"));
  app->setApplicationName(appName);
  app->setStyleSheet(Preferences::style());

  QString cmdName= basename(argv[0]);
  ostringstream versionMsg(QObject::tr("%1 Editor\nVersion %2 (%3)\n").arg(appName).arg(packageVersion).arg(releaseName).toStdString()
                           +QObject::tr("Copyright (c) 2010-%1 by NGRT4N Project. All rights reserved.\n").arg(releaseYear).toStdString()
                           +QObject::tr("%1").arg(packageUrl).toStdString());

  QString file = (argc >= 2)? argv[1] : "";
  int opt;
  if ( (opt = getopt(argc, argv, "hv") ) != -1) {
      switch (opt) {
        case 'v':
          cout << versionMsg.str() << endl;
          exit(0);

        case 'h':
          cout << usage.arg(cmdName).toStdString();
          exit(0);

        default:
          cout << usage.arg(cmdName).toStdString();
          exit (1);
          break;
        }
    }
  cout <<QObject::tr("Launching").toStdString()<< " "<<versionMsg.str()<<endl;
  Auth authentication;
  int userRole = authentication.exec();
  if( userRole != Auth::ADM_USER_ROLE && userRole != Auth::OP_USER_ROLE ) exit( 1 );

  SvCreator* svc = new SvCreator(userRole);
  svc->load(file);

  return app->exec();
}
