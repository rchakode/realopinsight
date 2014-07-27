/*
# AuthManager.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
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

#include "WebUtils.hpp"
#include "WebMainUI.hpp"
#include "DbSession.hpp"
#include "AuthManager.hpp"
#include "LdapAuthModel.hpp"
#include <Wt/Auth/Login>
#include <Wt/Auth/AuthService>
#include <Wt/Auth/AbstractUserDatabase>
#include <Wt/WLineEdit>
#include <functional>
#include <Wt/Auth/AuthModel>
#include <Wt/WPushButton>
#include <Wt/WImage>
#include <Wt/WApplication>
#include <Wt/WEnvironment>

AuthManager::AuthManager(DbSession* dbSession)
  : Wt::Auth::AuthWidget(dbSession->loginObject()),
    m_dbSession(dbSession),
    m_mainUI(NULL)
{
  //Wt::Auth::AuthModel* authModel = new Wt::Auth::AuthModel(m_dbSession->auth(), m_dbSession->users());
  LdapAuthModel* authModel = new LdapAuthModel(m_dbSession->auth(), m_dbSession->users());
  authModel->setVisible(Wt::Auth::AuthModel::RememberMeField, false);
  authModel->addPasswordAuth(m_dbSession->passwordAuthentificator());
  setModel(authModel);
  setRegistrationEnabled(false);
  m_dbSession->loginObject().changed().connect(this, &AuthManager::handleAuthentication);
}

void AuthManager::handleAuthentication(void)
{
  if (m_dbSession->isLogged()) {
    m_dbSession->setLoggedUser();
    RoiDboLoginSession sessionInfo;
    sessionInfo.username = m_dbSession->loggedUser().username;
    sessionInfo.sessionId = wApp->sessionId();
    sessionInfo.firstAccess =sessionInfo.lastAccess = Wt::WDateTime::currentDateTime();
    sessionInfo.status = RoiDboLoginSession::ActiveCookie;

    m_dbSession->addSession(sessionInfo);
    wApp->setCookie(sessionInfo.username, sessionInfo.sessionId, 3600, "", "", false);

    QString logMsg = QObject::tr("%1 logged in. Session Id: %2")
        .arg(m_dbSession->loggedUser().username.c_str(), sessionInfo.sessionId.c_str());
    LOG("info",logMsg.toStdString());
  } else {
    wApp->removeCookie(m_dbSession->loggedUser().username, "", "");
    LOG("info", QObject::tr("%1 logged out").arg(m_dbSession->loggedUser().username.c_str()).toStdString());
  }
}

void AuthManager::createLoginView(void)
{
  Wt::Auth::AuthWidget::createLoginView();
  bindWidget("footer", ngrt4n::footer());
}

void AuthManager::createLoggedInView(void)
{
  m_dbSession->setLoggedUser();

  RoiDboLoginSession sessionInfo;
  sessionInfo.username = m_dbSession->loggedUser().username;

  setTemplateText(tr("Wt.Auth.template.logged-in"));
  try {
    m_mainUI = new WebMainUI(this);
    bindWidget("main-ui", m_mainUI);

    long diffTimeSinceBuilt = time(NULL) - BUILD_TIME;
    if (diffTimeSinceBuilt > 31556926) {
      bindWidget("update-banner", new Wt::WText("<div class=\"alert alert-danger\">This copy of the software is too old."
                                                " Please go to"
                                                " <a href=\"http://realopinsight.com\">http://realopinsight.com</a>"
                                                " to get a new copy.</div>",
                                                Wt::XHTMLText));
    } else {
      bindEmpty("update-banner");
    }
  } catch (const std::bad_alloc& ) {
    bindWidget("main-ui", new Wt::WText("Error: no sufficient memory, please consider to upgrade your system !"));
  }

  Wt::WImage* image = new Wt::WImage(Wt::WLink("images/built-in/logout.png"), m_mainUI);
  image->setToolTip(QObject::tr("Sign out").toStdString());
  image->clicked().connect(this, &AuthManager::logout);
  bindWidget("logout-item", image);
}

void AuthManager::logout(void)
{
  m_dbSession->loginObject().logout();
  refresh();
}


bool AuthManager::isLogged(void)
{
  return m_dbSession->loginObject().loggedIn();
}

