#ifndef WEBLICENSEACTIVATION_HPP
#define WEBLICENSEACTIVATION_HPP
#include "WebPreferencesBase.hpp"
#include <Wt/WTemplate>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>

class WebLicenseActivation : public WebPreferencesBase, public Wt::WTemplate
{
public:
  WebLicenseActivation();
  void saveActivationKey(void);
  bool isActivated(const QString& version);

private:
  Wt::WLineEdit* m_activationKeyField;
  Wt::WPushButton* m_activeBtn;

  QString genLicenseKey(const QString& hostid, const QString& hostname, const QString& version);
  bool checkLicenseKey(const QString& key, const QString& version);
  bool isValidKey(const QString& key, const QString& hostid, const QString& hostname, const QString& version);
  QString getHostName(void);
  QString getHostId(void);

};

#endif // WEBLICENSEACTIVATION_HPP
