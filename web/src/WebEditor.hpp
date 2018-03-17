/*
 * WebEditor.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2018 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)        #
# Last Update : 03-01-2018                                                 #
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

#ifndef WEBEDITOR_HPP
#define WEBEDITOR_HPP

#include "dbo/DbSession.hpp"
#include "WebInputSelector.hpp"
#include "WebTree.hpp"
#include <Wt/WGridLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WMenu>
#include <Wt/WMenuItem>
#include <Wt/WStackedWidget>
#include "WebMap.hpp"
#include <Wt/WLocalizedStrings>
#include <Wt/WLabel>
#include <QHash>
#include <Wt/WApplication>
#include <Wt/WTemplate>
#include <Wt/WLineEdit>
#include <Wt/WTextArea>
#include <Wt/WComboBox>
#include <QMap>


class WebEditor : public Wt::WContainerWidget
{

  enum {
    MENU_ADD_SUBSERVICE = 1,
    MENU_DELETE_SUBSERVICE
  };

public:
  WebEditor(void);
  virtual ~WebEditor();
  void refreshContent(void);
  Wt::Signal<int, std::string>& operationCompleted(void) {return m_operationCompleted;}
  void setDbSession(DbSession* _dbSession) {m_dbSession = _dbSession;}
  void setConfigDir(const QString& _dirPath) {m_configDir = _dirPath;}

private:
  const static QMap<int, std::string> MENU_LABELS;

  DbSession* m_dbSession;
  QString m_configDir;

  Wt::Signal<int, std::string> m_operationCompleted;

  CoreDataT m_cdata;
  WebTree m_tree;

  std::string m_currentFilePath;
  Wt::WModelIndex m_currentTreeItemIndex;
  QString m_formerSelectedNodeId;

  Wt::WHBoxLayout* m_mainLayout;
  Wt::WTemplate m_fieldEditionPane;
  Wt::WLineEdit m_nameField;
  Wt::WComboBox m_typeField;
  Wt::WComboBox m_iconBox;
  Wt::WComboBox m_calcRuleBox;
  Wt::WComboBox m_propRuleBox;
  Wt::WTextArea m_descField;
  Wt::WLineEdit m_dataPointField;

  Wt::WImage m_newServiceViewBtn;
  Wt::WImage m_openServiceViewBtn;
  Wt::WImage m_saveCurrentViewBtn;
  Wt::WImage m_importNativeConfigBtn;

  InputSelector m_openViewSelector;
  InputSelector m_importNativeConfigSelector;

  Wt::WPopupMenu m_editionContextMenu;
  Wt::WMenuItem* m_menuAddSubService;
  Wt::WMenuItem* m_menuDeleteService;

  QMap<QString, int> m_iconIndexMap;


  void bindMainPanes(void);
  void unbindWidgets(void);
  void addEvents(void);
  void buildTree(void);
  void bindFormWidgets(void);


  void prepareTreeToEdition(void);
  void showTreeContextMenu(Wt::WModelIndex, Wt::WMouseEvent);
  void addNewSubService(const Wt::WModelIndex& currentTreeItemIndex);
  void fillInEditorFromCurrentSelection(void);
  void fillInEditorFromNodeInfo(const NodeT& ninfo);
  void updateNodeDataFromEditor(const QString& nodeId);

  void fixChildParentDependencies(void);
  void setParentChildDependency(const QString& childId, const QString& parentId);


  void handleTreeContextMenu(Wt::WMenuItem*);
  void handleKeyPressed(const Wt::WKeyEvent& event);
  void handleTreeItemSelectionChanged(void);
  void handleNodeLabelChanged(void);
  void handleNewView(void);
  void handleSaveView(void);
  void handleOpenViewButton(void);
  void handleOpenFile(const std::string& path, const std::string& option);
  void handleImportNativeConfigButton(void);

  std::pair<int, QString> saveContentToFile(const CoreDataT& cdata, const QString& destPath);
  void importNativeConfig(const SourceT& sinfo, const QString& hostgroup);
};


#endif /* WEBEDITOR_HPP */
