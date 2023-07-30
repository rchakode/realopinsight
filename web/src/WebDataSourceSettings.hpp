/*
# ------------------------------------------------------------------------ #
# Copyright (c) 2018 Rodrigue Chakode                                      #
# Creation : 17-12-2017                                                    #
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
#include <Wt/WLineEdit.h>
#include <Wt/WComboBox.h>
#include <Wt/WStringListModel.h>
#include <Wt/WPushButton.h>
#include <Wt/WDialog.h>
#include <Wt/WTemplate.h>
#include <Wt/WSpinBox.h>

class WebDataSourceSettings : public WebBaseSettings, public Wt::WTemplate
{
public:
  WebDataSourceSettings();
  virtual ~WebDataSourceSettings();
  Wt::Signal<int, std::string>& operationCompleted() { return m_operationCompleted; }
  void updateContents(void) { loadProperties();}
  void setEnabledInputs(bool enable);
  void setDbSession(DbSession* dbSession) {m_dbSession =  dbSession;}


protected:
  virtual void applyChanges(void);
  void applySourceChanges(int index);
  void handleCancel(void) {return;}
  virtual void fillInFormGivenSourceId(int sid);
  virtual void updateAllSourceWidgetStates(void);
  virtual void updateFields(void);
  virtual void addAsSource(void);
  virtual void deleteSource(void);


private:
  Wt::Signal<int, std::string> m_operationCompleted;
  Wt::WComboBox* m_sourceSelectionFieldRef;
  std::shared_ptr<Wt::WStringListModel> m_sourceDataModel;
  Wt::WLineEdit* m_monitorUrlFieldRef;
  Wt::WComboBox* m_monitorTypeFieldRef;
  Wt::WLineEdit* m_authStringFieldRef;
  Wt::WLineEdit* m_livestatusHostFieldRef;
  Wt::WLineEdit* m_livestatusPortFieldRef;
  Wt::WCheckBox* m_showAuthStringFieldRef;
  Wt::WCheckBox* m_dontVerifyCertificateFieldRef;
  Wt::WSpinBox* m_updateIntervalFieldRef;
  Wt::WPushButton* m_applyBtnRef;
  Wt::WPushButton* m_addAsNewBtnRef;
  Wt::WPushButton* m_deleteBtnRef;
  Wt::WComboBox* m_sourceIndexFieldRef;
  Wt::WDialog m_sourceIndexSelector;
  DbSession* m_dbSession;


  bool validateSourceSettingsFields(void);
  void handleAddAsSource(Wt::WMouseEvent);
  int getSourceId(int sid);
  int findFormSourceIndex(int sid);
  void updateSourceDataModel(int sid);
  void handleSourceBoxChanged(void);
  void updateComponentsVisibility(int monitorTypeCurrentIndex);
  void handleShowAuthStringChanged(void);
  void saveSourceInDatabase(const SourceT& sinfo);
  SourceT getSourceInfoByIndex(int sourceIndex);
};

#endif // WEBDATASOURCESETTINGS_HPP
