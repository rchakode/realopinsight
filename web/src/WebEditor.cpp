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



namespace {
  const IconMapT ICONS = ngrt4n::nodeIcons();
}

WebEditor::WebEditor(void) {

  bindFormWidgets();
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
  QString header = QString(Q_TR("-->Select an icon (Default is %1)").c_str()).arg(ngrt4n::DEFAULT_ICON);
  m_iconBox.addItem(header.toStdString());
  Q_FOREACH(const QString& label, ngrt4n::nodeIcons().keys()) {
    //QString path = icons.value(label);
    m_iconBox.addItem(label.toStdString());
  }

  // set propagation rules
  Q_FOREACH(const QString& rule, DashboardBase::propRules().keys()) {
    m_propRuleBox.addItem(rule.toStdString());
  }

  // set calculation rules
  Q_FOREACH(const QString& rule, DashboardBase::calcRules().keys()) {
    m_calcRuleBox.addItem(rule.toStdString());
  }


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


