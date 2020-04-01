/*
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Change: 17-12-2017                                                  #
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

#ifndef WEBAUTHSETTINGS_HPP
#define WEBAUTHSETTINGS_HPP

#include "WebBaseSettings.hpp"
#include <memory>
#include <Wt/WTemplate.h>
#include <Wt/WDialog.h>
#include <Wt/WRadioButton.h>
#include <Wt/WLineEdit.h>
#include <Wt/WComboBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WSpinBox.h>
#include <Wt/WCheckBox.h>
#include <Wt/WObject.h>
#include <Wt/WStringListModel.h>
#include <Wt/WLengthValidator.h>

class WebAuthSettings : public WebBaseSettings, public Wt::WTemplate
{
public:
  WebAuthSettings(void);
  virtual ~WebAuthSettings();
  void updateContents(void) { updateFields(); }
  Wt::Signal<int, std::string>& operationCompleted() { return m_operationCompleted; }
  Wt::Signal<int>& authSystemChanged() { return m_authSystemChanged; }


protected:
  virtual void updateFields(void);

private:
  Wt::Signal<int, std::string> m_operationCompleted;
  Wt::Signal<int> m_authSystemChanged;

  // auth settings properties
  Wt::WComboBox* m_authModeFieldRef;
  Wt::WLineEdit* m_ldapServerURIFieldRef;
  Wt::WComboBox* m_ldapVersionFieldRef;
  Wt::WLineEdit* m_ldapBindDnFieldRef;
  Wt::WLineEdit* m_ldapBindPassFieldRef;
  Wt::WLineEdit* m_ldapUIDFieldRef;
  Wt::WLineEdit* m_ldapSearchBaseFieldRef;
  Wt::WPushButton* m_applyBtnRef;
  Wt::WCheckBox* m_ldapSslUseCertFieldRef;
  Wt::WLineEdit* m_ldapSslCertFileFieldRef;
  Wt::WLineEdit* m_ldapSslCaFileFieldRef;

  void saveChanges(void);
  bool validateAuthSettingsFields(void);
  void handleAuthTypeChanged(void);
  void handleLdapUseSslChanged(void);
};

#endif // WEBAUTHSETTINGS_HPP
