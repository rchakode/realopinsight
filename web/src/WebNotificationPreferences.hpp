/*
 * WebPreferencesBase.hpp
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

#ifndef WEBNOTIFICATIONPREFERENCES_HPP
#define WEBNOTIFICATIONPREFERENCES_HPP

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
#include <Wt/WTemplate>
#include <memory>


class WebNotificationPreferences : public WebPreferencesBase, public Wt::WTemplate
{
public:
  WebNotificationPreferences(void);
  virtual ~WebNotificationPreferences(void);
  void updateContents(void) { updateFields(); }
  Wt::Signal<int, std::string>& operationCompleted() { return m_operationCompleted; }

protected:
  virtual void updateFields(void);

private:
  /** Signal **/
  Wt::Signal<int, std::string> m_operationCompleted;

  Wt::WComboBox m_notificationTypeBox;
  Wt::WLineEdit m_smtpServerAddrField;
  Wt::WLineEdit m_smtpServerPortField;
  Wt::WCheckBox m_smtpUseSslField;
  Wt::WLineEdit m_smtpUsernameField;
  Wt::WLineEdit m_smtpPasswordField;
  Wt::WPushButton m_notificationSettingsSaveBtn;

  void createFormWidgets(void);

  void addEvent(void);
  void bindFormWidgets(void);
  void unbindFormWidgets(void);
  void saveChanges(void);
  void updateEmailFieldsEnabledState(void);
  void handleLdapUseSslChanged(void);
};

#endif // WEBNOTIFICATIONPREFERENCES_HPP
