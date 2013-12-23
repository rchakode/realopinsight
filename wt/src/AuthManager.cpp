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
    m_mainUI(NULL)
{
  Wt::Auth::AuthModel* authModel = new Wt::Auth::AuthModel(DbSession::auth(), m_dbSession->users());
  authModel->addPasswordAuth(&m_dbSession->passwordAuthentificator());
  Wt::Auth::AuthWidget::setModel(authModel);

  loginObject.changed().connect(this, &AuthManager::handleAuthentication);
}

void AuthManager::handleAuthentication(void)
{
  if (loginObject.loggedIn()) {
    m_dbSession->setLoggedUser(loginObject.user().id());
    setTemplateText(tr("Wt.Auth.template.logged-in"));
    m_mainUI = new WebMainUI(this);
    bindWidget("main-ui", m_mainUI);

    Wt::WImage* image = new Wt::WImage(Wt::WLink("/images/built-in/logout.png"), m_mainUI);
    image->setToolTip("Sign out");
    image->clicked().connect(this, &AuthManager::logout);
    bindWidget("logout-item", image);

    Wt::log("notice")<<"[realopinsight] "<< m_dbSession->loggedUser().username<<" logged in.";
  } else {

  }
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

