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
#include "utils/wtwithqt/WQApplication"
#include "dbo/src/DbSession.hpp"
#include <Wt/WBootstrapTheme>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WLabel>
#include "WebUtils.hpp"
#include "AuthManager.hpp"


class RoiQApp : public QCoreApplication
{
public:
  RoiQApp(int& argc, char ** argv)
    : QCoreApplication(argc, argv) { ngrt4n::initCoreLogger(); }
  virtual ~RoiQApp() { ngrt4n::freeCoreLogger(); }

  virtual bool notify(QObject* receiver, QEvent* event) {
    try {
      if (event) return QCoreApplication::notify(receiver, event);
    } catch(std::exception& ex) {
      CORE_LOG("fatal", ex.what());
    }
    return false;
  }
};

class WebApp : public Wt::WQApplication
{
public:
  WebApp(const Wt::WEnvironment& env)
    : WQApplication(env, true)
  {
    m_theme.setVersion(Wt::WBootstrapTheme::Version3);
  }

protected:
  virtual void create()
  {
    m_dirroot = "/";
    m_docroot = docRoot() +  m_dirroot;
    setTwoPhaseRenderingThreshold(0);
    useStyleSheet(m_dirroot+"resources/css/ngrt4n.css");
    useStyleSheet(m_dirroot+"resources/css/font-awesome.min.css");
    messageResourceBundle().use(m_docroot+"resources/i18n/messages");
    setTheme(&m_theme);
    requireJQuery(m_dirroot+"resources/js/jquery-1.10.2.min.js");

    root()->setId("wrapper");

    WebBaseSettings settings;
    m_dbSession = new DbSession(settings.getDbType(), settings.getDbConnectionString());

    if (m_dbSession->isConnected()) {
      root()->addWidget(new AuthManager(m_dbSession));
    } else {
      root()->addWidget(new Wt::WLabel(Q_TR("Failed to connect to database, please check logs for more details")));
    }
  }

  virtual void destroy()
  {
    delete m_dbSession;
  }

private:
  Wt::WBootstrapTheme m_theme;
  DbSession* m_dbSession;
  std::string m_dirroot;
  std::string m_docroot;
};


#endif // REALOPINSIGHTQAPP_HPP
