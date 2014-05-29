/*
 * ServiceEditor.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
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


#include "ServiceEditor.hpp"
#include "Preferences.hpp"
#include "GraphView.hpp"
#include "DashboardBase.hpp"
#include "utilsCore.hpp"

typedef QList<QListWidgetItem*> CheckItemList;


ServiceEditor::ServiceEditor(QWidget* _parent )
  : QWidget(_parent),
    msettings(new Settings()),
    mlayout(new QGridLayout( this ) ),
    buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close))
{
  mitems["nameLabel"] = new QLabel(tr("Name"));
  mitems[NAME_FIELD] = new QLineEdit();

  mitems["typeLabel"] = new QLabel(tr("Type"));
  mitems[TYPE_FIELD] = new QComboBox();

  mitems["iconNameLabel"] = new QLabel(tr("Icon"));
  mitems[ICON_FIELD] = new QComboBox();

  mitems["priorityLabel"] = new QLabel(tr("Severity Handling Rules"));
  mitems[STATUS_CALC_RULE_FIELD] = new QComboBox();
  mitems[STATUS_PROP_RULE_FIELD] = new QComboBox();

  mitems["descriptionLabel"] = new QLabel(tr("Description"));
  mitems[DESCRIPTION_FIELD] = new QTextEdit();

  mitems["alarmMsgLabel"] = new QLabel(tr("Alarm Message"));
  mitems[ALARM_MSG_FIELD]  = new QTextEdit();

  mitems["notificationMsgLabel"] = new QLabel(tr("Notification Message"));
  mitems[NOTIFICATION_MSG_FIELD] = new QTextEdit();

  mitems["lowLevelAlarmsLabel"] = new QLabel(tr("Data Point"));
  mitems[CHECK_FIELD] = new QComboBox();
  mitems[CHECK_LIST_FIELD] = new QListWidget();

  layoutEditorComponents();

  addEvent();
}

ServiceEditor::~ServiceEditor()
{
  QMap<QString, QWidget*>::iterator iter = mitems.begin();

  while (iter != mitems.end()) {
    QString key = iter.key();
    QWidget* widget = mitems[key];
    QLabel* labelPtr = dynamic_cast<QLabel*>(widget);
    QLineEdit* lineEditPtr = dynamic_cast<QLineEdit*>(widget);
    QTextBlock* textBlockPtr = dynamic_cast<QTextBlock*>(widget);
    QComboBox* comboBoxPtr = dynamic_cast<QComboBox*>(widget);

    if(labelPtr) {
      delete labelPtr;
    } else if(lineEditPtr) {
      delete  lineEditPtr;
    } else if (textBlockPtr) {
      delete textBlockPtr;
    } else if (comboBoxPtr){
      delete comboBoxPtr;
    }
    mitems.remove(key);

    iter = mitems.begin();
  }
  mitems.clear();

  delete buttonBox;
  delete mlayout;

}

void ServiceEditor::loadChecks(const ChecksT& checks, const QString& srcId)
{
  checkField()->clear();
  for(ChecksT::const_iterator it=checks.begin(), end=checks.end(); it!=end; ++it) {
    checkField()->addItem(QString("%1:%2").arg(srcId, (it->second).id.c_str()));
  }
}

void ServiceEditor::setEnableFields(const bool& enable)
{
  mitems[CHECK_FIELD]->setEnabled(enable);
  mitems[CHECK_LIST_FIELD]->setEnabled(enable);
  mitems[ALARM_MSG_FIELD]->setEnabled(enable);
  mitems[NOTIFICATION_MSG_FIELD]->setEnabled(enable);
}

bool ServiceEditor::updateNodeContent(NodeListT& _nodes, const QString& _nodeId)
{
  NodeListT::iterator node = static_cast<const NodeListT::iterator>(_nodes.find(_nodeId));
  if( node != _nodes.end()) {
    node->name = nameField()->text();
    node->type = typeField()->currentIndex();
    node->sev_crule = statusCalcRuleField()->currentIndex();
    node->sev_prule = statusPropRuleField()->currentIndex();
    node->icon = iconField()->currentText();
    node->description = descriptionField()->toPlainText();
    node->alarm_msg  = alarmMsgField()->toPlainText();
    node->notification_msg = notificationMsgField()->toPlainText();
    if(node->type == NodeType::AlarmNode)
      node->child_nodes = checkField()->currentText();
    return true;
  }
  return false;
}


bool ServiceEditor::updateNodeContent(NodeListT::iterator& _node)
{
  _node->name             = nameField()->text();
  _node->type             = typeField()->currentIndex();
  _node->sev_crule        = statusCalcRuleField()->currentIndex();
  _node->sev_prule        = statusPropRuleField()->currentIndex();
  _node->icon             = iconField()->currentText();
  _node->description      = descriptionField()->toPlainText();
  _node->alarm_msg        = alarmMsgField()->toPlainText();
  _node->notification_msg = notificationMsgField()->toPlainText();
  if(_node->type == NodeType::AlarmNode) _node->child_nodes = checkField()->currentText();

  return true;
}

void ServiceEditor::fillFormWithNodeContent(const NodeListT& nodes, const QString& nodeId)
{
  NodeListT::const_iterator node = nodes.find(nodeId);
  if( node != nodes.end()) fillFormWithNodeContent(node);
}



void ServiceEditor::fillFormWithNodeContent(const NodeT& _node)
{
  nameField()->setText(_node.name);
  typeField()->setCurrentIndex(_node.type);
  statusCalcRuleField()->setCurrentIndex(_node.sev_crule);
  statusPropRuleField()->setCurrentIndex(_node.sev_prule);
  iconField()->setCurrentIndex(iconField()->findText((_node.icon)));
  descriptionField()->setText(_node.description);
  alarmMsgField()->setText(_node.alarm_msg);
  notificationMsgField()->setText(_node.notification_msg);

  QString checkId = "";
  if(_node.type == NodeType::AlarmNode) {
    QListWidget* checks = checkListField();
    QStringList childNodes = _node.child_nodes.split(ngrt4n::CHILD_SEP.c_str());
    QStringList::iterator childNodeIt = childNodes.begin();
    if (childNodeIt != childNodes.end()) {
      checkId = (*childNodeIt).trimmed();
      CheckItemList checkItems = checks->findItems(checkId, Qt::MatchExactly);
      CheckItemList::const_iterator _it = checkItems.begin();
      if(_it == checkItems.end()) {
        checkField()->addItem(checkId);
        QListWidgetItem* item = new QListWidgetItem(checkId);
        checks->addItem(item);
        checks->setItemSelected(item, true);
      } else {
        checks->setItemSelected(*_it, true);
      }
    }
    childNodes.clear();
  }
  checkField()->setCurrentIndex(checkField()->findText(checkId, Qt::MatchExactly));
}

void ServiceEditor::layoutEditorComponents(void)
{
  mlayoutRowIndex = 0;
  loadLabelFields(); ++mlayoutRowIndex;
  loadTypeFields(); ++mlayoutRowIndex;
  loadStatusHandlingFields(); ++mlayoutRowIndex;
  loadIconFields(); ++mlayoutRowIndex;
  loadDescriptionFields(); ++mlayoutRowIndex;
  loadAlarmMsgFields(); ++mlayoutRowIndex;
  loadNotificationMsgFields(); ++mlayoutRowIndex;
  loadCheckField(); ++mlayoutRowIndex;
  loadButtonBox(); ++mlayoutRowIndex;
  setEnableFields(false);
}


void ServiceEditor::loadLabelFields()
{
  nameField()->setMaxLength( MAX_NODE_NAME );
  mlayout->addWidget(mitems["nameLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(nameField(),mlayoutRowIndex,1,1,2);
}


void ServiceEditor::loadDescriptionFields()
{
  mlayout->addWidget(mitems["descriptionLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(descriptionField(),mlayoutRowIndex,1,1,2);
}


void ServiceEditor::loadTypeFields()
{
  typeField()->addItem( NodeType::toString(NodeType::ServiceNode) );
  typeField()->addItem( NodeType::toString(NodeType::AlarmNode) );
  mlayout->addWidget(mitems["typeLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(typeField(),mlayoutRowIndex,1,1,2);
}

void ServiceEditor::loadStatusHandlingFields(void)
{
  StringMapT crules = DashboardBase::calcRules();
  QString defaultRule = CalcRules::label(CalcRules::HighCriticity);
  statusCalcRuleField()->addItem(tr("Calculation rule (Default is %1)").arg(defaultRule), CalcRules::HighCriticity);

  Q_FOREACH(const QString& rule, crules.keys()) {
    statusCalcRuleField()->addItem(rule, crules.value(rule));
  }
  StringMapT prules = DashboardBase::propRules();
  defaultRule = PropRules::label(PropRules::Unchanged);
  statusPropRuleField()->addItem(tr("Propagation rule (Default is %1)").arg(defaultRule), PropRules::Unchanged);
  Q_FOREACH(const QString& rule, prules.keys()) {
    statusPropRuleField()->addItem(rule, prules.value(rule));
  }
  mlayout->addWidget(mitems["priorityLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(statusCalcRuleField(),mlayoutRowIndex,1);
  mlayout->addWidget(statusPropRuleField(),mlayoutRowIndex,2);
}

void ServiceEditor::loadAlarmMsgFields()
{
  mlayout->addWidget(mitems["alarmMsgLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(alarmMsgField(),mlayoutRowIndex,1,1,2);
}


void ServiceEditor::loadNotificationMsgFields()
{
  mlayout->addWidget(mitems["notificationMsgLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(notificationMsgField(),mlayoutRowIndex,1,1,2);
}


void ServiceEditor::loadIconFields()
{
  IconMapT icons = ngrt4n::nodeIcons();
  QString header = "-->Select a icon (Default is " + ngrt4n::DEFAULT_ICON + ")";
  iconField()->addItem(header, icons.value(ngrt4n::DEFAULT_ICON));
  Q_FOREACH(const QString& label, icons.keys()) {
    QString path = icons.value(label);
    iconField()->addItem(QIcon(path), label, icons.value(path));
  }
  mlayout->addWidget(mitems["iconNameLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(iconField(),mlayoutRowIndex, 1, 1, 2);
}


void ServiceEditor::loadCheckField(void)
{
  QLabel* help = new QLabel();
  help->setPixmap(QPixmap(":images/built-in/help.png"));
  QHBoxLayout* llayout =  new QHBoxLayout();
  llayout->addWidget(mitems["lowLevelAlarmsLabel"]);
  llayout->addWidget(help);
  mlayout->addLayout(llayout, mlayoutRowIndex, 0, 2, 1);
  mlayout->addWidget(checkField(), mlayoutRowIndex, 1, 1, 2);
  checkField()->setEditable(true);
  help->setToolTip(tr("This depends on your monitoring configuration:"
                      "\n * For Nagios this follows the patterns 'host_name/service_name'"
                      "\n    E.g. mysql-server.example.com/Current Load."
                      "\n    From RealOpInsght 3.0 and higher, both the host part and the service are required."
                      "\n * For Zabbix it follows the pattern 'host_name/trigger_name'"
                      "\n    E.g. Zabbix server/Zabbix http poller processes more than 75% busy"
                      "\n    From RealOpInsght 3.0 and higher, both the host part and the service are required."
                      "\n * For Zenoss it follows the patterns 'device_name/component_name'"
                      "\n    E.g. localhost/httpd, localhost"
                      "\n    From RealOpInsght 3.0 and higher, both the host part and the service are required."
                      "\nSee the online documentation for further details."
                      ));
}

void ServiceEditor::loadButtonBox(void)
{
  mlayout->addWidget(buttonBox, mlayoutRowIndex, 2);
}

void ServiceEditor::handleNodeTypeChanged( const QString& _text)
{
  if(_text == NodeType::toString(NodeType::AlarmNode)) {
    setEnableFields(true);
  } else {
    setEnableFields(false);
    checkListField()->clearSelection();
    checkField()->setCurrentIndex(0);
  }
}

void ServiceEditor::handleNodeTypeActivated( const QString& _text)
{
  if(_text == NodeType::toString(NodeType::AlarmNode)) {
    Q_EMIT nodeTypeActivated( NodeType::AlarmNode );
  } else {
    Q_EMIT nodeTypeActivated( NodeType::ServiceNode );
  }
}


void ServiceEditor::addEvent(void)
{
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(handleSaveClick()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(handleCloseClick()));
  connect(nameField(), SIGNAL(returnPressed ()), this, SLOT(handleReturnPressed() ) );
  connect(typeField(), SIGNAL(currentIndexChanged(const QString&)), this, SLOT(handleNodeTypeChanged( const QString& ) ) );
  connect(typeField(), SIGNAL(activated(const QString&)), this, SLOT(handleNodeTypeActivated( const QString& ) ) );
}
