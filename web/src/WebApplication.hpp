/*
 * RealOpInsightQApp.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 06-10-2014                                                 #
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

#ifndef REALOPINSIGHTQAPP_HPP
#define REALOPINSIGHTQAPP_HPP
#include "AuthManager.hpp"
#include "utils/wtwithqt/WQApplication.h"
#include "dbo/src/DbSession.hpp"
#include <Wt/WBootstrapTheme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLabel.h>

class RoiQApp : public QCoreApplication
{
public:
  RoiQApp(int& argc, char ** argv);
  virtual ~RoiQApp();
  virtual bool notify(QObject* receiver, QEvent* event);
};

class WebApp : public Wt::WQApplication
{
public:
  WebApp(const Wt::WEnvironment& env);

protected:
  virtual void create();
  virtual void destroy() {}

private:
  AuthManager* m_authManagerRef;
  std::shared_ptr<Wt::WBootstrapTheme> m_theme;
  std::unique_ptr<DbSession> m_dbSession;
};


#endif // REALOPINSIGHTQAPP_HPP
