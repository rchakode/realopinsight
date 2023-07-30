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

#include "dbo/src/DbSession.hpp"
#include "WebUtils.hpp"
#include "WebMainUI.hpp"
#include "AuthManager.hpp"
#include "AuthModelProxy.hpp"
#include <functional>
#include <ctime>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/AbstractUserDatabase.h>
#include <Wt/WLineEdit.h>
#include <Wt/Auth/AuthModel.h>
#include <Wt/WPushButton.h>
#include <Wt/WImage.h>
#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>

AuthManager::AuthManager(DbSession* dbSession)
  : Wt::Auth::AuthWidget(dbSession->wtAuthLogin()),
    m_dbSession(dbSession),
    m_mainUIRef(nullptr)
{
  auto authProxyModel = std::make_unique<AuthModelProxy>(m_dbSession);
  authProxyModel->loginFailed().connect(this, &AuthManager::handleLoginFailed);
  authProxyModel->setVisible(Wt::Auth::AuthModel::RememberMeField, false);
  authProxyModel->addPasswordAuth(m_dbSession->passwordAuthenticator());
  setModel(std::move(authProxyModel));
  setRegistrationEnabled(false);
  m_dbSession->wtAuthLogin().changed().connect(this, &AuthManager::handleAuthorization);
}


void AuthManager::handleAuthorization(void)
{
  if (! m_dbSession->isLogged()) {
    wApp->removeCookie("realopinsightcookie", "", "");
    CORE_LOG("info", QObject::tr("%1 logged out").arg(m_dbSession->loggedUser().username.c_str()).toStdString());
  } else {
    m_dbSession->decodeLoggedUser();
    DboLoginSession sessionInfo;
    sessionInfo.username = m_dbSession->loggedUser().username;
    sessionInfo.sessionId = wApp->sessionId();
    sessionInfo.firstAccess = sessionInfo.lastAccess = Wt::WDateTime::currentDateTime().toString().toUTF8();
    sessionInfo.status = DboLoginSession::ActiveCookie;
    m_dbSession->addSession(sessionInfo);
    wApp->setCookie(sessionInfo.username, sessionInfo.sessionId, 3600, "", "", false);
    CORE_LOG("info", QObject::tr("%1 logged in. Session Id: %2").arg(m_dbSession->loggedUser().username.c_str(), sessionInfo.sessionId.c_str()).toStdString());
  }
}

void AuthManager::createLoginView(void)
{
  Wt::Auth::AuthWidget::createLoginView();
  bindWidget("footer", ngrt4n::footer());
  auto infoBox = std::make_unique<Wt::WText>();
  m_infoBoxRef = infoBox.get();
  bindWidget("info-box", std::move(infoBox));
}

void AuthManager::createLoggedInView(void)
{
  setTemplateText(tr("Wt.Auth.template.logged-in"));
  m_dbSession->decodeLoggedUser();
  DboLoginSession sessionInfo;
  sessionInfo.username = m_dbSession->loggedUser().username;
  try {
    auto mainUI = std::make_unique<WebMainUI>(this);
    m_mainUIRef = mainUI.get();
    m_mainUIRef->signoutTriggered().connect(this, &AuthManager::logout);
    bindWidget("main-ui", std::move(mainUI));
    bindEmpty("update-banner");
  } catch (const std::bad_alloc& ) {
    bindString("main-ui", "You are likely running low on memory, please upgrade your system !");
  }
}


void AuthManager::logout(void)
{
  m_dbSession->wtAuthLogin().logout();
  m_mainUIRef->removeGlobalWidget();
  CORE_LOG("debug", "disconnected");
}


void AuthManager::handleLoginFailed(std::string data)
{
  m_infoBoxRef->setText(data);
  m_infoBoxRef->setStyleClass("alert alert-danger");
}
