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

AuthManager::AuthManager(DbSession* dbSession)
  : Wt::Auth::AuthWidget(DbSession::loginObject()),
    m_dbSession(dbSession),
    m_mainUI(NULL),
    m_logged(false)
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
    m_logged = true;
    m_dbSession->setLoggedUser(DbSession::loginObject().user().id());
    LOG("error", m_dbSession->loggedUser().username + " logged in.");
  } else {
    LOG("error", m_dbSession->loggedUser().username + " logged out");
  }
}

void AuthManager::createLoginView(void)
{
  //wApp->setInternalPath("");
  Wt::Auth::AuthWidget::setTemplateText(Wt::WString::tr("Wt.Auth.template.login"));
  Wt::Auth::AuthWidget::createLoginView();

  bindWidget("footer", utils::footer());

  Wt::Auth::AuthWidget::processEnvironment();
}

void AuthManager::createLoggedInView(void)
{
  m_logged = true;
  m_dbSession->setLoggedUser(DbSession::loginObject().user().id());
  Wt::log("notice")<<"[realopinsight] "<< m_dbSession->loggedUser().username<<" logged in.";

  setTemplateText(tr("Wt.Auth.template.logged-in"));
  m_mainUI = new WebMainUI(this);
  bindWidget("main-ui", m_mainUI);

  Wt::WImage* image = new Wt::WImage(Wt::WLink("/images/built-in/logout.png"), m_mainUI);
  image->setToolTip("Sign out");
  image->clicked().connect(this, &AuthManager::logout);
  bindWidget("logout-item", image);;
}

void AuthManager::logout(void)
{
  DbSession::loginObject().logout();
  refresh();
}


bool AuthManager::isLogged(void)
{
  return DbSession::loginObject().loggedIn();
}

