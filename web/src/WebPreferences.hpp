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

#include "WebPreferencesBase.hpp"
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


class WebPreferences : public WebPreferencesBase, public Wt::WContainerWidget
{
public:
  WebPreferences(void);
  virtual ~WebPreferences();

  Wt::Signal<int, std::string>& operationCompleted() { return m_operationCompleted; }
  Wt::Signal<int>& authSystemChanged() { return m_authSystemChanged; }

  virtual void setEnabledInputs(bool enable);
  void hideUnrequiredFields(void);
  void showSourceSettings(void);
  void showSourcesSettingsWidgets(bool display);
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
  Wt::Signal<int, std::string> m_operationCompleted;
  Wt::Signal<int> m_authSystemChanged;

  // source settings properties
  std::unique_ptr<Wt::WComboBox> m_sourceSelectionBox;
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
  std::unique_ptr<Wt::WDialog> m_sourceIndexSelector;

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
  Wt::WDialog* createSourceIndexSelector(void);
  void handleAddAsSourceOkAction(Wt::WComboBox* inputBox);
  int getSourceGlobalIndex(int sourceBoxIndex);
  int findSourceIndexInBox(int sourceGlobalIndex);
  void addToSourceBox(int sourceGlobalIndex);
  void bindFormWidget(void);
  void saveAuthSettings(void);
  void saveNotificationSettings(void);
  void fillInAuthSettings(void);
  void fillInNotificationSettings(void);
  bool validateSourceSettingsFields(void);
  bool validateAuthSettingsFields(void);
  void showLdapSslSettings(bool display);
  void showLivestatusSettings(int monitorTypeIndex);
  void updateEmailFieldsEnabledState(void);
  void handleSourceBoxChanged(void) { fillFromSource(getSourceGlobalIndex(m_sourceSelectionBox->currentIndex()));}
  void handleAuthTypeChanged(void);
  void handleShowAuthStringChanged(void);
  void handleLdapUseSslChanged(void);
};

#endif // WEBSESSION_HPP
