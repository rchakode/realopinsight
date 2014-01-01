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
  : Wt::Auth::AuthWidget(m_loginObject),
    m_dbSession(dbSession),
    m_mainUI(NULL),
    m_logged(false)
{
  Wt::Auth::AuthModel* authModel = new Wt::Auth::AuthModel(DbSession::auth(), m_dbSession->users());
  authModel->setVisible(Wt::Auth::AuthModel::RememberMeField, false);
  authModel->addPasswordAuth(&m_dbSession->passwordAuthentificator());
  Wt::Auth::AuthWidget::setModel(authModel);
  setRegistrationEnabled(false);
  m_loginObject.changed().connect(this, &AuthManager::handleAuthentication);
}

void AuthManager::handleAuthentication(void)
{
  if (m_loginObject.loggedIn()) {
    m_logged = true;
    m_dbSession->setLoggedUser(m_loginObject.user().id());
    Wt::log("notice")<<"[realopinsight] "<< m_dbSession->loggedUser().username<<" logged in.";
  } else {
    Wt::log("notice")<<"[realopinsight] "<< m_dbSession->loggedUser().username<<" logged out.";
  }
}

void AuthManager::createLoginView(void)
{
  //wApp->setInternalPath("");
  Wt::Auth::AuthWidget::setTemplateText(Wt::WString::tr("Wt.Auth.template.login"));
  Wt::Auth::AuthWidget::createLoginView();
  bindString("software", APP_NAME.toStdString());
  bindString("version", PKG_VERSION.toStdString());
  bindString("release-year", REL_YEAR.toStdString());
  Wt::Auth::AuthWidget::processEnvironment();
}

void AuthManager::createLoggedInView(void)
{
  m_logged = true;
  m_dbSession->setLoggedUser(m_loginObject.user().id());
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
  m_loginObject.logout();
  refresh();
}


bool AuthManager::isLogged(void)
{
  return m_loginObject.loggedIn();
}

