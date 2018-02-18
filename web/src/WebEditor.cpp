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
}

WebEditor::WebEditor(void) {
  m_tree.setCoreData(&m_cdata);
  activateTreeEditionFeatures();
  bindFormWidgets();
  newView();
}

WebEditor::~WebEditor()
{
  unbindWidgets();
}


void  WebEditor::openViewFile(const QString& path)
{
  //TODO: m_tree.setCoreData(&m_cdata);
}

void WebEditor::bindFormWidgets(void)
{
  setLayout(m_mainLayout = new Wt::WHBoxLayout());

  m_mainLayout->setContentsMargins(0, 0, 0, 0);


  m_mainLayout->addWidget(&m_tree);
  m_mainLayout->addWidget(&m_fieldEditionPane);

  m_mainLayout->setSpacing(2);
  m_mainLayout->setResizable(0);
  m_mainLayout->setResizable(1);

  bindEditionForm();
}


void WebEditor::unbindWidgets(void)
{
  m_mainLayout->removeWidget(&m_tree);
  m_mainLayout->removeWidget(&m_fieldEditionPane);
  clear();
}



void WebEditor::bindEditionForm(void)
{
  m_saveBtn.setText(Q_TR("Save"));

  // set node type values
  m_typeField.addItem(NodeType::toString(NodeType::BusinessService).toStdString());
  m_typeField.addItem(NodeType::toString(NodeType::ITService).toStdString());
  m_typeField.addItem(NodeType::toString(NodeType::ExternalService).toStdString());

  // set icon type values
  m_iconBox.addItem(QObject::tr("-->elect an icon (Default is %1)").arg(ngrt4n::DEFAULT_ICON).toStdString());
  for (const auto& icon: ngrt4n::nodeIcons().keys()) {
    m_iconBox.addItem(icon.toStdString());
  }


  // set propagation rules
  m_propRuleBox.addItem(QObject::tr("-->Select a rule (Default is %1)").arg(PropRules(PropRules::Unchanged).toString()).toStdString());
  Q_FOREACH(const QString& rule, DashboardBase::propRules().keys()) {
    m_propRuleBox.addItem(rule.toStdString());
  }

  // set calculation rules
  m_calcRuleBox.addItem(QObject::tr("-->Select a rule (Default is %1)").arg(CalcRules(CalcRules::Worst).toString()).toStdString());
  Q_FOREACH(const QString& rule, DashboardBase::calcRules().keys()) {
    m_calcRuleBox.addItem(rule.toStdString());
  }

  // styling buttons
  m_saveBtn.setStyleClass("btn btn-info");

  // bind template fields
  m_fieldEditionPane.setTemplateText(Wt::WString::tr("editor-fields-form.tpl"));
  m_fieldEditionPane.bindWidget("name-field", &m_nameField);
  m_fieldEditionPane.bindWidget("type-field", &m_typeField);
  m_fieldEditionPane.bindWidget("calc-rule-field", &m_calcRuleBox);
  m_fieldEditionPane.bindWidget("prop-rule-field", &m_propRuleBox);
  m_fieldEditionPane.bindWidget("icon-field", &m_iconBox);
  m_fieldEditionPane.bindWidget("description-field", &m_descField);
  m_fieldEditionPane.bindWidget("monitoring-item-field", &m_checkItemField);
  m_fieldEditionPane.bindWidget("save-button", &m_saveBtn);
}


NodeT WebEditor::createNode(const QString& id, const QString& label,const QString& parent)
{
  NodeT node;
  node.id = id;
  node.name = label;
  node.parent = parent;
  node.type = NodeType::BusinessService;
  node.sev = ngrt4n::Unknown;
  node.sev_crule = CalcRules::Worst;
  node.sev_prule = PropRules::Unchanged;
  node.icon = ngrt4n::DEFAULT_ICON;
  node.child_nodes = QString();
  node.thresholdLimits = QVector<ThresholdT>();
  return node;
}

void WebEditor::newView(void)
{
  //FIXME: if (treatCloseAction(false) == 0) {
  ngrt4n::clearCoreData(m_cdata);
  //FIXME: m_activeConfig.clear();

  //FIXME: m_tree.clearTree();
  NodeT node = createNode(ngrt4n::ROOT_ID, QObject::tr("New View"), "");
  m_cdata.bpnodes.insert(node.id, node);
  reload();

  //FIXME: refreshUIWidgets();
  //FIXME:}
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

  m_menuAddSubService = m_editionContextMenu.addItem("Add sub service");
  m_menuAddSubService->triggered().connect(this, &WebEditor::handleTreeContextMenu);


  m_menuDeleteService = m_editionContextMenu.addItem("Delete service");
  //setAttributeValue("oncontextmenu", "event.cancelBubble = true; event.returnValue = false; return false;");

  m_tree.doubleClicked().connect(this, &WebEditor::showTreeContextMenu);
}

void WebEditor::showTreeContextMenu(void)
{
  Wt::WModelIndexSet sitems = m_tree.selectedIndexes();
  if (sitems.empty()) {
    return;
  }

  //sitem = *sitems.cbegin()

  m_editionContextMenu.popup(Wt::WPoint(50, 50));

}

void WebEditor::handleTreeContextMenu(Wt::WMenuItem* menu)
{
  qDebug() << menu->text().toUTF8().c_str();
}


