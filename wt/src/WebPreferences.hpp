/*
 * WebSettingUI.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 29-12-2013                                                 #
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
#include <memory>

class QString;

class WebPreferences : public Preferences, public Wt::WObject
{
public:
  enum InputTypeT {
    SourceTypeInput=1,
    SourceIndexInput=2
  };

  WebPreferences(void);
  virtual ~WebPreferences();
  void show(void) {m_dialog->show();}
  Wt::WWidget* getWidget(void) {return m_dialog->contents();}
  void setEnabledInputs(bool enable);

protected :
  virtual void applyChanges(void);
  void handleCancel(void) {m_dialog->accept();}
  virtual void fillFromSource(int _sidx);
  virtual void updateSourceBtnState(void);
  virtual void updateFields(void);
  virtual void saveAsSource(const qint32& index, const QString& type);
  virtual void addAsSource(void);
  virtual void deleteSource(void);

private:
  std::shared_ptr<Wt::WDialog> m_dialog;
  std::shared_ptr<Wt::WText> m_infoBox;
  std::shared_ptr<Wt::WComboBox> m_sourceBox;
  std::shared_ptr<Wt::WStringListModel> m_sourceBoxModel;
  std::shared_ptr<Wt::WLineEdit> m_monitorUrlField;
  std::shared_ptr<Wt::WLineEdit> m_authStringField;
  std::shared_ptr<Wt::WLineEdit> m_livestatusHostField;
  std::shared_ptr<Wt::WLineEdit> m_livestatusPortField;
  std::shared_ptr<Wt::WComboBox> m_monitorTypeField;
  std::shared_ptr<Wt::WCheckBox> m_showAuthStringField;
  std::shared_ptr<Wt::WCheckBox> m_useNgrt4ndField;
  std::shared_ptr<Wt::WCheckBox> m_dontVerifyCertificateField;
  std::shared_ptr<Wt::WSpinBox> m_updateIntervalField;
  std::shared_ptr<Wt::WPushButton> m_applyChangeBtn;
  std::shared_ptr<Wt::WPushButton> m_addAsSourceBtn;
  std::shared_ptr<Wt::WPushButton> m_deleteSourceBtn;

  void promptUser(int inputType);
  void handleInput(const std::string& input, int inputType);
};

#endif // WEBSESSION_HPP
