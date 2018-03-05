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

const QMap<int, std::string> WebEditor::MENU_TEXTS = {
  {WebEditor::MENU_ADD_SUBSERVICE, Q_TR("Add sub service")},
  {WebEditor::MENU_DELETE_SUBSERVICE, Q_TR("Delete sub service")}
};

WebEditor::WebEditor(void) :
  m_operationCompleted(this)
{
  m_tree.setCoreData(&m_cdata);
  activateTreeEditionFeatures();
  bindMainPanes();
  bindFormWidgets();
}

WebEditor::~WebEditor()
{
  unbindWidgets();
}


void  WebEditor::openServiceView(void)
{
  //TODO: m_tree.setCoreData(&m_cdata);
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
  clear();
}



void WebEditor::bindFormWidgets(void)
{
  m_fieldEditionPane.setTemplateText(Wt::WString::tr("editor-fields-form.tpl"));

  // new service button
  m_newServiceViewBtn.clicked().connect(this, &WebEditor::newView);
  m_newServiceViewBtn.setImageLink(Wt::WLink("images/built-in/new.png"));
  m_newServiceViewBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("new-service-view", &m_newServiceViewBtn);


  // open service button
  m_openServiceViewBtn.clicked().connect(this, &WebEditor::openServiceView);
  m_openServiceViewBtn.setImageLink(Wt::WLink("images/built-in/open.png"));
  m_openServiceViewBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("open-service-view", &m_openServiceViewBtn);

  // save service button
  m_saveCurrentViewBtn.setImageLink(Wt::WLink("images/built-in/save.png"));
  m_saveCurrentViewBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("save-current-view", &m_saveCurrentViewBtn);

  // rename service button
  m_renameCurrentViewBtn.setImageLink(Wt::WLink("images/built-in/rename.png"));
  m_renameCurrentViewBtn.setStyleClass("btn");
  m_fieldEditionPane.bindWidget("rename-current-view", &m_renameCurrentViewBtn);

  // name field
  m_fieldEditionPane.bindWidget("name-field", &m_nameField);
  m_nameField.blurred().connect(this, &WebEditor::handleNodeLabelChanged);


  // set node type values
  m_fieldEditionPane.bindWidget("type-field", &m_typeField);

  m_typeField.addItem(NodeType::toString(NodeType::BusinessService).toStdString());
  m_typeField.addItem(NodeType::toString(NodeType::ITService).toStdString());
  m_typeField.addItem(NodeType::toString(NodeType::ExternalService).toStdString());

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
  m_fieldEditionPane.bindWidget("monitoring-item-field", &m_dataPointField);
}



void WebEditor::newView(void)
{
  //FIXME: if (treatCloseAction(false) == 0) {
  ngrt4n::clearCoreData(m_cdata);
  //FIXME: m_activeConfig.clear();

  //FIXME: m_tree.clearTree();
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

  reload();
}


void WebEditor::reload(void)
{
  //FIXME: m_hasLeftUpdates = true;

  //FIXME: fillInEditorFields(m_cdata.bpnodes.find(ngrt4n::ROOT_ID));
  m_tree.build();
  //fillEditorFromService(m_tree.rootItem());
  //FIXME: updateWindowTitle("*");
}


void WebEditor::activateTreeEditionFeatures()
{
  m_tree.activateEditionFeatures();

  m_editionContextMenu.addItem("images/plus.png", MENU_TEXTS[MENU_ADD_SUBSERVICE])
      ->triggered().connect(this, &WebEditor::handleTreeContextMenu);

  m_editionContextMenu.addItem("images/minus.png", MENU_TEXTS[MENU_DELETE_SUBSERVICE])
      ->triggered().connect(this, &WebEditor::handleTreeContextMenu);

  m_tree.selectionChanged().connect(this, &WebEditor::handleTreeItemSelectionChanged);
  m_tree.doubleClicked().connect(this, &WebEditor::showTreeContextMenu);
  m_tree.keyPressed().connect(this, &WebEditor::handleKeyPressed);
}


void WebEditor::handleTreeItemSelectionChanged(void)
{
  updateNodeDataFromEditor(m_formerSelectedNodeId);

  Wt::WModelIndexSet selectedTreeItems = m_tree.selectedIndexes();
  if (! selectedTreeItems.empty()) {
    m_currentTreeItemIndex = *(selectedTreeItems.begin());
    fillInEditorFromCurrentSelection();
  } else {
    m_currentTreeItemIndex = Wt::WModelIndex();
  }
}


void WebEditor::showTreeContextMenu(Wt::WModelIndex, Wt::WMouseEvent event) {
  if (m_currentTreeItemIndex.isValid()) {
    m_editionContextMenu.popup(event);
  }
}


void WebEditor::handleTreeContextMenu(Wt::WMenuItem* menu)
{
  std::string triggeredMenu = menu->text().toUTF8();

  if (triggeredMenu == MENU_TEXTS[ MENU_ADD_SUBSERVICE ]) {
    addNewSubService(m_currentTreeItemIndex);
  } else if (triggeredMenu == MENU_TEXTS[ MENU_DELETE_SUBSERVICE ] ) {
    //TODO delete service
  }
}


void WebEditor::handleKeyPressed(Wt::WKeyEvent event)
{
  if (event.modifiers() == Wt::ShiftModifier && event.key() == Wt::Key_C) {
    addNewSubService(m_currentTreeItemIndex);
  }
}

void WebEditor::addNewSubService(const Wt::WModelIndex& currentTreeItemIndex)
{
  if (! currentTreeItemIndex.isValid()) {
    return ;
  }

  QString currentSrvId = m_tree.getNodeIdFromTreeItem(currentTreeItemIndex);

  NodeT childSrv;
  childSrv.id = ngrt4n::genNodeId();
  childSrv.name = "New service";
  childSrv.type = NodeType::BusinessService;
  childSrv.sev_prule = PropRules::Unchanged;
  childSrv.sev_crule = CalcRules::Worst;
  childSrv.weight = ngrt4n::WEIGHT_UNIT;
  childSrv.icon = ngrt4n::DEFAULT_ICON;
  childSrv.parent = currentSrvId;


  NodeListT::iterator parentSrv = m_cdata.bpnodes.find(currentSrvId);
  if (parentSrv == m_cdata.bpnodes.end()) {
    CORE_LOG("debug", QObject::tr("Not node found with parent id: %1").arg(currentSrvId).toStdString());
    return;
  }

  m_cdata.bpnodes.insert(childSrv.id, childSrv);

  if (parentSrv->type != NodeType::BusinessService) {
    m_operationCompleted.emit(ngrt4n::OperationFailed, QObject::tr("Action not allowed on parent node type: %1").arg(NodeType::toString(parentSrv->type)).toStdString());
    return;
  }

  if (parentSrv->child_nodes.isEmpty()) {
    parentSrv->child_nodes = childSrv.id;
  } else {
    parentSrv->child_nodes.append(CHILD_SEPERATOR % childSrv.id);
  }


  bool bindToParent = true;
  Wt::WStandardItem* subSrvItem = m_tree.addTreeEntry(childSrv, bindToParent);

  Wt::WModelIndexSet itemsToSelect;
  m_tree.expand(currentTreeItemIndex);
  itemsToSelect.insert(subSrvItem->index());
  m_tree.setSelectedIndexes(itemsToSelect);
}


void WebEditor::fillInEditorFromCurrentSelection(void)
{
  QString nodeId = m_tree.getNodeIdFromTreeItem(m_currentTreeItemIndex);
  NodeListT::const_iterator node_cit;

  if (ngrt4n::findNode(m_cdata.bpnodes, m_cdata.cnodes, nodeId, node_cit)) {
    m_formerSelectedNodeId = nodeId;
    m_nameField.setText(node_cit->name.toStdString());
    m_descField.setText(node_cit->description.toStdString());
    m_typeField.setCurrentIndex(node_cit->type);
    m_iconBox.setCurrentIndex(m_iconIndexMap[node_cit->icon]);
    m_calcRuleBox.setCurrentIndex(node_cit->sev_crule);
    m_propRuleBox.setCurrentIndex(node_cit->sev_prule);
    m_dataPointField.setText(node_cit->child_nodes.toStdString());
  }
}


void WebEditor::updateNodeDataFromEditor(const QString& nodeId)
{
  NodeListT::iterator node_it;
  if (ngrt4n::findNode(m_cdata.bpnodes, m_cdata.cnodes, nodeId, node_it)) {
    node_it->name =  QString::fromStdString(m_nameField.text().toUTF8());
    node_it->description =  QString::fromStdString(m_descField.text().toUTF8());
    node_it->type =  m_typeField.currentIndex();
    node_it->icon = QString::fromStdString(m_iconBox.currentText().toUTF8());
    node_it->sev_crule  = m_calcRuleBox.currentIndex();
    node_it->sev_prule  = m_propRuleBox.currentIndex();
    node_it->child_nodes  = QString::fromStdString(m_dataPointField.text().toUTF8());
  }
}


void WebEditor::handleNodeLabelChanged(void)
{
  m_tree.updateItemLabel(m_formerSelectedNodeId, m_nameField.text().toUTF8().c_str());
}
