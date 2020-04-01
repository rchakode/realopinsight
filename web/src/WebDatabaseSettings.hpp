/*
 * WebPreferencesBase.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2017 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)        #
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

#ifndef WEBPOSTGRESQLSETTINGS_HPP
#define WEBPOSTGRESQLSETTINGS_HPP

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


class WebDatabaseSettings : public WebBaseSettings, public Wt::WTemplate
{
  Q_OBJECT
public:
  WebDatabaseSettings(void);
  virtual ~WebDatabaseSettings(void);
  void updateContents(void);
  Wt::Signal<int, std::string>& operationCompleted() { return m_operationCompleted; }


Q_SIGNALS:
  void operationCompleted(int, std::string);

protected:
  virtual void updateFields(void);

private:
  Wt::Signal<int, std::string> m_operationCompleted;
  Wt::WComboBox* m_dbTypeBoxRef;
  Wt::WLineEdit* m_dbServerFieldRef;
  Wt::WLineEdit* m_dbPortFieldRef;
  Wt::WLineEdit* m_dbNameFieldRef;
  Wt::WCheckBox* m_dbEmptyStateRef;
  Wt::WLineEdit* m_dbUserFieldRef;
  Wt::WLineEdit* m_dbPassFieldRef;
  Wt::WPushButton* m_applyBtnRef;

  void applySettings(void);
  void updateFieldEnabledState(void);
};

#endif // WEBPOSTGRESQLSETTINGS_HPP
