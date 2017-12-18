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
# ------------------------------------------------------------------------ #
*/



#ifndef WEBDATASOURCESETTINGS_HPP
#define WEBDATASOURCESETTINGS_HPP

#include "WebBaseSettings.hpp"
#include "Validators.hpp"
#include <Wt/WLineEdit>
#include <Wt/WComboBox>
#include <Wt/WStringListModel>
#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WTemplate>
#include <Wt/WSpinBox>

class WebDataSourceSettings : public WebBaseSettings, public Wt::WTemplate
{
public:
  WebDataSourceSettings();
  virtual ~WebDataSourceSettings();
  Wt::Signal<int, std::string>& operationCompleted() { return m_operationCompleted; }
  void updateContents(void) { loadProperties();}
  void setEnabledInputs(bool enable);


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
  /** signals **/
  Wt::Signal<int, std::string> m_operationCompleted;

  /** other members **/
  Wt::WComboBox m_sourceSelectionBox;
  Wt::WStringListModel m_sourceBoxModel;
  Wt::WLineEdit m_monitorUrlField;
  Wt::WComboBox m_monitorTypeField;
  Wt::WLineEdit m_authStringField;
  Wt::WLineEdit m_livestatusHostField;
  Wt::WLineEdit m_livestatusPortField;
  Wt::WCheckBox m_showAuthStringField;
  Wt::WCheckBox m_useNgrt4ndField;
  Wt::WCheckBox m_dontVerifyCertificateField;
  Wt::WSpinBox m_updateIntervalField;
  Wt::WPushButton m_applyChangeBtn;
  Wt::WPushButton m_addAsSourceBtn;
  Wt::WPushButton m_deleteSourceBtn;
  Wt::WDialog m_sourceIndexSelector;


  void createFormWidgets(void);
  void bindFormWidgets(void);
  void unbindFormWidgets(void);
  void addEvent(void);
  bool validateSourceSettingsFields(void);
  void renderSourceIndexSelector(void);
  void handleAddAsSourceOkAction(Wt::WComboBox* inputBox);
  int getSourceGlobalIndex(int sourceBoxIndex);
  int findSourceIndexInBox(int sourceGlobalIndex);
  void addToSourceBox(int sourceGlobalIndex);
  void handleSourceBoxChanged(void);
  void showLivestatusSettings(int monitorTypeIndex);
  void handleShowAuthStringChanged(void);
};

#endif // WEBDATASOURCESETTINGS_HPP
