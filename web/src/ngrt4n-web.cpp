/*
 * ngrt4n-web.cpp
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

#include <QCoreApplication>
#include "dbo/DbSession.hpp"
#include "utils/wtwithqt/WQApplication"
#include <Wt/WServer>
#include "WebUtils.hpp"
#include "AuthManager.hpp"
#include "WebMainUI.hpp"
#include "Applications.hpp"




Wt::WApplication* createRealOpInsightWApplication(const Wt::WEnvironment& env)
{
  return new WebApp(env);
}



int main(int argc, char **argv)
{
  RealOpInsightQApp qtApp (argc, argv);
  Q_INIT_RESOURCE(ngrt4n);

  try {
    std::string configurationFile = "/opt/realopinsight/etc/wt_config.xml";
    Wt::WServer server(argv[0], configurationFile);
    server.setServerConfiguration(argc, argv);
    server.addEntryPoint(Wt::Application, &createRealOpInsightWApplication, "", "favicon.ico");

    if (server.start()) {
      Wt::WServer::waitForShutdown();
      server.stop();
    }
  } catch (dbo::Exception& ex){
    std::cerr << QObject::tr("[FATAL] %1").arg(ex.what()).toStdString();
    exit(1);
  } catch (std::exception &ex) {
    std::cerr << QObject::tr("[FATAL] %1").arg(ex.what()).toStdString();
    exit(1);
  }
  return qtApp.exec();
}
