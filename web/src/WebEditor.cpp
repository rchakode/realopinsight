/*
 * WebEditor.cpp
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

#include "WebUtils.hpp"
#include "WebEditor.hpp"
#include "Base.hpp"
#include "utilsCore.hpp"
#include "Parser.hpp"
#include "WebBaseSettings.hpp"
#include "WebInputSelector.hpp"
#include "ZbxHelper.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <fstream>
#include <QDebug>
#include <Wt/WApplication>
#include <Wt/WPanel>
#include <Wt/WPointF>
#include <Wt/WText>
#include <Wt/WLink>
#include <Wt/WImage>
#include <Wt/WTemplate>
#include <Wt/WPoint>



namespace {
  const IconMapT ICONS = ngrt4n::nodeIcons();
}

const QMap<int, std::string> WebEditor::MENU_LABELS = {
  {WebEditor::MENU_ADD_SUBSERVICE, Q_TR("Add sub service")},
  {WebEditor::MENU_DELETE_SUBSERVICE, Q_TR("Delete sub service")}
};

WebEditor::WebEditor(void) :
  m_operationCompleted(this)
{
  configureTreeComponent();
  enableContextMenus();
  bindMainPanes();
  bindFormWidgets();
}

WebEditor::~WebEditor()
{
  unbindWidgets();
}


void WebEditor::enableContextMenus() {
  m_editionContextMenu.addItem("images/plus.png", MENU_LABELS[MENU_ADD_SUBSERVICE])
      ->triggered().connect(this, &WebEditor::handleTreeContextMenu);

  m_editionContextMenu.addItem("images/minus.png", MENU_LABELS[MENU_DELETE_SUBSERVICE])
      ->triggered().connect(this, &WebEditor::handleTreeContextMenu);
}

void WebEditor::configureTreeComponent()
{
  m_tree.setCdata(&m_cdata);
  m_tree.activateEditionFeatures();

  m_tree.selectionChanged().connect(this, &WebEditor::handleTreeItemSelectionChanged);
  m_tree.doubleClicked().connect(this, &WebEditor::showTreeContextMenu);
  m_tree.keyPressed().connect(this, &WebEditor::handleKeyPressed);
}


void  WebEditor::handleOpenViewButton(void)
{
  m_openViewDialog.updateContentWithViewList(m_dbSession->listViews());
  m_openViewDialog.show();
}


void  WebEditor::handleOpenFile(const std::string& path, const std::string&)
{
  Parser parser(path.c_str(), &m_cdata, Parser::ParsingModeEditor, ngrt4n::DotLayout);

  int rc = parser.process();

  if (rc != 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, parser.lastErrorMsg().toStdString());
    return ;
  }

  rebuiltTree();

  m_currentFilePath = path;
}


void WebEditor::bindMainPanes(void)
{
  setLayout(m_mainLayout = new Wt::WHBoxLayout());

  m_mainLayout->setContentsMargins(0, 0, 0, 0);

  m_mainLayout->addWidget(&m_tree);
  m_mainLayout->addWidget(&m_fieldEditionPane);

  m_mainLayout->setSpacing(2);
  m_mainLayout->setResizable(0);
  m_mainLayout->setResizable(1);
}


void WebEditor::unbindWidgets(void)
{
  m_mainLayout->removeWidget(&m_tree);
  m_mainLayout->removeWidget(&m_fieldEditionPane);
  m_dataPointItemsLayout->removeWidget(&m_dataPointField);
  m_dataPointItemsLayout->removeWidget(&m_dataPointSourceField);
  m_dataPointItemsLayout->removeWidget(&m_dataPointGroupField);
  m_typeItemsLayout->removeWidget(&m_typeField);
  m_typeItemsLayout->removeWidget(&m_typeExternalServiceNameField);
  clear();
}



void WebEditor::bindFormWidgets(void)
{
  m_fieldEditionPane.setTemplateText(Wt::WString::tr("editor-fields-form.tpl"));

  // new service button
  m_newServiceViewBtn.setToolTip(Q_TR("Create a new service view"));
  m_newServiceViewBtn.setImageLink(Wt::WLink("images/built-in/new.png"));
  m_newServiceViewBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("new-service-view", &m_newServiceViewBtn);
  // bind signal
  m_newServiceViewBtn.clicked().connect(this, &WebEditor::handleNewViewButton);

  // open service button
  m_openServiceViewBtn.setToolTip(Q_TR("Open and edit an existing service view"));
  m_openServiceViewBtn.setImageLink(Wt::WLink("images/built-in/open.png"));
  m_openServiceViewBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("open-service-view", &m_openServiceViewBtn);
  // connect signals
  m_openServiceViewBtn.clicked().connect(this, &WebEditor::handleOpenViewButton);
  m_openViewDialog.dataTriggered().connect(this, &WebEditor::handleOpenFile);

  // save service button
  m_saveCurrentViewBtn.setToolTip(Q_TR("Save changes"));
  m_saveCurrentViewBtn.setImageLink(Wt::WLink("images/built-in/save.png"));
  m_saveCurrentViewBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("save-current-view", &m_saveCurrentViewBtn);
  // connect signals
  m_saveCurrentViewBtn.clicked().connect(this, &WebEditor::handleSaveViewButton);

  // import native monitoring data button
  m_importMonitoringConfigBtn.setToolTip(Q_TR("Import of monitoring data as group-based tree"));
  m_importMonitoringConfigBtn.setImageLink(Wt::WLink("images/built-in/import-monitoring-data.png"));
  m_importMonitoringConfigBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("import-native-config", &m_importMonitoringConfigBtn);
  // connect signal
  m_importMonitoringConfigBtn.clicked().connect(this, &WebEditor::handleImportMonitoringConfigButton);
  m_importMonitoringConfigDialog.dataTriggered().connect(this, &WebEditor::importMonitoringConfig);

  // import Zabbix IT Services
  m_importZabbixItServicesBtn.setToolTip(Q_TR("Import Zabbix IT services"));
  m_importZabbixItServicesBtn.setImageLink(Wt::WLink("images/built-in/import-zabbix_32x32.png"));
  m_importZabbixItServicesBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("import-zabbix-it-service", &m_importZabbixItServicesBtn);
  // connect signal
  m_importZabbixItServicesBtn.clicked().connect(this, &WebEditor::handleImportZabbixItServiceButton);
  m_importZabbixItServicesDialog.itemTriggered().connect(this, &WebEditor::importZabbixITServices);


  // import Nagios BPI button
  m_importNagiosBpiBtn.setToolTip(Q_TR("Import Nagios BPI"));
  m_importNagiosBpiBtn.setImageLink(Wt::WLink("images/built-in/import-nagios_32x32.png"));
  m_importNagiosBpiBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("import-nabios-bpi", &m_importNagiosBpiBtn);
  // connect signal
  m_importNagiosBpiBtn.clicked().connect(this, &WebEditor::handleImportNagiosBpiButton);
  m_importNagiosBpiDialog.fileUploaded().connect(this, &WebEditor::importNagiosBpi);

  // name field
  m_fieldEditionPane.bindWidget("name-field", &m_nameField);
  m_nameField.blurred().connect(this, &WebEditor::handleNodeLabelChanged);


  // set node type values
  m_fieldEditionPane.bindWidget("type-field", &m_typeItemsContainer);

  m_typeItemsContainer.setLayout(m_typeItemsLayout = new Wt::WHBoxLayout());
  m_typeItemsLayout->addWidget(&m_typeField);
  m_typeItemsLayout->addWidget(&m_typeExternalServiceNameField);

  m_typeField.addItem(NodeType::toString(NodeType::BusinessService).toStdString());
  m_typeField.addItem(NodeType::toString(NodeType::ITService).toStdString());
  m_typeField.addItem(NodeType::toString(NodeType::ExternalService).toStdString());

  m_typeField.changed().connect(this, &WebEditor::handleNodeTypeChanged);
  m_typeExternalServiceNameField.setHidden(true);

  // set icon type values
  for (const auto& icon: ngrt4n::nodeIcons().keys()) {
    int index = m_iconBox.count();
    m_iconBox.addItem(icon.toStdString());
    m_iconIndexMap[icon] =index;

    if (icon == ngrt4n::DEFAULT_ICON) {
      m_iconBox.setCurrentIndex(index);
    }
  }

  // propagation rule field
  m_fieldEditionPane.bindWidget("prop-rule-field", &m_propRuleBox);

  m_propRuleBox.addItem(PropRules(PropRules::Unchanged).toString().toStdString());
  m_propRuleBox.addItem(PropRules(PropRules::Decreased).toString().toStdString());
  m_propRuleBox.addItem(PropRules(PropRules::Increased).toString().toStdString());

  // calculation rules field
  m_fieldEditionPane.bindWidget("calc-rule-field", &m_calcRuleBox);
  m_calcRuleBox.addItem(CalcRules(CalcRules::Worst).toString().toStdString());
  m_calcRuleBox.addItem(CalcRules(CalcRules::Average).toString().toStdString());
  m_calcRuleBox.addItem(CalcRules(CalcRules::WeightedAverageWithThresholds).toString().toStdString());

  m_fieldEditionPane.bindWidget("icon-field", &m_iconBox);
  m_fieldEditionPane.bindWidget("description-field", &m_descField);

  // data point-related field and widgets
  m_fieldEditionPane.bindWidget("monitoring-item-field", &m_dataPointItemsContainer);

  m_dataPointItemsContainer.setLayout(m_dataPointItemsLayout = new Wt::WHBoxLayout());
  m_dataPointItemsLayout->addWidget(&m_dataPointSourceField, 1);
  m_dataPointItemsLayout->addWidget(&m_dataPointGroupField, 1);
  m_dataPointItemsLayout->addWidget(&m_dataPointField, 3);
  m_dataPointItemsContainer.setDisabled(true);

  m_dataPointField.blurred().connect(this, &WebEditor::handleDataPointChanged);
  m_dataPointSourceField.activated().connect(this, &WebEditor::handleDataPointSourceChanged);
  m_dataPointGroupField.activated().connect(this, &WebEditor::handleDataPointGroupChanged);

  // options auto completion
  Wt::WSuggestionPopup::Options dataPointSuggestionOptions;
  dataPointSuggestionOptions.highlightBeginTag = "<span class=\"highlight\">";
  dataPointSuggestionOptions.highlightEndTag = "</span>";
  dataPointSuggestionOptions.listSeparator = ',';
  dataPointSuggestionOptions.whitespace = " \\n";
  dataPointSuggestionOptions.wordSeparators = "-., \":\\n;";
  dataPointSuggestionOptions.appendReplacedText = "";

  m_dataPointListPopup.reset(new Wt::WSuggestionPopup(
                               Wt::WSuggestionPopup::generateMatcherJS(dataPointSuggestionOptions),
                               Wt::WSuggestionPopup::generateReplacerJS(dataPointSuggestionOptions)
                               ));
  m_dataPointListPopup->forEdit(&m_dataPointField);
  m_dataPointField.setPlaceholderText(Q_TR("Type a monitoring data point"));

  m_dataPointListModel.reset(new Wt::WStringListModel());
  m_dataPointListPopup.get()->setModel(static_cast<Wt::WStringListModel*>(m_dataPointListModel.get()));

  m_dataPointGroupField.addItem(Q_TR("Set a group for filtering"));
}



void WebEditor::handleNewViewButton(void)
{
  m_cdata.clear();

  NodeT node;

  node.id = ngrt4n::ROOT_ID;
  node.name = QObject::tr("New service view");
  node.type = NodeType::BusinessService;
  node.parent = "";
  node.sev_prule = PropRules::Unchanged;
  node.sev_crule = CalcRules::Worst;
  node.weight = ngrt4n::WEIGHT_UNIT;
  node.icon = ngrt4n::DEFAULT_ICON;

  m_cdata.bpnodes.insert(node.id, node);

  rebuiltTree();
}


void WebEditor::rebuiltTree(void)
{
  m_tree.build();

  auto rnode = m_cdata.bpnodes.find(ngrt4n::ROOT_ID);
  if (rnode != m_cdata.bpnodes.end()) {
    fillInEditorFromNodeInfo(*rnode);
    m_tree.expandNodeById(ngrt4n::ROOT_ID);
    m_tree.selectNodeById(ngrt4n::ROOT_ID);
  }
}



void WebEditor::handleTreeItemSelectionChanged(void)
{
  updateNodeDataFromEditor(m_formerSelectedNodeId);

  Wt::WModelIndexSet selectedTreeItems = m_tree.selectedIndexes();
  if (! selectedTreeItems.empty()) {
    m_selectedTreeItemIndex = *(selectedTreeItems.begin());
    fillInEditorFromCurrentSelection();
  } else {
    m_selectedTreeItemIndex = Wt::WModelIndex();
  }
}


void WebEditor::showTreeContextMenu(Wt::WModelIndex, Wt::WMouseEvent event) {
  if (m_selectedTreeItemIndex.isValid()) {
    m_editionContextMenu.popup(event);
  }
}


void WebEditor::handleTreeContextMenu(Wt::WMenuItem* menu)
{
  std::string triggeredMenu = menu->text().toUTF8();

  if (triggeredMenu == MENU_LABELS[ MENU_ADD_SUBSERVICE ]) {
    addSubServiceFromTreeNodeIndex(m_selectedTreeItemIndex);
  } else if (triggeredMenu == MENU_LABELS[ MENU_DELETE_SUBSERVICE ] ) {
    removeServiceByTreeNodeIndex(m_selectedTreeItemIndex);
  }
}


void WebEditor::handleKeyPressed(const Wt::WKeyEvent& event)
{
  if (event.modifiers() != Wt::ShiftModifier) {
    return;
  }

  switch (event.key()) {
    case Wt::Key_C:
      addSubServiceFromTreeNodeIndex(m_selectedTreeItemIndex);
      break;
    case Wt::Key_X:
      removeServiceByTreeNodeIndex(m_selectedTreeItemIndex);
      break;
    case Wt::Key_S:
      handleSaveViewButton();
      break;
    case Wt::Key_N:
      handleNewViewButton();
      break;
    case Wt::Key_O:
      handleOpenViewButton();
      break;
    case Wt::Key_I:
      handleImportMonitoringConfigButton();
      break;
    case Wt::Key_Z:
      handleImportZabbixItServiceButton();
      break;
    case Wt::Key_G:
      handleImportNagiosBpiButton();
      break;
    default:
      break;
  }
}


void WebEditor::addSubServiceFromTreeNodeIndex(const Wt::WModelIndex& index)
{
  static int newNodeIndex = 0;

  if (! index.isValid()) {
    return ;
  }

  m_tree.expand(index);
  QString nodeId = m_tree.findNodeIdFromTreeItem(index);

  NodeT childSrv;
  childSrv.id = ngrt4n::generateId();
  childSrv.name = QObject::tr("New service %1").arg(++newNodeIndex);
  childSrv.type = NodeType::BusinessService;
  childSrv.sev_prule = PropRules::Unchanged;
  childSrv.sev_crule = CalcRules::Worst;
  childSrv.weight = ngrt4n::WEIGHT_UNIT;
  childSrv.icon = ngrt4n::DEFAULT_ICON;
  childSrv.parent = nodeId;

  m_cdata.bpnodes.insert(childSrv.id, childSrv);

  auto selectedSrvIt = m_cdata.bpnodes.find(nodeId);
  if (selectedSrvIt == m_cdata.bpnodes.end()) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("No parent node found with id: %1").arg(nodeId).toStdString());
    return;
  }

  if (selectedSrvIt->type != NodeType::BusinessService) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Action not allowed on node type: %1").arg(NodeType::toString(selectedSrvIt->type)).toStdString());
    return;
  }

  bool bindToParent = true;
  bool selectItemAfterProcessing = true;
  m_tree.addTreeItem(childSrv, bindToParent, selectItemAfterProcessing);
}


void WebEditor::removeServiceByTreeNodeIndex(const Wt::WModelIndex& index)
{
  if (! index.isValid()) {
    return ;
  }

  QString nodeId = m_tree.findNodeIdFromTreeItem(index);

  NodeListT::iterator ninfoIt;

  if ( ! ngrt4n::findNode(&m_cdata, nodeId, ninfoIt) ) {
    return ;
  }

  int depth = findNodeDepth(*ninfoIt);
  auto parentId = ninfoIt->parent;

  m_formerSelectedNodeId.clear();

  // check it's the root node, and forbid the deletion
  if (depth == 1 || parentId.isEmpty()) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Operation not allowed on root node"));
    return ;
  }

  // find all the descendant nodes
  auto descendants = findDescendantNodes(nodeId);

  // find descendants from m_cdata
  for (const auto& node: descendants) {
    removeNodeFromCdata(node);
  }

  // remove the node itself from m_cdata
  removeNodeFromCdata(*ninfoIt);

  // reconstruct m_cdata.edges
  bindParentChildEdges();

  // rebuild the tree
  m_tree.build();
  m_tree.expandToDepth(depth > 1 ? depth -1 : 1);
  //m_tree.selectNodeById(parentId);

}


void WebEditor::removeNodeFromCdata(const NodeT& ninfo)
{
  if (ninfo.type == NodeType::ITService) {
    m_cdata.cnodes.remove(ninfo.id);
  } else {
    m_cdata.bpnodes.remove(ninfo.id);
  }
}


QList<NodeT> WebEditor::findDescendantNodes(const QString& nodeId) {

  QList<NodeT> descendants;

  for (const auto& node:  m_cdata.bpnodes) {
    if (node.parent == nodeId) {
      descendants.append(node);
    }
  }

  for (const auto& node:  m_cdata.cnodes) {
    if (node.parent == nodeId) {
      descendants.append(node);
    }
  }

  for (const auto& node: descendants) {
    descendants.append( findDescendantNodes(node.id) ) ;
  }

  return descendants;
}

int WebEditor::findNodeDepth(const NodeT& ninfo)
{
  if (ninfo.id == ngrt4n::ROOT_ID) {
    return 1;
  }

  NodeListT::const_iterator parentNodeIt;

  bool success = ngrt4n::findNode(m_cdata.bpnodes, m_cdata.cnodes, ninfo.parent, parentNodeIt);

  if (! success) {
    return -1;
  }

  return findNodeDepth(*parentNodeIt) + 1;
}


void WebEditor::bindParentChildEdges(void)
{
  m_cdata.edges.clear();
  for (const auto& node:  m_cdata.bpnodes) {
    m_cdata.edges.insertMulti(node.parent, node.id);
  }

  for (const auto& node:  m_cdata.cnodes) {
    m_cdata.edges.insertMulti(node.parent, node.id);
  }

}


void WebEditor::fillInEditorFromCurrentSelection(void)
{
  QString nodeId = m_tree.findNodeIdFromTreeItem(m_selectedTreeItemIndex);
  NodeListT::const_iterator node_cit;

  bool success = ngrt4n::findNode(m_cdata.bpnodes, m_cdata.cnodes, nodeId, node_cit);
  if (! success) {
    return;
  }

  fillInEditorFromNodeInfo(*node_cit);

  m_formerSelectedNodeId = nodeId;
}


void WebEditor::fillInEditorFromNodeInfo(const NodeT& ninfo)
{
  m_nameField.setText(ninfo.name.toStdString());
  m_descField.setText(ninfo.description.toStdString());
  m_typeField.setCurrentIndex(ninfo.type);
  m_iconBox.setCurrentIndex(m_iconIndexMap[ninfo.icon]);
  m_calcRuleBox.setCurrentIndex(ninfo.sev_crule);
  m_propRuleBox.setCurrentIndex(ninfo.sev_prule);

  if (ninfo.type ==  NodeType::ITService) {
    m_dataPointField.setText(ninfo.child_nodes.toStdString());
  } else if (NodeType::ExternalService) {
    m_typeExternalServiceNameField.setValueText(ninfo.child_nodes.toStdString());
  }

  m_typeExternalServiceNameField.setHidden(ninfo.type != NodeType::ExternalService);
  m_dataPointItemsContainer.setDisabled(ninfo.type != NodeType::ITService);
}


void WebEditor::updateNodeDataFromEditor(const QString& nodeId)
{
  NodeListT::iterator node_it;
  bool success = ngrt4n::findNode(m_cdata.bpnodes, m_cdata.cnodes, nodeId, node_it);
  if (! success) {
    return ;
  }

  node_it->name =  QString::fromStdString(m_nameField.text().toUTF8());
  node_it->description =  QString::fromStdString(m_descField.text().toUTF8());
  node_it->type =  m_typeField.currentIndex();
  node_it->icon = QString::fromStdString(m_iconBox.currentText().toUTF8());
  node_it->sev_crule  = m_calcRuleBox.currentIndex();
  node_it->sev_prule  = m_propRuleBox.currentIndex();

  if (node_it->type ==  NodeType::ITService) {
    node_it->child_nodes = QString::fromStdString( m_dataPointField.text().toUTF8() );
  } else if (NodeType::ExternalService && m_typeExternalServiceNameField.currentIndex() != 0) {
    node_it->child_nodes = QString::fromStdString( m_typeExternalServiceNameField.currentText().toUTF8() );
  }

  m_tree.updateItemLabel(nodeId, node_it->name.toStdString());
}

void WebEditor::refreshDynamicContents(void)
{
  m_dataPointSourceField.clear();

  m_dataPointSourceField.addItem(Q_TR("Set a source for autocompletion"));
  for (const auto& sinfo: WebBaseSettings().fetchSourceList(MonitorT::Auto)) {
    m_dataPointSourceField.addItem(sinfo.id.toStdString());
  }

  m_typeExternalServiceNameField.clear();

  m_typeExternalServiceNameField.addItem(Q_TR("Please select an external service"));
  for (const auto& v: m_dbSession->listViews()) {
    m_typeExternalServiceNameField.addItem(v.name);
  }
}


void WebEditor::handleNodeLabelChanged(void)
{
  m_tree.updateItemLabel(m_formerSelectedNodeId, m_nameField.text().toUTF8());
  updateNodeDataFromEditor(m_formerSelectedNodeId);
}

void WebEditor::handleDataPointChanged(void)
{
  if (m_dataPointField.isEnabled()) {
    auto dataPoint = m_dataPointField.text().toUTF8();
    size_t slashIndex = dataPoint.find("/");
    m_nameField.setText(dataPoint.substr(slashIndex + 1) + "("  + dataPoint.substr(0, slashIndex) + ")");
  }
}


void WebEditor::handleNodeTypeChanged(void)
{
  const int type = m_typeField.currentIndex();

  bool isItService = (type == NodeType::ITService);
  bool isExternalService = (type == NodeType::ExternalService);


  m_nameField.setDisabled(isItService);
  m_dataPointItemsContainer.setDisabled(! isItService);

  m_typeExternalServiceNameField.setHidden(! isExternalService);

  if (isItService) {
    m_nameField.setText("");
    m_nameField.setPlaceholderText(Q_TR("Value automatically computed from data point field"));
  }

  QString nodeId = m_tree.findNodeIdFromTreeItem(m_selectedTreeItemIndex);
  NodeListT::ConstIterator ninfoIt;
  if ( ! ngrt4n::findNode(m_cdata.bpnodes, m_cdata.cnodes, nodeId, ninfoIt) ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("No service selected"));
    return ;
  }

  if (type == NodeType::ITService || type == NodeType::ExternalService) {
    if (! findDescendantNodes(nodeId).empty()) {
      m_typeField.setCurrentIndex(ninfoIt->type);
      m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Type not allowed for item with descendants"));
      return ;
    }
  }

  if (type == ninfoIt->type) {
    return ;
  }

  // backup node info before processing that implies to remove it and thus invalidate its iterator
  NodeT ninfo = *ninfoIt;
  ninfo.child_nodes.clear();

  switch (type) {
    case NodeType::ITService:
      if (m_cdata.bpnodes.remove(ninfo.id) > 0) {
        m_cdata.cnodes.insert(ninfo.id, ninfo);
      } else {
        m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Node not found in parent list"));
      }
      break;

    case NodeType::ExternalService:
    case NodeType::BusinessService:
    default:
      if (ninfo.type == NodeType::ExternalService || ninfo.type == NodeType::BusinessService) {
        m_cdata.bpnodes[nodeId].child_nodes.clear(); // typically for external service child_nodes is not empty
        break;
      }

      if (m_cdata.cnodes.remove(ninfo.id) > 0) {
        m_cdata.bpnodes.insert(ninfo.id, ninfo);
      } else {
        m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Node not found in child list"));
      }
      break;
  }

  updateNodeDataFromEditor(ninfoIt->id);
}


void WebEditor::handleSaveViewButton(void)
{
  std::string destPath = "";
  if (m_currentFilePath.empty()) {
    destPath = QString("%1/%2.ms.ngrt4n.xml").arg(m_configDir, ngrt4n::generateId()).toStdString();
  } else {
    destPath = m_currentFilePath;
  }

  updateNodeDataFromEditor(m_formerSelectedNodeId);

  ngrt4n::fixParentChildrenDependencies(m_cdata);

  auto saveStatus = saveContentToFile(m_cdata, destPath.c_str());

  if (saveStatus.first != 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, saveStatus.second.toStdString());
  } else {
    m_operationCompleted.emit(ngrt4n::OperationSucceeded, Q_TR("Saved"));
    m_currentFilePath = destPath;
  }

}


std::pair<int, QString> WebEditor::saveContentToFile(const CoreDataT& cdata, const QString& destPath)
{
  auto rootService = cdata.bpnodes.constFind(ngrt4n::ROOT_ID);

  if (rootService == cdata.bpnodes.cend()) {
    return std::make_pair(-1, QObject::tr("Invalid or incompleted view"));
  }

  std::pair<int, QString> saveResult = ngrt4n::saveDataAsDescriptionFile(destPath, cdata);

  if (saveResult.first != 0) {
    return std::make_pair(-1, saveResult.second);
  }

  DboView vinfo;
  vinfo.name = rootService->name.toStdString();
  vinfo.service_count = cdata.bpnodes.size() + cdata.cnodes.size();
  vinfo.path = destPath.toStdString();

  // save view in database if it's the 1st time
  if (m_currentFilePath.empty()) {
    int rc = m_dbSession->addView(vinfo);
    if (rc != 0) {
      CORE_LOG("error", m_dbSession->lastError());
      return std::make_pair(rc,  m_dbSession->lastError().c_str());
    }

    return std::make_pair(0, "");
  }

  int rc = m_dbSession->updateViewWithPath(vinfo, destPath.toStdString());
  if (rc != 0) {
    CORE_LOG("error", m_dbSession->lastError());
    return std::make_pair(rc, m_dbSession->lastError().c_str());
  }

  return std::make_pair(0, "");
}



void WebEditor::handleImportMonitoringConfigButton(void)
{
  WebBaseSettings settings;
  auto sources = settings.fetchSourceList(MonitorT::Auto);
  m_importMonitoringConfigDialog.updateContentWithSourceList(sources.keys(), InputSelector::SourceWithTextFilter);
  m_importMonitoringConfigDialog.show();
}


void WebEditor::handleImportZabbixItServiceButton(void)
{
  WebBaseSettings settings;
  auto sources = settings.fetchSourceList(MonitorT::Zabbix);
  m_importZabbixItServicesDialog.updateContentWithSourceList(sources.keys(), InputSelector::SourceOnly);
  m_importZabbixItServicesDialog.show();
}

void WebEditor::handleImportNagiosBpiButton(void)
{
  WebBaseSettings settings;
  auto sources = settings.fetchSourceList(MonitorT::Nagios);
  m_importNagiosBpiDialog.updateContentWithSourceList(sources.keys(), InputSelector::SourceWithFileFilter);
  m_importNagiosBpiDialog.show();
}




void WebEditor::importMonitoringConfig(const std::string& srcId, const std::string& groupFilter)
{
  WebBaseSettings settings;

  if (srcId.empty()) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("No source selected"));
    return ;
  }

  auto q_srcId = QString::fromStdString(srcId);
  auto sources = settings.fetchSourceList(MonitorT::Auto);
  auto src = sources.constFind(q_srcId);
  if (src == sources.cend()) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("No source with id: %1").arg(q_srcId).toStdString());
    return ;
  }

  CoreDataT cdata;
  auto importResult = ngrt4n::importHostGroupAsBusinessView(*src, groupFilter.c_str(), cdata);
  if (importResult.first != 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, importResult.second.toStdString());
    return  ;
  }

  m_currentFilePath.clear();

  auto destPath = QString("%1/%2_autoimport.ms.ngrt4n.xml").arg(m_configDir, ngrt4n::generateId());
  auto saveStatus = saveContentToFile(cdata, destPath);
  if (saveStatus.first != 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, saveStatus.second.toStdString());
    return ;
  }

  handleOpenFile(destPath.toStdString(), "");
}

void WebEditor::handleDataPointSourceChanged(int index)
{
  m_operationCompleted.emit(ngrt4n::OperationInProgress, Q_TR("Import of monitoring data in progress..."));

  auto srcId = QString::fromStdString(m_dataPointSourceField.itemText(index).toUTF8());

  SourceT srcInfo;
  WebBaseSettings settings;

  if (! settings.loadSource(srcId, srcInfo)) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Cannot load source info"));
    return ;
  }

  if (! m_dataPointsListBySource[srcId].empty()) {
    m_operationCompleted.emit(ngrt4n::OperationFinished, Q_TR("Nothing to do"));
    return ;
  }

  ChecksT checks;
  auto importResult = ngrt4n::importMonitorItemAsDataPoints(srcInfo, "", checks);
  if (importResult.first != 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, importResult.second.toStdString());
    return;
  }

  const std::string ALL_GROUPS = Q_TR("All groups");
  const std::string NO_GROUP = Q_TR("Unclassified");

  m_dataPointListModel->removeRows(0, m_dataPointListModel->rowCount());
  m_dataPointGroupField.clear();

  m_dataPointGroupField.addItem(ALL_GROUPS);
  for (const auto& check: checks) {
    std::string dataPoint = QString("%1:%2").arg(srcId, check.id.c_str()).toStdString();
    QStringList groups = QString::fromStdString(check.host_groups).split(ngrt4n::CHILD_SEP.c_str());

    if (groups.isEmpty()) {
      m_dataPointsListByGroup[NO_GROUP.c_str()].push_back(dataPoint);
      m_dataPointGroupField.addItem(NO_GROUP);
    } else {
      for (const auto& group: groups) {
        auto gpKey = group.toStdString();
        m_dataPointsListByGroup[gpKey].push_back(dataPoint);
        if (m_dataPointGroupField.findText(gpKey) < 0) {
          m_dataPointGroupField.addItem(gpKey);
        }
      }
      m_dataPointsListBySource[srcId].push_back(dataPoint);
    }

  }

  m_dataPointListModel->setStringList(m_dataPointsListBySource[srcId]);

  m_dataPointField.setPlaceholderText(QObject::tr("Autocompletion enabled for %1").arg(srcId).toStdString());

  m_operationCompleted.emit(ngrt4n::OperationSucceeded, Q_TR("Import completed"));
}


void WebEditor::handleDataPointGroupChanged(int)
{
  std::string group = m_dataPointGroupField.currentText().toUTF8();
  m_dataPointListModel->setStringList(m_dataPointsListByGroup[group]);

  m_dataPointField.setPlaceholderText(QObject::tr("Autocompletion enabled for '%1' group").arg(group.c_str()).toStdString());
}


void WebEditor::importNagiosBpi(const std::string& srcId, const std::string& bpiConfigPath)
{
  QString sourceId(srcId.c_str());
  QString bpiConfigFile(bpiConfigPath.c_str());

  if (sourceId.isEmpty()) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("No source selected"));
    return;
  }

  if (bpiConfigFile.isEmpty()) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("No file selected"));
    return;
  }

  QFile file(bpiConfigFile);
  if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Cannot open file: %1").arg(bpiConfigFile).toStdString());
    return;
  }

  bool parsingFailed = false;
  std::string parsingErrorMsg = "";
  QString line;
  int lineIndex = 0;
  QTextStream streamReader(&file);
  NodeT rootSrv;
  rootSrv.id = ngrt4n::ROOT_ID;
  rootSrv.name = QObject::tr("Nagios BPI Services"); //FIXME added a prefix ?
  rootSrv.type = NodeType::BusinessService;

  CoreDataT cdata;
  while (static_cast<void>(line = streamReader.readLine()), ! line.isNull()) {

    ++lineIndex;

    line = line.trimmed();
    if ( line.startsWith("#")
         || line.isEmpty()
         || ! line.startsWith("define")) continue;

    if (! line.endsWith("{")) {
      parsingFailed = true;
      parsingErrorMsg = QObject::tr("Group definition must end with '{' at line %1").arg(lineIndex).toStdString();
      break;
    }

    QStringList fields = line.mid(0, line.size() - 1).trimmed().split(" ");
    if (fields.size() != 2) {
      parsingFailed = true;
      parsingErrorMsg = QObject::tr("Bad group definition at line %1").arg(lineIndex).toStdString();
      break;
    }

    QString groudId = fields[1];

    NodeListT::Iterator currentGroupSrv = cdata.bpnodes.find(groudId);
    if (currentGroupSrv == cdata.bpnodes.end()) {
      NodeT groupSrv;
      groupSrv.id = groudId;
      groupSrv.name = groudId;
      groupSrv.type = NodeType::BusinessService;
      groupSrv.parent = "";
      groupSrv.sev_prule = PropRules::Unchanged;
      groupSrv.sev_crule = CalcRules::Worst;
      groupSrv.weight = ngrt4n::WEIGHT_UNIT;
      groupSrv.icon = ngrt4n::DEFAULT_ICON;
      currentGroupSrv = cdata.bpnodes.insert(groudId, groupSrv);
    }

    currentGroupSrv->sev_crule = CalcRules::Worst;

    // now parse group config
    std::pair<int, std::string> groupMembersExtractResult;
    double warningThreshold  = 0;
    double criticalThreshold = 0;
    while (static_cast<void>(line = streamReader.readLine()), ! line.isNull()) {

      ++lineIndex;

      line = line.trimmed();

      if (line.isEmpty()) continue;
      if (line == "}") break;

      fields = line.split("=");
      if (fields.size() != 2) {
        parsingFailed = true;
        parsingErrorMsg = QObject::tr("Bad group attribute definition at line %1").arg(lineIndex).toStdString();
        break;
      }

      if (fields[0] == "title") {
        currentGroupSrv->name = fields[1];
      } else if (fields[0] == "desc") {
        currentGroupSrv->description = fields[1];
      } else if (fields[0] == "members") {
        bool hasCluster = false;
        QString members = fields[1];
        groupMembersExtractResult = extractNagiosBpiGroupMembers(currentGroupSrv->id,
                                                                 sourceId,
                                                                 members,
                                                                 cdata.bpnodes,
                                                                 cdata.cnodes,
                                                                 currentGroupSrv->child_nodes,
                                                                 hasCluster);
        if (groupMembersExtractResult.first < 0) {
          parsingFailed = true;
          parsingErrorMsg = QObject::tr("Cannot find group members: %1 (%2)").arg(members, groupMembersExtractResult.second.c_str()).toStdString();
          break;
        } else {
          currentGroupSrv->sev_crule = hasCluster ? CalcRules::Average : CalcRules::Worst;
        }

      } else if (fields[0] == "warning_threshold") {
        warningThreshold = fields[1].toDouble();
      } else if (fields[0] == "critical_threshold") {
        criticalThreshold = fields[1].toDouble();
      } else if (fields[0] == "priority") {
        /// not applicable
      }
    }


    if (parsingFailed) {
      break;
    } else {
      if (groupMembersExtractResult.first > 0) {
        if (warningThreshold > 0 || criticalThreshold > 0) {
          currentGroupSrv->sev_crule = CalcRules::WeightedAverageWithThresholds;
        }
        ThresholdT threshold;
        if (warningThreshold > 0.0) {
          threshold.weight = warningThreshold / groupMembersExtractResult.first;
          threshold.sev_in = ngrt4n::Major;
          threshold.sev_out = ngrt4n::Major;
          currentGroupSrv->thresholdLimits.push_back(threshold);
        }
        if (warningThreshold > 0.0 ) {
          threshold.weight = criticalThreshold / groupMembersExtractResult.first;
          threshold.sev_in = ngrt4n::Critical;
          threshold.sev_out = ngrt4n::Critical;
          currentGroupSrv->thresholdLimits.push_back(threshold);
        }
      }
    }
  }

  file.close();

  if (parsingFailed) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, parsingErrorMsg);
    return ;
  }

  attachOrphanedNodesToRoot(cdata.bpnodes, rootSrv);
  attachOrphanedNodesToRoot(cdata.cnodes, rootSrv);
  cdata.bpnodes.insert(rootSrv.id, rootSrv);

  m_currentFilePath.clear();

  auto destPath = QString("%1/%2_autoimport.ms.ngrt4n.xml").arg(m_configDir, ngrt4n::generateId());

  auto saveStatus = saveContentToFile(cdata, destPath);
  if (saveStatus.first != 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, saveStatus.second.toStdString());
    return ;
  }

  handleOpenFile(destPath.toStdString(), "");

  m_operationCompleted.emit(ngrt4n::OperationSucceeded, "Completed");
}



void WebEditor::attachOrphanedNodesToRoot(NodeListT& nodes, NodeT& root)
{
  for (auto& node: nodes) {
    if (! node.parent.isEmpty()) {
      continue;
    }

    node.parent = root.id;
    if (! root.child_nodes.isEmpty()) {
      root.child_nodes.append(ngrt4n::CHILD_Q_SEP);
    }

    root.child_nodes.append(node.id);
  }
}


std::pair<int, std::string> WebEditor::extractNagiosBpiGroupMembers(const QString& parentServiceId,
                                                                    const QString& sourceId,
                                                                    const QString& groupMembers,
                                                                    NodeListT& bpnodes,
                                                                    NodeListT& cnodes,
                                                                    QString& childrenIds,
                                                                    bool& hasCluster)
{

  std::pair<int, std::string> processOut = std::make_pair(0, "");
  hasCluster = false;
  QStringList members = groupMembers.split(",");
  if (members.isEmpty()) {
    processOut.first = 0;
    return processOut;
  }

  for (const auto& member: members) {
    if (member.isEmpty()) {
      continue;
    }
    bool isClusterMember = member.endsWith(";&");
    bool isEssentialMember = member.endsWith(";|");
    bool isGroupMember = member.startsWith("$");
    int start = isGroupMember ? 1 : 0;
    int count = (isClusterMember || isEssentialMember) ? member.size() - (start + 2) : member.size() - start;
    if (isClusterMember) hasCluster = true;

    QString memberLabel = member.mid(start, count);
    QString memberId = memberLabel;
    QString currentChildId = "";

    if (isGroupMember) {
      NodeListT::Iterator currentMemberSrvIt = bpnodes.find(memberId);

      if (currentMemberSrvIt != bpnodes.end()) {
        currentMemberSrvIt->parent = parentServiceId;
      } else {
        NodeT memberSrv;
        memberSrv.id = memberId;
        memberSrv.name = memberLabel;
        memberSrv.type = NodeType::BusinessService;
        memberSrv.parent = parentServiceId;
        memberSrv.sev_prule = PropRules::Unchanged;
        memberSrv.sev_crule = CalcRules::Worst;
        memberSrv.weight = ngrt4n::WEIGHT_UNIT;
        memberSrv.icon = ngrt4n::DEFAULT_ICON;
        currentMemberSrvIt = bpnodes.insert(memberId, memberSrv);
      }

      currentMemberSrvIt->weight = isEssentialMember ? ngrt4n::WEIGHT_MAX: ngrt4n::WEIGHT_UNIT;
      currentChildId = memberId.trimmed();

    } else {
      QStringList fields = memberId.split(";");

      if (fields.size() == 2) {
        currentChildId = ngrt4n::generateId();
        NodeT cnode;
        cnode.id = currentChildId;
        cnode.name = QString("%1 on %2").arg(fields[1], fields[0]);
        cnode.type = NodeType::ITService;
        cnode.parent = parentServiceId;
        cnode.sev_prule = PropRules::Unchanged;
        cnode.sev_crule = CalcRules::Worst;
        cnode.weight = isEssentialMember ? ngrt4n::WEIGHT_MAX: ngrt4n::WEIGHT_UNIT;
        cnode.child_nodes = QString("%1:%2/%3").arg(sourceId, fields[0].trimmed(), fields[1].trimmed());
        cnode.icon = ngrt4n::DEFAULT_ICON;
        cnodes.insert(cnode.id, cnode);
      } else {
        processOut.first = -1;
        processOut.second = QObject::tr("Bad service entry %1").arg(memberId).toStdString();
        break;
      }
    }

    if (childrenIds.isEmpty()) {
      childrenIds = currentChildId;
    } else {
      childrenIds += QString::fromStdString(ngrt4n::CHILD_SEP) + currentChildId;
    }
    ++processOut.first;
  }

  return processOut;
}


void WebEditor::importZabbixITServices(const std::string& srcId)
{
  if (srcId.empty()) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("No source selected"));
    return;
  }

  CoreDataT cdata;

  auto allZbxSources = WebBaseSettings().fetchSourceList(MonitorT::Zabbix);
  auto importStatus = ZbxHelper().loadITServices(allZbxSources[srcId.c_str()], cdata);
  if (importStatus.first != 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Importation failed: %1").arg(importStatus.second).toStdString());
    return ;
  }

  m_currentFilePath.clear();

  auto destPath = QString("%1/%2_autoimport.ms.ngrt4n.xml").arg(m_configDir, ngrt4n::generateId());
  auto saveStatus = saveContentToFile(cdata, destPath);
  if (saveStatus.first != 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, saveStatus.second.toStdString());
    return ;
  }

  handleOpenFile(destPath.toStdString(), "");

  m_operationCompleted.emit(ngrt4n::OperationSucceeded, "Completed");
}
