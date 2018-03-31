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
  const QString CHILD_SEPERATOR(ngrt4n::CHILD_SEP.c_str());
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


void  WebEditor::handleOpenFile(const std::string& path, const std::string& option)
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

  m_newServiceViewBtn.clicked().connect(this, &WebEditor::handleNewViewButton);

  // open service button
  m_openServiceViewBtn.setToolTip(Q_TR("Open and edit an existing service view"));
  m_openServiceViewBtn.setImageLink(Wt::WLink("images/built-in/open.png"));
  m_openServiceViewBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("open-service-view", &m_openServiceViewBtn);

  m_openServiceViewBtn.clicked().connect(this, &WebEditor::handleOpenViewButton);
  m_openViewDialog.dataSelectionTriggered().connect(this, &WebEditor::handleOpenFile);

  // save service button
  m_saveCurrentViewBtn.setToolTip(Q_TR("Save changes"));
  m_saveCurrentViewBtn.setImageLink(Wt::WLink("images/built-in/save.png"));
  m_saveCurrentViewBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("save-current-view", &m_saveCurrentViewBtn);

  m_saveCurrentViewBtn.clicked().connect(this, &WebEditor::handleSaveViewButton);

  // import native button
  m_importNativeConfigBtn.setToolTip(Q_TR("Import of native monitoring settings as service tree"));
  m_importNativeConfigBtn.setImageLink(Wt::WLink("images/built-in/import.png"));
  m_importNativeConfigBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("import-native-config", &m_importNativeConfigBtn);

  m_importNativeConfigBtn.clicked().connect(this, &WebEditor::handleImportNativeConfigButton);
  m_importNativeConfigDialog.dataSelectionTriggered().connect(this, &WebEditor::importNativeConfig);

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

  m_dataPointSourceField.activated().connect(this, &WebEditor::handleDataPointSourceChanged);
  m_dataPointGroupField.activated().connect(this, &WebEditor::handleDataPointGroupChanged);

  // options auto completion
  Wt::WSuggestionPopup::Options dataPointSuggestionOptions;
  dataPointSuggestionOptions.highlightBeginTag = "<span class=\"highlight\">";
  dataPointSuggestionOptions.highlightEndTag = "</span>";
  dataPointSuggestionOptions.listSeparator = ',';
  dataPointSuggestionOptions.whitespace = " \\n";
  dataPointSuggestionOptions.wordSeparators = "-., \":\\n;";
  dataPointSuggestionOptions.appendReplacedText = ", ";

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
    return ;
  }

  switch (event.key()) {
    case Wt::Key_C:
      addSubServiceFromTreeNodeIndex(m_selectedTreeItemIndex);
      break;
    case Wt::Key_X:
      removeServiceByTreeNodeIndex(m_selectedTreeItemIndex);
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

  WebBaseSettings settings;
  m_dataPointSourceField.addItem(Q_TR("Set a source for autocompletion"));
  for (const auto& sinfo: settings.fetchSourceList(MonitorT::Auto)) {
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


void WebEditor::handleNodeTypeChanged(void)
{
  const int type = m_typeField.currentIndex();

  m_typeExternalServiceNameField.setHidden(type != NodeType::ExternalService);
  m_dataPointItemsContainer.setDisabled(type != NodeType::ITService);

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
        break; // do nothing as in this case the node should be already on the right list
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

  fixParentChildrenDependencies();

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



void WebEditor::fixParentChildrenDependencies(void)
{
  for (const auto& srv_it: m_cdata.bpnodes) {
    setParentChildDependency(srv_it.id, srv_it.parent);
  }

  for (const auto& srv_it: m_cdata.cnodes) {
    setParentChildDependency(srv_it.id, srv_it.parent);
  }
}


void WebEditor::setParentChildDependency(const QString& childId, const QString& parentId)
{
  auto parent_it = m_cdata.bpnodes.find(parentId);
  if (parent_it == m_cdata.bpnodes.end()) {
    return ;
  }

  if (parent_it->child_nodes.isEmpty()) {
    parent_it->child_nodes = childId;
  } else {
    parent_it->child_nodes += (CHILD_SEPERATOR % childId);
  }
}



void WebEditor::handleImportNativeConfigButton(void)
{
  WebBaseSettings settings;
  auto sources = settings.fetchSourceList(MonitorT::Auto);

  m_importNativeConfigDialog.updateContentWithSourceList(sources.keys());
  m_importNativeConfigDialog.show();
}



void WebEditor::importNativeConfig(const std::string& srcId, const std::string& groupFilter)
{
  WebBaseSettings settings;

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

  QString destPath = QString("%1/%2_autoimport.ms.ngrt4n.xml").arg(m_configDir, ngrt4n::generateId());

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


void WebEditor::handleDataPointGroupChanged(int index)
{
  std::string group = m_dataPointGroupField.currentText().toUTF8();
  m_dataPointListModel->setStringList(m_dataPointsListByGroup[group]);

  m_dataPointField.setPlaceholderText(QObject::tr("Autocompletion enabled for '%1' group").arg(group.c_str()).toStdString());
}
