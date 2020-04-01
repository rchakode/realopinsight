/*
 * realopinsight-web.cpp
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

#include "WebUtils.hpp"
#include "AuthManager.hpp"
#include "WebMainUI.hpp"
#include "WebApplication.hpp"
#include "utils/wtwithqt/WQApplication.h"
#include <QCoreApplication>
#include <Wt/WServer.h>


std::unique_ptr<Wt::WApplication> createRoiApplication(const Wt::WEnvironment& env)
{
  return std::make_unique<WebApp>(env);
}


int main(int argc, char **argv)
{
  RoiQApp qtApp (argc, argv);
  Q_INIT_RESOURCE(realopinsight);

  try {
    Wt::WServer server(argv[0], SettingFactory::webConfigPath());
    server.setServerConfiguration(argc, argv);
    server.addEntryPoint(Wt::EntryPointType::Application, createRoiApplication, "", "favicon.ico");

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
