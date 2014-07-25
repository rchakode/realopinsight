#include "LdapAuthModel.hpp"
#include "UserMngtUI.hpp"
#include <ldap.h>
#include <QObject>
#include <QDebug>

LdapAuthModel::LdapAuthModel(WebPreferences* preferences,
                             const Wt::Auth::AuthService& baseAuth,
                             Wt::Auth::AbstractUserDatabase& users,
                             Wt::WObject* parent)
  : Wt::Auth::AuthModel(baseAuth, users, parent),
    m_preferences(preferences)
{
}

//void LdapAuthModel::reset()
//{
//  Wt::Auth::AuthModel::reset();
//}
//bool LdapAuthModel::isVisible(Wt::WFormModel::Field field) const
//{
//  return Wt::Auth::AuthModel::isVisible(field);
//}

//bool LdapAuthModel::validateField(Wt::WFormModel::Field field)
//{
//  return Wt::Auth::AuthModel::validateField(field);
//}

bool LdapAuthModel::validate()
{
  // Implement validation
  //return Wt::Auth::AuthModel::validate();
  return true;
}

//void LdapAuthModel::configureThrottling(Wt::WInteractWidget* button)
//{
//  Wt::Auth::AuthModel::configureThrottling(button);
//}

//void LdapAuthModel::updateThrottling(Wt::WInteractWidget* button)
//{
//  Wt::Auth::AuthModel::updateThrottling(button);
//}

bool LdapAuthModel::login(Wt::Auth::Login& login)
{
  //if (! valid())
  //  return false;

  std::string username = valueText(Wt::Auth::FormBaseModel::LoginNameField).toUTF8();
  std::string password = valueText(Wt::Auth::AuthModel::PasswordField).toUTF8();

  if (username == "admin") {
    qDebug() << "Login through built-in database";
    return Wt::Auth::AuthModel::login(login);
  }

  qDebug() << "Login through LDAP"<< m_preferences->getLdapServerUri() ;

  if (ldapLogin(username, password)) {
    Wt::Auth::User user;
    login.login(user);
    qDebug() << "Authentication successful";
    return true;
  }

  qDebug() << m_lastError;
  return false;
}

void LdapAuthModel::logout(Wt::Auth::Login& login)
{
  login.logout();
  //This also removes the remember-me cookie for the user.
}

Wt::Auth::EmailTokenResult LdapAuthModel::processEmailToken(const std::string& token)
{
  return Wt::Auth::AuthModel::processEmailToken(token);
}

Wt::Auth::User LdapAuthModel::processAuthToken()
{
  return Wt::Auth::AuthModel::processAuthToken();
}


bool LdapAuthModel::ldapLogin(const std::string& username, const std::string& password)
{
  QString bindDn = m_preferences->getLdapDnFormat().replace("{USERNAME}", username.c_str(), Qt::CaseInsensitive);
  bool resultStatus = false;

  // Intialize a connection handler
  LDAP* ldapHandler;
  if (ldap_initialize(&ldapHandler, m_preferences->getLdapServerUri().toAscii())) {
    m_lastError = QObject::tr("failed initializing annuary handler");
    return resultStatus;
  }

  // Set protocol
  int version = m_preferences->getLdapVersion();
  ldap_set_option(ldapHandler, LDAP_OPT_PROTOCOL_VERSION, &version);

  // Prepare credentials
  struct berval cred;
  cred.bv_val = const_cast<char*>(password.c_str());
  cred.bv_len = password.length();

  // User authentication (bind)
  int rc = ldap_sasl_bind_s(ldapHandler, bindDn.toAscii().data(), LDAP_SASL_SIMPLE, &cred, NULL,NULL,NULL);
  if (rc == LDAP_SUCCESS) {
    resultStatus = true;
  } else {
    m_lastError = QString("Authentication failed: %1").arg(ldap_err2string(rc));
  }

  // Free ldap handler
  ldap_unbind_ext(ldapHandler, NULL, NULL);
  return resultStatus;
}
