/*
 * WebEditor.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2018 Rodrigue Chakode                                      #
# Creation : 03-01-2018                                                    #
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

#include "dbo/src/DbSession.hpp"
#include "WebInputField.hpp"
#include "WebTree.hpp"
#include "WebMap.hpp"
#include <QMap>
#include <QHash>
#include <Wt/WGridLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WPushButton.h>
#include <Wt/WMenu.h>
#include <Wt/WMenuItem.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WLocalizedStrings.h>
#include <Wt/WLabel.h>
#include <Wt/WApplication.h>
#include <Wt/WTemplate.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>
#include <Wt/WComboBox.h>
#include <Wt/WSuggestionPopup.h>
#include <Wt/WStringListModel.h>
#include <Wt/WFileUpload.h>


class WebEditor : public Wt::WContainerWidget
{

  enum {
    MENU_ADD_SUBSERVICE = 1,
    MENU_DELETE_SUBSERVICE
  };

public:
  WebEditor(void);
  virtual ~WebEditor();
  void rebuildTree(void);
  Wt::Signal<int, std::string>& operationCompleted(void) {return m_operationCompleted;}
  void setDbSession(DbSession* _dbSession) {m_dbSession = _dbSession;}
  void setConfigDir(const QString& _dirPath) {m_configDir = _dirPath;}
  void refreshDynamicContents(void);

private:
  const static QMap<int, std::string> MENU_LABELS;
  Wt::Signal<int, std::string> m_operationCompleted;
  DbSession* m_dbSession;
  QString m_configDir;

  CoreDataT m_cdata;
  WebTree* m_treeRef;

  std::string m_currentFilePath;
  Wt::WModelIndex m_selectedTreeIndex;
  QString m_formerSelectedNodeId;

  Wt::WTemplate* m_editionPaneRef;
  Wt::WLineEdit* m_nameFieldRef;
  Wt::WComboBox* m_calcRuleFieldRef;
  Wt::WComboBox* m_propRuleFieldRef;
  Wt::WComboBox* m_iconFieldRef;
  Wt::WLineEdit* m_descFieldRef;
  Wt::WComboBox* m_typeFieldRef;
  Wt::WComboBox* m_externalTypeSelectorFieldRef;
  Wt::WLineEdit* m_checkFieldRef;
  Wt::WComboBox* m_checkGroupFieldRef;
  Wt::WComboBox* m_checkSourceFieldRef;
  Wt::WContainerWidget* m_checkSelectorPanelRef;
  Wt::WStringListModel* m_checkSuggestionModelRef;
  std::unique_ptr<Wt::WSuggestionPopup> m_checkSuggestionPanel;
  QMap<std::string,  std::vector< Wt::WString > > m_dataPointsListByGroup;
  QMap<QString,  std::vector< Wt::WString > > m_dataPointsListBySource;

  WebInputField m_viewSelector;
  WebInputField m_configImporter;
  WebInputField m_zabbixITViewSelector;
  WebInputField m_nagiosBPISelector;

  Wt::WPopupMenu m_contextMenus;
  Wt::WMenuItem* m_menuAddSubService;
  Wt::WMenuItem* m_menuDeleteService;

  QMap<QString, int> m_iconIndexMap;

  void addEvents(void);
  void buildTree(void);
  void showTreeContextMenu(Wt::WModelIndex, Wt::WMouseEvent);
  void addChildUnderTreeIndex(const Wt::WModelIndex& index);
  void removeNodeByTreeIndex(const Wt::WModelIndex& index);
  void fillInEditorFromCurrentSelection(void);
  void fillInEditorFromNodeInfo(const NodeT& ninfo);
  void updateNodeDataFromEditor(const QString& nodeId);
  void removeNodeFromCdata(const NodeT& ninfo);

  QList<NodeT> findDescendantNodes(const QString& nodeId);
  int findNodeDepth(const NodeT& ninfo);
  void bindParentChildEdges(void);

  void handleTreeContextMenu(Wt::WMenuItem*);
  void handleKeyPressed(const Wt::WKeyEvent& event);
  void handleTreeItemSelectionChanged(void);
  void handleNodeLabelChanged(void);
  void handleDataPointChanged(void);
  void handleNodeTypeChanged(int index);
  void handleNewViewButton(void);
  void handleSaveViewButton(void);
  void handleOpenViewButton(void);
  void handleOpenFile(const std::string& path, const std::string& option);
  void handleImportMonitoringConfigButton(void);
  void handleImportZabbixItServiceButton(void);
  void handleImportNagiosBpiButton(void);
  void handleDataPointSourceChanged(int index);
  void handleDataPointGroupChanged(int index);

  void importMonitoringConfig(const std::string& srcId, const std::string& groupFilter);
  void importNagiosBpi(const std::string& sourceId, const std::string& bpiConfigPath);
  void importZabbixITServices(const std::string& srcId);

  std::pair<int, QString> registerViewWithPath(const CoreDataT& cdata, const QString& destPath);

  void attachOrphanedNodesToRoot(NodeListT& nodes, NodeT& root);
  std::pair<int, std::string> extractNagiosBpiGroupMembers(const QString& parentServiceId,
                                    const QString& sourceId,
                                    const QString& groupMembers,
                                    NodeListT& bpnodes,
                                    NodeListT& cnodes,
                                    QString& childrenIds,
                                    bool& hasCluster);
};


#endif /* WEBEDITOR_HPP */
