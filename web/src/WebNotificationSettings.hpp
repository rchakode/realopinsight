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

#ifndef WEBNOTIFICATIONSETTINGS_HPP
#define WEBNOTIFICATIONSETTINGS_HPP

#include "WebBaseSettings.hpp"
#include <memory>
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
#include <Wt/WTemplate.h>


class WebNotificationSettings : public WebBaseSettings, public Wt::WTemplate
{
  Q_OBJECT
public:
  WebNotificationSettings(void);
  virtual ~WebNotificationSettings(void);
  void updateContents(void) { updateFields(); }
  Wt::Signal<int, std::string>& operationCompleted() { return m_operationCompleted; }


protected:
  virtual void updateFields(void);

private:
  Wt::Signal<int, std::string> m_operationCompleted;
  Wt::WComboBox* m_notificationTypeFieldRef;
  Wt::WLineEdit* m_smtpServerFieldRef;
  Wt::WLineEdit* m_smtpPortFieldRef;
  Wt::WCheckBox* m_smtpUseSslFieldRef;
  Wt::WLineEdit* m_smtpUserFieldRef;
  Wt::WLineEdit* m_smtpPassFieldRef;
  Wt::WPushButton* m_applyBtnRef;

  void saveChanges(void);
  void updateEmailFieldsEnabledState(void);
  void handleLdapUseSslChanged(void);
};

#endif // WEBNOTIFICATIONSETTINGS_HPP
