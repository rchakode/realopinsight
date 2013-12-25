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

namespace {
  Wt::Auth::Login loginObject;
}

AuthManager::AuthManager(DbSession* dbSession)
  : Wt::Auth::AuthWidget(loginObject),
    m_dbSession(dbSession),
    m_mainUI(NULL),
    m_logged(false)
{
  Wt::Auth::AuthModel* authModel = new Wt::Auth::AuthModel(DbSession::auth(), m_dbSession->users());
  authModel->addPasswordAuth(&m_dbSession->passwordAuthentificator());
  Wt::Auth::AuthWidget::setModel(authModel);

  loginObject.changed().connect(this, &AuthManager::handleAuthentication);
}

void AuthManager::handleAuthentication(void)
{
  if (loginObject.loggedIn()) {
    if (! m_logged) {
      m_dbSession->setLoggedUser(loginObject.user().id());
      Wt::log("notice")<<"[realopinsight] "<< m_dbSession->loggedUser().username<<" logged in.";

      setTemplateText(tr("Wt.Auth.template.logged-in"));
      m_mainUI = new WebMainUI(this);
      bindWidget("main-ui", m_mainUI);

      Wt::WImage* image = new Wt::WImage(Wt::WLink("/images/built-in/logout.png"), m_mainUI);
      image->setToolTip("Sign out");
      image->clicked().connect(this, &AuthManager::logout);
      bindWidget("logout-item", image);

      m_logged = true;
     // sleep(4);
     //m_mainUI->handleRefresh();
    }
  } else {
    m_logged = false;
  }
}

void AuthManager::create(void)
{
  Wt::Auth::AuthWidget::create();
  bindString("software", APP_NAME.toStdString());
  bindString("version", PKG_VERSION.toStdString());
  bindString("release-year", REL_YEAR.toStdString());
}

void AuthManager::logout(void)
{
  loginObject.logout();
  create();
}


bool AuthManager::isLogged(void)
{
  return loginObject.loggedIn();
}

