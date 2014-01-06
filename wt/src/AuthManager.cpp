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
  : Wt::Auth::AuthWidget(DbSession::loginObject()),
    m_dbSession(dbSession),
    m_mainUI(NULL)
{
  Wt::Auth::AuthModel* authModel = new Wt::Auth::AuthModel(DbSession::auth(), m_dbSession->users());
  authModel->setVisible(Wt::Auth::AuthModel::RememberMeField, false);
  authModel->addPasswordAuth(&m_dbSession->passwordAuthentificator());
  Wt::Auth::AuthWidget::setModel(authModel);
  setRegistrationEnabled(false);
  DbSession::loginObject().changed().connect(this, &AuthManager::handleAuthentication);
}

void AuthManager::handleAuthentication(void)
{
  if (DbSession::loginObject().loggedIn()) {

    m_dbSession->setLoggedUser(DbSession::loginObject().user().id());

    LoginSession sessionInfo;
    sessionInfo.username = m_dbSession->loggedUser().username;
    sessionInfo.sessionId = wApp->sessionId();
    sessionInfo.firstAccess =sessionInfo.lastAccess = Wt::WDateTime::currentDateTime();
    sessionInfo.status = LoginSession::ActiveCookie;

    //FIXME: add only if there is not an active cookie
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
  wApp->setInternalPath(ngrt4n::LINK_LOGIN);

  Wt::Auth::AuthWidget::setTemplateText(Wt::WString::tr("Wt.Auth.template.login"));
  Wt::Auth::AuthWidget::createLoginView();

  bindWidget("footer", utils::footer());

  Wt::Auth::AuthWidget::processEnvironment();
}

void AuthManager::createLoggedInView(void)
{
  m_dbSession->setLoggedUser(DbSession::loginObject().user().id());

  LoginSession sessionInfo;
  sessionInfo.username = m_dbSession->loggedUser().username;

  try {
    sessionInfo.sessionId = wApp->environment().getCookie(sessionInfo.username);
    LOG("notice", "Using cookie "+sessionInfo.sessionId);
  } catch (const std::exception& ex) {
    sessionInfo.sessionId = "";
    LOG("notice", "No cookie set");
  }

  if (sessionInfo.sessionId.empty() ||
      m_dbSession->checkUserCookie(sessionInfo) == LoginSession::ActiveCookie) {
    setTemplateText(tr("Wt.Auth.template.logged-in"));
    m_mainUI = new WebMainUI(this);
    bindWidget("main-ui", m_mainUI);

    Wt::WImage* image = new Wt::WImage(Wt::WLink("/images/built-in/logout.png"), m_mainUI);
    image->setToolTip("Sign out");
    image->clicked().connect(this, &AuthManager::logout);
    bindWidget("logout-item", image);
  } else {
    logout();
  }
}

void AuthManager::logout(void)
{
  wApp->setInternalPath(ngrt4n::LINK_LOGOUT);
  DbSession::loginObject().logout();
  refresh();
}


bool AuthManager::isLogged(void)
{
  return DbSession::loginObject().loggedIn();
}

