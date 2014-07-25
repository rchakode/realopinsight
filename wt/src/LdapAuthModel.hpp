#ifndef LDAPAUTHMODEL_HPP
#define LDAPAUTHMODEL_HPP

#include "WebPreferences.hpp"
#include <Wt/Auth/AuthModel>
#include <Wt/Auth/Login>
#include <QString>
#include <string>

class LdapAuthModel : public Wt::Auth::AuthModel
{
public:
  LdapAuthModel(WebPreferences* preferences,
                const Wt::Auth::AuthService& baseAuth,
                Wt::Auth::AbstractUserDatabase& users,
                Wt::WObject* parent=0);
  //  virtual void reset();
  //  virtual bool isVisible(Wt::WFormModel::Field field) const;
  //  virtual bool validateField(Wt::WFormModel::Field field);
  virtual bool validate();
  //  virtual void configureThrottling(Wt::WInteractWidget* button);
  //  virtual void updateThrottling(Wt::WInteractWidget* button);
  virtual bool login(Wt::Auth::Login& login);
  virtual void logout(Wt::Auth::Login& login);
  virtual Wt::Auth::EmailTokenResult processEmailToken(const std::string& token);
  virtual Wt::Auth::User processAuthToken();

private:
  QString m_lastError;
  WebPreferences* m_preferences;
  bool ldapLogin(const std::string& username, const std::string& password);
};

#endif // LDAPAUTHMODEL_HPP
