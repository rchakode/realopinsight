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

#include "WebApplication.hpp"
#include "WebBaseSettings.hpp"

RoiQApp::RoiQApp(int& argc, char ** argv)
  : QCoreApplication(argc, argv)
{
  ngrt4n::initCoreLogger();
}


RoiQApp::~RoiQApp()
{
  ngrt4n::freeCoreLogger();
}

bool RoiQApp::notify(QObject* receiver, QEvent* event)
{
  try {
    if (event && receiver) {
      return QCoreApplication::notify(receiver, event);
    }
  } catch(std::exception& ex) {
    CORE_LOG("fatal", ex.what());
  }
  return false;
}


WebApp::WebApp(const Wt::WEnvironment& env)
  : WQApplication(env, true)
{
  m_theme = std::make_shared<Wt::WBootstrapTheme>();
  m_theme->setVersion(Wt::BootstrapVersion::v3);
  setTheme(m_theme);

  useStyleSheet("/resources/css/font-awesome.min.css");
  useStyleSheet("/resources/css/kube/font.css");
  useStyleSheet("/resources/css/kube/kube.min.css");
  useStyleSheet("/resources/css/kube/kube.legenda.css");
  useStyleSheet("/resources/css/kube/master.css");
  useStyleSheet("/resources/css/kube/highlight.css");
  useStyleSheet("/resources/css/kube/master.css");
  useStyleSheet("/resources/css/realopinsight.css");
  requireJQuery("/resources/js/jquery-3.4.1.min.js");
  messageResourceBundle().use(docRoot()+"/resources/i18n/messages");
  setTwoPhaseRenderingThreshold(0);
  root()->setId("wrapper");

  m_dbSession = std::make_unique<DbSession>();
}

void WebApp::create()
{
  if (m_dbSession->isReady()) {
    m_authManagerRef = root()->addNew<AuthManager>(m_dbSession.get());
  } else {
    root()->addWidget(std::make_unique<Wt::WLabel>(Q_TR("Failed to connect to database. Please check logs for more details")));
  }
}

