/*
 * WebAuthPreferences.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Creation: 21-06-2015                                                     #
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

#ifndef WEBAUTHPREFERENCES_HPP
#define WEBAUTHPREFERENCES_HPP

#include "WebPreferencesBase.hpp"
#include <Wt/WTemplate>
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


class WebAuthPreferences : public WebPreferencesBase, public Wt::WTemplate
{
public:
  WebAuthPreferences(void);
  virtual ~WebAuthPreferences();
  void updateContents(void) { updateFields(); }
  Wt::Signal<int, std::string>& operationCompleted() { return m_operationCompleted; }
  Wt::Signal<int>& authSystemChanged() { return m_authSystemChanged; }


protected:
  virtual void updateFields(void);

private:
  Wt::Signal<int, std::string> m_operationCompleted;
  Wt::Signal<int> m_authSystemChanged;

  // auth settings properties
  Wt::WComboBox m_authenticationModeField;
  Wt::WLineEdit m_ldapServerUriField;
  Wt::WComboBox m_ldapVersionField;
  Wt::WLineEdit m_ldapBindUserDnField;
  Wt::WLineEdit m_ldapBindUserPasswordField;
  Wt::WLineEdit m_ldapIdField;
  Wt::WLineEdit m_ldapSearchBaseField;
  Wt::WPushButton m_authSettingsSaveBtn;
  Wt::WCheckBox m_ldapSslUseCertField;
  Wt::WLineEdit m_ldapSslCertFileField;
  Wt::WLineEdit m_ldapSslCaFileField;
  Wt::WLineEdit m_authStringField;
  Wt::WCheckBox m_showAuthStringField;


  void createLdapSettingsFields(void);
  void createAuthSettingsFields(void);
  void createButtons(void);

  void addEvent(void);
  void bindFormWidgets(void);
  void unbindFormWidgets(void);
  void saveChanges(void);
  bool validateAuthSettingsFields(void);
  void showLdapSslSettings(bool display);
  void handleAuthTypeChanged(void);
  void handleShowAuthStringChanged(void);
  void handleLdapUseSslChanged(void);
};

#endif // WEBAUTHPREFERENCES_HPP
