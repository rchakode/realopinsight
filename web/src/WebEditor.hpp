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
#include "WebInputSelector.hpp"
#include "WebTree.hpp"
#include <Wt/WGridLayout>
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
#include <Wt/WSuggestionPopup>
#include <Wt/WStringListModel>
#include <Wt/WFileUpload>


class WebEditor : public Wt::WContainerWidget
{

  enum {
    MENU_ADD_SUBSERVICE = 1,
    MENU_DELETE_SUBSERVICE
  };

public:
  WebEditor(void);
  virtual ~WebEditor();
  void rebuiltTree(void);
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
  WebTree m_tree;

  std::string m_currentFilePath;
  Wt::WModelIndex m_selectedTreeItemIndex;
  QString m_formerSelectedNodeId;

  Wt::WHBoxLayout* m_mainLayout;
  Wt::WTemplate m_fieldEditionPane;
  Wt::WLineEdit m_nameField;
  Wt::WComboBox m_iconBox;
  Wt::WComboBox m_calcRuleBox;
  Wt::WComboBox m_propRuleBox;
  Wt::WTextArea m_descField;

  // node type-related field and widgets
  Wt::WComboBox m_typeField;
  Wt::WComboBox m_typeExternalServiceSelectorField;
  Wt::WContainerWidget m_typeItemsContainer;
  Wt::WHBoxLayout* m_typeItemsLayout;

  // data point-related field and widgets
  Wt::WLineEdit m_dataPointField;
  Wt::WComboBox m_dataPointGroupField;
  Wt::WComboBox m_dataPointSourceField;
  Wt::WContainerWidget m_dataPointItemsContainer;
  Wt::WHBoxLayout* m_dataPointItemsLayout;
  std::unique_ptr<Wt::WSuggestionPopup> m_dataPointListPopup;
  std::unique_ptr<Wt::WStringListModel> m_dataPointListModel;
  QMap<std::string,  std::vector< Wt::WString > > m_dataPointsListByGroup;
  QMap<QString,  std::vector< Wt::WString > > m_dataPointsListBySource;

  Wt::WImage m_newServiceViewBtn;
  Wt::WImage m_openServiceViewBtn;
  Wt::WImage m_saveCurrentViewBtn;
  Wt::WImage m_importMonitoringConfigBtn;
  Wt::WImage m_importZabbixItServicesBtn;
  Wt::WImage m_importNagiosBpiBtn;

  InputSelector m_openViewDialog;
  InputSelector m_importMonitoringConfigDialog;
  InputSelector m_importZabbixItServicesDialog;
  InputSelector m_importNagiosBpiDialog;

  Wt::WPopupMenu m_editionContextMenu;
  Wt::WMenuItem* m_menuAddSubService;
  Wt::WMenuItem* m_menuDeleteService;

  QMap<QString, int> m_iconIndexMap;


  void bindMainPanes(void);
  void unbindWidgets(void);
  void addEvents(void);
  void buildTree(void);
  void bindFormWidgets(void);

  void configureTreeComponent(void);
  void enableContextMenus(void);
  void showTreeContextMenu(Wt::WModelIndex, Wt::WMouseEvent);
  void addSubServiceFromTreeNodeIndex(const Wt::WModelIndex& index);
  void removeServiceByTreeNodeIndex(const Wt::WModelIndex& index);
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
  void handleNodeTypeChanged(void);
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
  void importZabbixITServices(const QString& srcId);
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
