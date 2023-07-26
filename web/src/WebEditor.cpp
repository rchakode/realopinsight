/*
 * WebEditor.cpp
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

#include "WebUtils.hpp"
#include "WebEditor.hpp"
#include "Base.hpp"
#include "utilsCore.hpp"
#include "Parser.hpp"
#include "WebBaseSettings.hpp"
#include "WebInputField.hpp"
#include "ZbxHelper.hpp"
#include "K8sHelper.hpp"
#include <fstream>
#include <regex>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <Wt/WApplication.h>
#include <Wt/WPanel.h>
#include <Wt/WPointF.h>
#include <Wt/WText.h>
#include <Wt/WLink.h>
#include <Wt/WImage.h>
#include <Wt/WTemplate.h>
#include <Wt/WPoint.h>


const QMap<int, std::string> WebEditor::MENU_LABELS = {
  {WebEditor::MENU_ADD_SUBSERVICE, Q_TR("Add child (Shift+C)")},
  {WebEditor::MENU_DELETE_SUBSERVICE, Q_TR("Delete node and descendants (Shift+X)")}
};

WebEditor::WebEditor(void)
{
  auto mainLayout = std::make_unique<Wt::WHBoxLayout>();

  m_viewSelector.dataTriggered().connect(this, &WebEditor::handleOpenFile);
  m_nagiosBPISelector.fileUploaded().connect(this, &WebEditor::importNagiosBpi);
  m_zabbixITViewSelector.itemTriggered().connect(this, &WebEditor::importZabbixITServices);
  m_configImporter.dataTriggered().connect(this, &WebEditor::importMonitoringConfig);

  auto tree = std::make_unique<WebTree>(&m_cdata);
  m_treeRef = tree.get();
  m_treeRef->activateEditionFeatures();
  m_treeRef->selectionChanged().connect(this, &WebEditor::handleTreeItemSelectionChanged);
  m_treeRef->doubleClicked().connect(this, &WebEditor::showTreeContextMenu);
  m_treeRef->keyPressed().connect(this, &WebEditor::handleKeyPressed);
  mainLayout->addWidget(std::move(tree));

  m_contextMenus.addItem("images/plus.png", MENU_LABELS[MENU_ADD_SUBSERVICE])->triggered().connect(this, &WebEditor::handleTreeContextMenu);
  m_contextMenus.addItem("images/minus.png", MENU_LABELS[MENU_DELETE_SUBSERVICE])->triggered().connect(this, &WebEditor::handleTreeContextMenu);

  auto editionPane = std::make_unique<Wt::WTemplate>(Wt::WString::tr("editor-fields-form.tpl"));
  m_editionPaneRef = editionPane.get();
  mainLayout->addWidget(std::move(editionPane));

  auto serviceBtn = std::make_unique<Wt::WImage>(Wt::WLink("images/built-in/new.png"));
  serviceBtn->setToolTip(Q_TR("Start a new edition"));
  serviceBtn->clicked().connect(this, &WebEditor::handleNewViewButton);
  m_editionPaneRef->bindWidget("new-service-view", std::move(serviceBtn));

  auto openServiceBtn = std::make_unique<Wt::WImage>(Wt::WLink("images/built-in/open.png"));
  openServiceBtn->setToolTip(Q_TR("Open an existing service view for edition"));
  openServiceBtn->clicked().connect(this, &WebEditor::handleOpenViewButton);
  m_editionPaneRef->bindWidget("open-service-view", std::move(openServiceBtn));

  auto saveCurrentServiceBtn = std::make_unique<Wt::WImage>(Wt::WLink("images/built-in/save.png"));
  saveCurrentServiceBtn->setToolTip(Q_TR("Save changes"));
  saveCurrentServiceBtn->clicked().connect(this, &WebEditor::handleSaveViewButton);
  m_editionPaneRef->bindWidget("save-current-view", std::move(saveCurrentServiceBtn));

  auto importConfigBtn = std::make_unique<Wt::WImage>(Wt::WLink("images/built-in/import-monitoring-data.png"));
  importConfigBtn->setToolTip(Q_TR("Import monitoring data as group-based tree"));
  importConfigBtn->clicked().connect(this, &WebEditor::handleImportMonitoringConfigButton);
  m_editionPaneRef->bindWidget("import-native-config", std::move(importConfigBtn));

  auto importZabbixItServices = std::make_unique<Wt::WImage>(Wt::WLink("images/built-in/import-zabbix_32x32.png"));
  importZabbixItServices->setToolTip(Q_TR("Import Zabbix IT services"));
  importZabbixItServices->clicked().connect(this, &WebEditor::handleImportZabbixItServiceButton);
  m_editionPaneRef->bindWidget("import-zabbix-it-service", std::move(importZabbixItServices));

  auto importNagiosBpiBtn = std::make_unique<Wt::WImage>(Wt::WLink("images/built-in/import-nagios_32x32.png"));
  importNagiosBpiBtn->setToolTip(Q_TR("Import Nagios BPI"));
  importNagiosBpiBtn->clicked().connect(this, &WebEditor::handleImportNagiosBpiButton);
  m_editionPaneRef->bindWidget("import-nabios-bpi", std::move(importNagiosBpiBtn));

  // name field
  m_nameFieldRef = m_editionPaneRef->bindNew<Wt::WLineEdit>("name-field");
  m_nameFieldRef->setPlaceholderText(Q_TR("Set service name"));
  m_nameFieldRef->blurred().connect(this, &WebEditor::handleNodeLabelChanged);

  auto typeItemsLayout = std::make_unique<Wt::WHBoxLayout>();

  auto externalTypeSelectorField = std::make_unique<Wt::WComboBox>();
  m_externalTypeSelectorFieldRef = externalTypeSelectorField.get();
  externalTypeSelectorField->setHidden(true);
  typeItemsLayout->addWidget(std::move(externalTypeSelectorField));

  auto typeField = std::make_unique<Wt::WComboBox>();
  m_typeFieldRef = typeField.get();
  typeField->addItem(NodeType::toString(NodeType::BusinessService).toStdString());
  typeField->addItem(NodeType::toString(NodeType::ITService).toStdString());
  typeField->addItem(NodeType::toString(NodeType::ExternalService).toStdString());
  typeField->activated().connect(this, &WebEditor::handleNodeTypeChanged);
  typeItemsLayout->addWidget(std::move(typeField));

  auto typeItemsContainer = std::make_unique<Wt::WContainerWidget>();
  typeItemsContainer->setLayout(std::move(typeItemsLayout));
  m_editionPaneRef->bindWidget("type-field", std::move(typeItemsContainer));

  auto propRuleField = std::make_unique<Wt::WComboBox>();
  m_propRuleFieldRef = propRuleField.get();
  propRuleField->addItem(PropRules(PropRules::Unchanged).toString().toStdString());
  propRuleField->addItem(PropRules(PropRules::Decreased).toString().toStdString());
  propRuleField->addItem(PropRules(PropRules::Increased).toString().toStdString());
  m_editionPaneRef->bindWidget("prop-rule-field", std::move(propRuleField));

  // calculation rules field
  auto calcRuleField = std::make_unique<Wt::WComboBox>();
  m_calcRuleFieldRef = calcRuleField.get();
  calcRuleField->addItem(CalcRules(CalcRules::Worst).toString().toStdString());
  calcRuleField->addItem(CalcRules(CalcRules::Average).toString().toStdString());
  calcRuleField->addItem(CalcRules(CalcRules::WeightedAverageWithThresholds).toString().toStdString());
  m_editionPaneRef->bindWidget("calc-rule-field", std::move(calcRuleField));

  // set icon type values
  auto iconField = std::make_unique<Wt::WComboBox>();
  m_iconFieldRef = iconField.get();
  for (const auto& icon: ngrt4n::NodeIcons.keys()) {
    int index = m_iconFieldRef->count();
    m_iconFieldRef->addItem(icon.toStdString());
    m_iconIndexMap[icon] =index;
    if (icon == ngrt4n::DEFAULT_ICON) {
      m_iconFieldRef->setCurrentIndex(index);
    }
  }
  m_editionPaneRef->bindWidget("icon-field", std::move(iconField));

  auto descField = std::make_unique<Wt::WLineEdit>();
  m_descFieldRef = descField.get();
  m_editionPaneRef->bindWidget("description-field", std::move(descField));

  auto checkSelectorPanel = std::make_unique<Wt::WContainerWidget>();
  auto checkSelectorLayout = std::make_unique<Wt::WHBoxLayout>();

  m_checkSelectorPanelRef = checkSelectorPanel.get();
  checkSelectorPanel->setDisabled(true);

  auto checkSourceField = std::make_unique<Wt::WComboBox>();
  m_checkSourceFieldRef = checkSourceField.get();
  checkSourceField->activated().connect(this, &WebEditor::handleDataPointSourceChanged);
  checkSelectorLayout->addWidget(std::move(checkSourceField), 1);

  auto checkGroupField = std::make_unique<Wt::WComboBox>();
  m_checkGroupFieldRef = checkGroupField.get();
  checkGroupField->addItem(Q_TR("Set a group for filtering"));
  checkGroupField->activated().connect(this, &WebEditor::handleDataPointGroupChanged);
  checkSelectorLayout->addWidget(std::move(checkGroupField), 1);

  auto checkField = std::make_unique<Wt::WLineEdit>();
  m_checkFieldRef = checkField.get();
  checkField->setPlaceholderText(Q_TR("Type a monitoring data point"));
  checkField->blurred().connect(this, &WebEditor::handleDataPointChanged);
  checkSelectorLayout->addWidget(std::move(checkField), 3);

  Wt::WSuggestionPopup::Options checkSuggestionConfig;
  checkSuggestionConfig.highlightBeginTag = "<span class=\"highlight\">";
  checkSuggestionConfig.highlightEndTag = "</span>";
  checkSuggestionConfig.listSeparator = ',';
  checkSuggestionConfig.whitespace = " \\n";
  checkSuggestionConfig.wordSeparators = "-., \":\\n;";
  checkSuggestionConfig.appendReplacedText = "";
  auto checkListModel = std::make_unique<Wt::WStringListModel>();
  m_checkSuggestionModelRef = checkListModel.get();
  m_checkSuggestionPanel = std::make_unique<Wt::WSuggestionPopup>(Wt::WSuggestionPopup::generateMatcherJS(checkSuggestionConfig), Wt::WSuggestionPopup::generateReplacerJS(checkSuggestionConfig));
  m_checkSuggestionPanel->forEdit(m_checkFieldRef);
  m_checkSuggestionPanel.get()->setModel(std::move(checkListModel));

  checkSelectorPanel->setLayout(std::move(checkSelectorLayout));
  m_editionPaneRef->bindWidget("monitoring-item-field", std::move(checkSelectorPanel));

  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(2);
  mainLayout->setResizable(0);
  mainLayout->setResizable(1);
  setLayout(std::move(mainLayout));
}


WebEditor::~WebEditor(){}


void  WebEditor::handleOpenViewButton(void)
{
  // dynamic view starting with "Source?:" must not be edited. E.g. Kubernetes view
  DbViewsT viewList;
  for (auto && view: m_dbSession->listViews()) {
    if (! std::regex_match(view.name, std::regex("Source[0-9]:.+"))) {
      viewList.push_back(view);
    }
  }
  m_viewSelector.updateContentWithViewList(viewList);
  m_viewSelector.show();
}


void  WebEditor::handleOpenFile(const std::string& path, const std::string&)
{
  Parser parser{&m_cdata, Parser::ParsingModeEditor, m_dbSession};
  auto outParser = parser.parse(path.c_str());
  if (outParser.first != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, outParser.second.toStdString());
    return ;
  }
  int rc = parser.processRenderingData();
  if (rc != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, parser.lastErrorMsg().toStdString());
    return ;
  }
  rebuildTree();
  m_currentFilePath = path;
}


void WebEditor::handleNewViewButton(void)
{
  static int unnamedViewIndex = 0;
  NodeT node;
  node.id = ngrt4n::ROOT_ID;
  node.name = QObject::tr("Unnamed service view - %1").arg(QString::number(unnamedViewIndex++));
  node.type = NodeType::BusinessService;
  node.parents.clear();
  node.sev = ngrt4n::Unknown;
  node.sev_prule = PropRules::Unchanged;
  node.sev_crule = CalcRules::Worst;
  node.weight = ngrt4n::WEIGHT_UNIT;
  node.icon = ngrt4n::DEFAULT_ICON;

  m_cdata.clear();
  m_cdata.monitor = MonitorT::Any;
  m_cdata.bpnodes.insert(node.id, node);

  rebuildTree();
}


void WebEditor::rebuildTree(void)
{
  m_treeRef->build();
  auto rnode = m_cdata.bpnodes.find(ngrt4n::ROOT_ID);
  if (rnode != m_cdata.bpnodes.end()) {
    fillInEditorFromNodeInfo(*rnode);
    m_treeRef->expandNodeById(ngrt4n::ROOT_ID);
    m_treeRef->selectNodeById(ngrt4n::ROOT_ID);
  }
}



void WebEditor::handleTreeItemSelectionChanged(void)
{
  updateNodeDataFromEditor(m_formerSelectedNodeId);
  Wt::WModelIndexSet selectedTreeItems = m_treeRef->selectedIndexes();
  if (! selectedTreeItems.empty()) {
    m_selectedTreeIndex = *(selectedTreeItems.begin());
    fillInEditorFromCurrentSelection();
  } else {
    m_selectedTreeIndex = Wt::WModelIndex();
  }
}


void WebEditor::showTreeContextMenu(Wt::WModelIndex, Wt::WMouseEvent event) {
  if (m_selectedTreeIndex.isValid()) {
    m_contextMenus.popup(event);
  }
}


void WebEditor::handleTreeContextMenu(Wt::WMenuItem* menu)
{
  std::string triggeredMenu = menu->text().toUTF8();
  if (triggeredMenu == MENU_LABELS[ MENU_ADD_SUBSERVICE ]) {
    addChildUnderTreeIndex(m_selectedTreeIndex);
  } else if (triggeredMenu == MENU_LABELS[ MENU_DELETE_SUBSERVICE ] ) {
    removeNodeByTreeIndex(m_selectedTreeIndex);
  }
}


void WebEditor::handleKeyPressed(const Wt::WKeyEvent& event)
{
  if (event.modifiers() != Wt::KeyboardModifier::Shift) {
    return;
  }

  switch (event.key()) {
  case Wt::Key::C:
    addChildUnderTreeIndex(m_selectedTreeIndex);
    break;
  case Wt::Key::X:
    removeNodeByTreeIndex(m_selectedTreeIndex);
    break;
  case Wt::Key::S:
    handleSaveViewButton();
    break;
  case Wt::Key::N:
    handleNewViewButton();
    break;
  case Wt::Key::O:
    handleOpenViewButton();
    break;
  case Wt::Key::I:
    handleImportMonitoringConfigButton();
    break;
  case Wt::Key::Z:
    handleImportZabbixItServiceButton();
    break;
  case Wt::Key::G:
    handleImportNagiosBpiButton();
    break;
  default:
    break;
  }
}


void WebEditor::addChildUnderTreeIndex(const Wt::WModelIndex& index)
{
  if (! index.isValid()) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Unsupported action (invalid selection)"));
    return ;
  }
  m_treeRef->expand(index);
  QString nodeId = m_treeRef->findNodeIdFromTreeItem(index);

  static int newNodeIndex = 0;
  NodeT child;
  child.name = QObject::tr("Unnamed service - %1").arg(++newNodeIndex);
  child.id = ngrt4n::generateId();
  child.type = NodeType::BusinessService;
  child.sev_prule = PropRules::Unchanged;
  child.sev_crule = CalcRules::Worst;
  child.weight = ngrt4n::WEIGHT_UNIT;
  child.icon = ngrt4n::DEFAULT_ICON;
  child.parents = QSet<QString>{ nodeId };

  m_cdata.bpnodes.insert(child.id, child);

  auto selectedNodeIt = m_cdata.bpnodes.find(nodeId);
  if (selectedNodeIt == m_cdata.bpnodes.end()) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("No parent node found with id: %1").arg(nodeId).toStdString());
    return;
  }

  if (selectedNodeIt->type != NodeType::BusinessService) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Action not allowed on node type: %1").arg(NodeType::toString(selectedNodeIt->type)).toStdString());
    return;
  }

  m_treeRef->registerNodeItem(child, nodeId, true);
  // FIXME: check the construction of parent->child dependency
}


void WebEditor::removeNodeByTreeIndex(const Wt::WModelIndex& index)
{
  if (! index.isValid()) {
    return ;
  }

  QString nodeId = m_treeRef->findNodeIdFromTreeItem(index);
  NodeListT::iterator ninfoIt;
  if ( ! ngrt4n::findNode(&m_cdata, nodeId, ninfoIt) ) {
    return ;
  }

  if (ninfoIt->id == ngrt4n::ROOT_ID) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Operation not allowed on root item"));
    return ;
  }

  auto descendants = findDescendantNodes(nodeId);
  for (const auto& node: descendants) {
    removeNodeFromCdata(node);
  }
  removeNodeFromCdata(*ninfoIt);
  bindParentChildEdges();
  m_formerSelectedNodeId.clear();

  m_treeRef->build();
  int depth = findNodeDepth(*ninfoIt);
  m_treeRef->expandToDepth(depth > 1 ? depth - 1 : 1);
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
    if (! node.parents.empty() && *node.parents.begin() == nodeId) {
      descendants.append(node);
    }
  }

  for (const auto& node:  m_cdata.cnodes) {
    if (! node.parents.empty() && *node.parents.begin() == nodeId) {
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
  bool success = ngrt4n::findNode(m_cdata.bpnodes, m_cdata.cnodes, *ninfo.parents.begin(), parentNodeIt);
  if (! success) {
    return -1;
  }

  return findNodeDepth(*parentNodeIt) + 1;
}


void WebEditor::bindParentChildEdges(void)
{
  m_cdata.edges.clear();
  for (const auto& node:  m_cdata.bpnodes) {
    if (! node.parents.empty()) {
      m_cdata.edges.insert(*node.parents.begin(), node.id);
    }
  }

  for (const auto& node:  m_cdata.cnodes) {
    if (! node.parents.empty()) {
      m_cdata.edges.insert(*node.parents.begin(), node.id);
    }
  }
}


void WebEditor::fillInEditorFromCurrentSelection(void)
{
  QString nodeId = m_treeRef->findNodeIdFromTreeItem(m_selectedTreeIndex);
  NodeListT::const_iterator nodeInfo;
  bool success = ngrt4n::findNode(m_cdata.bpnodes, m_cdata.cnodes, nodeId, nodeInfo);
  if (! success) {
    return;
  }
  fillInEditorFromNodeInfo(*nodeInfo);
  m_formerSelectedNodeId = nodeId;
}


void WebEditor::fillInEditorFromNodeInfo(const NodeT& ninfo)
{
  m_nameFieldRef->setText(ninfo.name.toStdString());
  m_descFieldRef->setText(ninfo.description.toStdString());
  m_iconFieldRef->setCurrentIndex(m_iconIndexMap[ninfo.icon]);
  m_calcRuleFieldRef->setCurrentIndex(ninfo.sev_crule);
  m_propRuleFieldRef->setCurrentIndex(ninfo.sev_prule);
  m_typeFieldRef->setCurrentIndex(ninfo.type);
  m_typeFieldRef->activated().emit(ninfo.type);

  switch (ninfo.type) {
  case NodeType::ITService:
    m_checkFieldRef->setText(ninfo.child_nodes.toStdString());
    break;
  case NodeType::ExternalService:
    m_externalTypeSelectorFieldRef->setValueText(ninfo.child_nodes.toStdString());
    break;
  default:
    break;
  }

  m_externalTypeSelectorFieldRef->setHidden(NodeType::ExternalService != ninfo.type);
  m_checkSelectorPanelRef->setDisabled(ninfo.type != NodeType::ITService);
}


void WebEditor::updateNodeDataFromEditor(const QString& nodeId)
{
  NodeListT::iterator nodeIt;
  bool success = ngrt4n::findNode(m_cdata.bpnodes, m_cdata.cnodes, nodeId, nodeIt);
  if (! success) {
    return ;
  }

  nodeIt->name =  QString::fromStdString(m_nameFieldRef->text().toUTF8());
  nodeIt->description =  QString::fromStdString(m_descFieldRef->text().toUTF8());
  nodeIt->type =  m_typeFieldRef->currentIndex();
  nodeIt->icon = QString::fromStdString(m_iconFieldRef->currentText().toUTF8());
  nodeIt->sev_crule  = m_calcRuleFieldRef->currentIndex();
  nodeIt->sev_prule  = m_propRuleFieldRef->currentIndex();

  switch (nodeIt->type) {
  case NodeType::ITService:
    nodeIt->child_nodes = QString::fromStdString(m_checkFieldRef->text().toUTF8() );
    break;
  case NodeType::ExternalService:
    if (m_externalTypeSelectorFieldRef->currentIndex() != 0) {
      nodeIt->child_nodes = QString::fromStdString( m_externalTypeSelectorFieldRef->currentText().toUTF8() );
    }
    break;
  default:
    break;
  }

  m_treeRef->updateItemLabel(nodeId, nodeIt->name.toStdString());
}

void WebEditor::refreshDynamicContents(void)
{
  m_checkSourceFieldRef->clear();
  m_checkSourceFieldRef->addItem(Q_TR("Set a source for autocompletion"));
  auto sources = m_dbSession->listSources(MonitorT::Any);
  for (const auto& sinfo: sources) {
    m_checkSourceFieldRef->addItem(sinfo.id.toStdString());
  }
  m_externalTypeSelectorFieldRef->clear();
  m_externalTypeSelectorFieldRef->addItem(Q_TR("Select an external service"));
  for (auto&& v: m_dbSession->listViews()) {
    m_externalTypeSelectorFieldRef->addItem(v.name);
  }
}


void WebEditor::handleNodeLabelChanged(void)
{
  m_treeRef->updateItemLabel(m_formerSelectedNodeId, m_nameFieldRef->text().toUTF8());
  updateNodeDataFromEditor(m_formerSelectedNodeId);
}

void WebEditor::handleDataPointChanged(void)
{
  if (m_checkFieldRef->isEnabled()) {
    auto dataPoint =m_checkFieldRef->text().toUTF8();
    size_t slashIndex = dataPoint.find("/");
    m_nameFieldRef->setText(dataPoint.substr(slashIndex + 1) + "("  + dataPoint.substr(0, slashIndex) + ")");
  }
}


void WebEditor::handleNodeTypeChanged(int index)
{
  m_checkSelectorPanelRef->setDisabled(index != NodeType::ITService);
  m_externalTypeSelectorFieldRef->setHidden(index != NodeType::ExternalService);

  QString nodeId = m_treeRef->findNodeIdFromTreeItem(m_selectedTreeIndex);
  NodeListT::ConstIterator ninfoIt;
  if ( ! ngrt4n::findNode(m_cdata.bpnodes, m_cdata.cnodes, nodeId, ninfoIt) ) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Please select a node first"));
    return;
  }

  // get the node info before the iterator to expire
  NodeT nodeInfo = *ninfoIt;
  nodeInfo.child_nodes.clear();

  if (index == nodeInfo.type) {
    return ;
  }

  if ( QSet<int>{NodeType::ITService, NodeType::ExternalService}.contains(index) ) {
    if ( ! findDescendantNodes(nodeId).empty() ) {
      m_typeFieldRef->setCurrentIndex(nodeInfo.type);
      m_typeFieldRef->activated().emit(nodeInfo.type);
      m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Type not allowed for item with descendants"));
      return ;
    }
  }

  switch (index) {
  case NodeType::ITService:
    if (m_cdata.bpnodes.remove(nodeInfo.id) > 0) {
      m_cdata.cnodes.insert(nodeInfo.id, nodeInfo);
    } else {
      m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Node not found in parent list"));
    }
    break;
  default: // => NodeType::ExternalService or NodeType::BusinessService:
    if ( QSet<int>{NodeType::ExternalService, NodeType::BusinessService}.contains(nodeInfo.type) ) {
      m_cdata.bpnodes[nodeId].child_nodes.clear(); // typically for external service child_nodes is not empty
      break;
    }
    if (m_cdata.cnodes.remove(nodeInfo.id) > 0) {
      m_cdata.bpnodes.insert(nodeInfo.id, nodeInfo);
    } else {
      m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Node not found in child list"));
    }
    break;

  }
  updateNodeDataFromEditor(nodeInfo.id);
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
  ngrt4n::fixupDependencies(m_cdata);

  auto outRegisterView = registerViewWithPath(m_cdata, destPath.c_str());
  if (outRegisterView.first != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, outRegisterView.second.toStdString());
  } else {
    m_operationCompleted.emit(ngrt4n::OperationSucceeded, Q_TR("Saved"));
    m_currentFilePath = destPath;
  }
}


std::pair<int, QString> WebEditor::registerViewWithPath(const CoreDataT& cdata, const QString& destPath)
{
  auto rootService = cdata.bpnodes.constFind(ngrt4n::ROOT_ID);
  if (rootService == cdata.bpnodes.cend()) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("Invalid or incompleted view with root"));
  }

  std::pair<int, QString> saveResult = ngrt4n::saveViewDataToPath(cdata, destPath);
  if (saveResult.first != ngrt4n::RcSuccess) {
    return std::make_pair(ngrt4n::RcGenericFailure, saveResult.second);
  }

  DboView vinfo;
  vinfo.name = rootService->name.toStdString();
  vinfo.service_count = cdata.bpnodes.size() + cdata.cnodes.size();
  vinfo.path = destPath.toStdString();

  // save view in database if it's the 1st time
  if (m_currentFilePath.empty()) {
    auto addViewOut = m_dbSession->addView(vinfo);
    if (addViewOut.first != ngrt4n::RcSuccess) {
      CORE_LOG("error", addViewOut.second.toStdString());
      return addViewOut;
    }
    return std::make_pair(ngrt4n::RcSuccess, "");
  }

  auto updateViewOut = m_dbSession->updateViewWithPath(vinfo, destPath.toStdString());
  if (updateViewOut.first !=  ngrt4n::RcSuccess) {
    CORE_LOG("error", updateViewOut.second.toStdString());
    return updateViewOut;
  }

  return std::make_pair(ngrt4n::RcSuccess, "");
}



void WebEditor::handleImportMonitoringConfigButton(void)
{
  auto sources = m_dbSession->listSources(MonitorT::Any);
  m_configImporter.updateContentWithSourceList(sources.keys(), WebInputField::SourceWithTextFilter);
  m_configImporter.show();
}


void WebEditor::handleImportZabbixItServiceButton(void)
{
  auto sources = m_dbSession->listSources(MonitorT::Zabbix);
  m_zabbixITViewSelector.updateContentWithSourceList(sources.keys(), WebInputField::SourceOnly);
  m_zabbixITViewSelector.show();
}

void WebEditor::handleImportNagiosBpiButton(void)
{
  auto sources = m_dbSession->listSources(MonitorT::Nagios);
  m_nagiosBPISelector.updateContentWithSourceList(sources.keys(), WebInputField::SourceWithFileFilter);
  m_nagiosBPISelector.show();
}




void WebEditor::importMonitoringConfig(const std::string& srcId, const std::string& groupFilter)
{
  if (srcId.empty()) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("No source selected"));
    return ;
  }

  auto findSourceOut = m_dbSession->findSourceById(srcId.c_str());
  if (! findSourceOut.first) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("No source with id: %1").arg(srcId.c_str()).toStdString());
    return ;
  }

  CoreDataT cdata;
  auto loadViewByGroupOut = ngrt4n::loadDynamicViewByGroup(findSourceOut.second, groupFilter.c_str(), cdata);
  if (loadViewByGroupOut.first != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, loadViewByGroupOut.second.toStdString());
    return ;
  }
  ngrt4n::fixupDependencies(cdata);

  m_currentFilePath.clear();

  auto destPath = QString("%1/%2_autoimport.ms.ngrt4n.xml").arg(m_configDir, ngrt4n::generateId());
  auto saveStatus = registerViewWithPath(cdata, destPath);
  if (saveStatus.first != 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, saveStatus.second.toStdString());
    return ;
  }

  handleOpenFile(destPath.toStdString(), "");
}

void WebEditor::handleDataPointSourceChanged(int index)
{
  m_operationCompleted.emit(ngrt4n::OperationInProgress, Q_TR("Discovery of monitoring items in progress..."));

  auto srcId = QString::fromStdString(m_checkSourceFieldRef->itemText(index).toUTF8());
  auto findSourceOut = m_dbSession->findSourceById(srcId);
  if (! findSourceOut.first) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, Q_TR("Cannot load source info"));
    return ;
  }

  if (! m_dataPointsListBySource[srcId].empty()) {
    m_operationCompleted.emit(ngrt4n::OperationFinished, Q_TR("Nothing to do"));
    return ;
  }

  ChecksT checks;
  auto importResult = ngrt4n::loadDataItems(findSourceOut.second, "", checks);
  if (importResult.first != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, importResult.second.toStdString());
    return;
  }

  const std::string ALL_GROUPS = Q_TR("All groups");
  const std::string NO_GROUP = Q_TR("Unclassified");

  m_checkSuggestionModelRef->removeRows(0, m_checkSuggestionModelRef->rowCount());
  m_checkGroupFieldRef->clear();

  m_checkGroupFieldRef->addItem(ALL_GROUPS);
  for (const auto& check: checks) {
    std::string dataPoint = QString("%1:%2").arg(srcId, check.id.c_str()).toStdString();
    QStringList groups = QString::fromStdString(check.host_groups).split(ngrt4n::CHILD_Q_SEP);

    if (groups.isEmpty()) {
      m_dataPointsListByGroup[NO_GROUP.c_str()].push_back(dataPoint);
      m_checkGroupFieldRef->addItem(NO_GROUP);
    } else {
      for (const auto& group: groups) {
        auto gpKey = group.toStdString();
        m_dataPointsListByGroup[gpKey].push_back(dataPoint);
        if (m_checkGroupFieldRef->findText(gpKey) < 0) {
          m_checkGroupFieldRef->addItem(gpKey);
        }
      }
      m_dataPointsListBySource[srcId].push_back(dataPoint);
    }
  }

  m_checkSuggestionModelRef->setStringList(m_dataPointsListBySource[srcId]);
  m_checkFieldRef->setPlaceholderText(QObject::tr("Autocompletion enabled for %1").arg(srcId).toStdString());
  m_operationCompleted.emit(ngrt4n::OperationSucceeded, Q_TR("Discovery of monitoring items completed"));
}


void WebEditor::handleDataPointGroupChanged(int)
{
  std::string group = m_checkGroupFieldRef->currentText().toUTF8();
  m_checkSuggestionModelRef->setStringList(m_dataPointsListByGroup[group]);
  m_checkFieldRef->setPlaceholderText(QObject::tr("Autocompletion enabled for '%1' group").arg(group.c_str()).toStdString());
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
  rootSrv.name = QObject::tr("Nagios BPI Services"); //TODO added a prefix ?
  rootSrv.type = NodeType::BusinessService;
  rootSrv.sev = ngrt4n::Unknown;

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

    QString groupId = fields[1];

    NodeListT::Iterator currentGroupSrv = cdata.bpnodes.find(groupId);
    if (currentGroupSrv == cdata.bpnodes.end()) {
      NodeT groupSrv;
      groupSrv.id = groupId;
      groupSrv.name = groupId;
      groupSrv.type = NodeType::BusinessService;
      groupSrv.parents.clear();
      groupSrv.sev_prule = PropRules::Unchanged;
      groupSrv.sev_crule = CalcRules::Worst;
      groupSrv.weight = ngrt4n::WEIGHT_UNIT;
      groupSrv.icon = ngrt4n::DEFAULT_ICON;
      currentGroupSrv = cdata.bpnodes.insert(groupId, groupSrv);
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

  auto saveStatus = registerViewWithPath(cdata, destPath);
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
    if (! node.parents.isEmpty()) {
      continue;
    }

    node.parents = QSet<QString>{ root.id };
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
        currentMemberSrvIt->parents = QSet<QString>{ parentServiceId };
      } else {
        NodeT memberSrv;
        memberSrv.id = memberId;
        memberSrv.name = memberLabel;
        memberSrv.type = NodeType::BusinessService;
        memberSrv.parents = QSet<QString>{parentServiceId};
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
        cnode.parents = QSet<QString>{ parentServiceId };
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
  auto allZbxSources = m_dbSession->listSources(MonitorT::Zabbix);
  auto outLoadItServices = ZbxHelper().loadITServices(allZbxSources[srcId.c_str()], cdata);
  if (outLoadItServices.first != ngrt4n::RcSuccess) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Importation failed: %1").arg(outLoadItServices.second).toStdString());
    return ;
  }

  m_currentFilePath.clear();
  auto destPath = QString("%1/%2_autoimport.ms.ngrt4n.xml").arg(m_configDir, ngrt4n::generateId());
  auto saveStatus = registerViewWithPath(cdata, destPath);
  if (saveStatus.first != 0) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, saveStatus.second.toStdString());
    return ;
  }

  handleOpenFile(destPath.toStdString(), "");

  m_operationCompleted.emit(ngrt4n::OperationSucceeded, "Completed");
}
