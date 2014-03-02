#include "WebUtils.hpp"
#include "WebMainUI.hpp"
#include "DbSession.hpp"
#include "AuthManager.hpp"
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
  Wt::Auth::AuthModel* authModel = new Wt::Auth::AuthModel(DbSession::auth(), m_dbSession->users());
  authModel->setVisible(Wt::Auth::AuthModel::RememberMeField, false);
  authModel->addPasswordAuth(&m_dbSession->passwordAuthentificator());
  Wt::Auth::AuthWidget::setModel(authModel);
  setRegistrationEnabled(false);
  m_dbSession->loginObject().changed().connect(this, &AuthManager::handleAuthentication);
}

void AuthManager::handleAuthentication(void)
{
  if (m_dbSession->loginObject().loggedIn()) {

   // Wt::WApplication::instance()->changeSessionId();
   // refresh();

    m_dbSession->setLoggedUser(m_dbSession->loginObject().user().id());

    LoginSession sessionInfo;
    sessionInfo.username = m_dbSession->loggedUser().username;
    sessionInfo.sessionId = wApp->sessionId();
    sessionInfo.firstAccess =sessionInfo.lastAccess = Wt::WDateTime::currentDateTime();
    sessionInfo.status = LoginSession::ActiveCookie;

    m_dbSession->addSession(sessionInfo);

    wApp->setCookie(sessionInfo.username, sessionInfo.sessionId, 3600, "", "", false);
    LOG("error", m_dbSession->loggedUser().username + " logged in. Session Id: " + sessionInfo.sessionId);
  } else {
    wApp->removeCookie(m_dbSession->loggedUser().username, "", "");
    LOG("error", m_dbSession->loggedUser().username + " logged out");
  }
}

void AuthManager::createLoginView(void)
{
  Wt::Auth::AuthWidget::createLoginView();
  bindWidget("footer", ngrt4n::footer());
}

void AuthManager::createLoggedInView(void)
{
  m_dbSession->setLoggedUser(m_dbSession->loginObject().user().id());

  LoginSession sessionInfo;
  sessionInfo.username = m_dbSession->loggedUser().username;

  setTemplateText(tr("Wt.Auth.template.logged-in"));
  m_mainUI = new WebMainUI(this);
  bindWidget("main-ui", m_mainUI);

  Wt::WImage* image = new Wt::WImage(Wt::WLink("/images/built-in/logout.png"), m_mainUI);
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

