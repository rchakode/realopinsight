/*
 * WebSettingUI.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 23-03-2014                                                 #
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

#ifndef WEBSESSION_HPP
#define WEBSESSION_HPP

#include "Preferences.hpp"
#include <Wt/WDialog>
#include <Wt/WRadioButton>
#include <Wt/WLineEdit>
#include <Wt/WComboBox>
#include <Wt/WPushButton>
#include <Wt/WSpinBox>
#include <Wt/WCheckBox>
#include <Wt/WObject>
#include <Wt/WStringListModel>
#include <Wt/WLengthValidator>
#include <memory>

class QString;

class WebPreferencesBase : public Preferences
{
public:
  enum AuthenticationModeT {
    BuiltIn = 0,
    LDAP = 1
  };

  enum NotificationTypeT {
    NoNotification = 0,
    EmailNotification = 1
  };

  WebPreferencesBase(void);
  int getDbState(void) { return value(Settings::GLOBAL_DB_STATE_KEY, "0").toInt();}
  void setDbState(int state) {setEntry(Settings::GLOBAL_DB_STATE_KEY, QString::number(state));}

  std::string getLdapServerUri(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_SERVER_URI).toStdString();}
  std::string getLdapBindUserDn(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_BIND_USER_DN).toStdString();}
  std::string getLdapSearchBase(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_SEARCH_BASE).toStdString();}
  std::string getLdapBindUserPassword(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_BIND_USER_PASSWORD).toStdString();}
  std::string getLdapIdField(void) const;
  int getLdapVersion(void) const;
  int getAuthenticationMode(void) const;
  bool getLdapSslUseMyCert(void) const {return m_settings->keyValue(Settings::AUTH_LDAP_SSL_USE_CERT).toInt() == Wt::Checked;}
  std::string getLdapSslCertFile(void) const {return m_settings->keyValue(Settings::AUTH_LDAP_SSL_CERT_FILE).toStdString();}
  std::string getLdapSslCaFile(void) const {return m_settings->keyValue(Settings::AUTH_LDAP_SSL_CA_FILE).toStdString();}
  static std::string authTypeString(int authSystem) {return (authSystem == LDAP) ? "LDAP" : "Built-in";}
  int getNotificationType(void) const { return m_settings->keyValue(Settings::NOTIF_TYPE).toInt();}
  std::string getSmtpServerAddr(void) const { return m_settings->keyValue(Settings::NOTIF_MAIL_SMTP_SERVER_ADRR).toStdString();}
  std::string getSmtpServerPort(void) const { return m_settings->keyValue(Settings::NOTIF_MAIL_SMTP_SERVER_PORT).toStdString();}
  std::string getSmtpUsername(void) const { return m_settings->keyValue(Settings::NOTIF_MAIL_SMTP_USERNAME).toStdString();}
  std::string getSmtpPassword(void) const { return m_settings->keyValue(Settings::NOTIF_MAIL_SMTP_PASSWORD).toStdString();}
  int getSmtpUseSsl(void) const { return m_settings->keyValue(Settings::NOTIF_MAIL_SMTP_USE_SSL).toInt();}


protected :
  virtual void fillFromSource(int srcIndex) {}
  virtual void updateAllSourceWidgetStates(void) {}
  virtual void updateFields(void) {}
  virtual void saveAsSource(const qint32& idx, const QString& type){}

protected Q_SLOTS:
  virtual void applyChanges(void) {}
  virtual void handleCancel(void) {}
  virtual void addAsSource(void) {}
  virtual void deleteSource(void){}
};

class WebPreferences : public WebPreferencesBase, public Wt::WContainerWidget
{
public:
  enum InputTypeT {
    SourceTypeInput=1,
    SourceIndexInput=2
  };


  WebPreferences(void);
  virtual ~WebPreferences();

  Wt::Signal<std::string>& errorOccurred() { return m_errorOccurred; }
  Wt::Signal<int>& authSystemChanged() { return m_authSystemChanged; }

  virtual void setEnabledInputs(bool enable);
  void hideUnrequiredFields(void);
  void showMonitoringSettings(void);
  void showMonitoringSettingsWidgets(bool display);
  void showAuthSettings(void);
  void showAuthSettingsWidgets(bool display);
  void showNotificationSettings(void);
  void showNotificationSettingsWidgets(bool display);

protected:
  virtual void applyChanges(void);
  void handleCancel(void) {return;}
  virtual void fillFromSource(int _index);
  virtual void updateAllSourceWidgetStates(void);
  virtual void updateFields(void);
  virtual void saveAsSource(const qint32& index, const QString& type);
  virtual void addAsSource(void);
  virtual void deleteSource(void);

private:
  Wt::Signal<std::string> m_errorOccurred;
  Wt::Signal<int> m_authSystemChanged;

  // monitoring settings properties
  std::unique_ptr<Wt::WComboBox> m_sourceBox;
  std::unique_ptr<Wt::WStringListModel> m_sourceBoxModel;
  std::unique_ptr<Wt::WLineEdit> m_monitorUrlField;
  std::unique_ptr<Wt::WLineEdit> m_authStringField;
  std::unique_ptr<Wt::WLineEdit> m_livestatusHostField;
  std::unique_ptr<Wt::WLineEdit> m_livestatusPortField;
  std::unique_ptr<Wt::WComboBox> m_monitorTypeField;
  std::unique_ptr<Wt::WCheckBox> m_showAuthStringField;
  std::unique_ptr<Wt::WCheckBox> m_useNgrt4ndField;
  std::unique_ptr<Wt::WCheckBox> m_dontVerifyCertificateField;
  std::unique_ptr<Wt::WSpinBox> m_updateIntervalField;
  std::unique_ptr<Wt::WPushButton> m_applyChangeBtn;
  std::unique_ptr<Wt::WPushButton> m_addAsSourceBtn;
  std::unique_ptr<Wt::WPushButton> m_deleteSourceBtn;

  // auth settings properties
  std::unique_ptr<Wt::WComboBox> m_authenticationModeField;
  std::unique_ptr<Wt::WLineEdit> m_ldapServerUriField;
  std::unique_ptr<Wt::WComboBox> m_ldapVersionField;
  std::unique_ptr<Wt::WLineEdit> m_ldapBindUserDnField;
  std::unique_ptr<Wt::WLineEdit> m_ldapBindUserPasswordField;
  std::unique_ptr<Wt::WLineEdit> m_ldapIdField;
  std::unique_ptr<Wt::WLineEdit> m_ldapSearchBaseField;
  std::unique_ptr<Wt::WPushButton> m_authSettingsSaveBtn;
  std::unique_ptr<Wt::WCheckBox> m_ldapSslUseCertField;
  std::unique_ptr<Wt::WLineEdit> m_ldapSslCertFileField;
  std::unique_ptr<Wt::WLineEdit> m_ldapSslCaFileField;

  // notification settings properties
  std::unique_ptr<Wt::WComboBox> m_notificationTypeBox;
  std::unique_ptr<Wt::WLineEdit> m_smtpServerAddrField;
  std::unique_ptr<Wt::WLineEdit> m_smtpServerPortField;
  std::unique_ptr<Wt::WCheckBox> m_smtpUseSslField;
  std::unique_ptr<Wt::WLineEdit> m_smtpUsernameField;
  std::unique_ptr<Wt::WLineEdit> m_smtpPasswordField;
  std::unique_ptr<Wt::WPushButton> m_notificationSettingsSaveBtn;

  void createLdapSettingsFields(void);
  void createAuthSettingsFields(void);
  void createSourceSettingsFields(void);
  void createNotificationSettingsFields(void);
  void createButtons(void);

  void addEvent(void);
  void promptUser(int inputType);
  void handleInput(const std::string& input, int inputType);
  int getSourceGlobalIndex(int sourceBoxIndex);
  int findSourceIndexInBox(int sourceGlobalIndex);
  void addToSourceBox(int sourceGlobalIndex);
  void bindFormWidget(void);
  void saveAuthSettings(void);
  void saveNotificationSettings(void);
  void fillInAuthSettings(void);
  void fillInNotificationSettings(void);
  bool validateMonitoringSettingsFields(void);
  bool validateAuthSettingsFields(void);
  void showLdapSslSettings(bool display);
  void showLivestatusSettings(int monitorTypeIndex);
  void updateEmailFieldsEnabledState(void);
  void handleSourceBoxChanged(void) { fillFromSource(getSourceGlobalIndex(m_sourceBox->currentIndex()));}
  void handleAuthTypeChanged(void);
  void handleShowAuthStringChanged(void);
  void handleLdapUseSslChanged(void);
};

#endif // WEBSESSION_HPP
