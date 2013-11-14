/*
 * ServiceEditor.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
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
#include "utilsClient.hpp"


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

void ServiceEditor::loadStatusFile(const QString& path)
{
  ChecksT checks;
  MonitorBroker::loadNagiosCollectedData(path.toStdString(), checks);
  setCheckListField( checks );
}

void ServiceEditor::setCheckListField(const ChecksT& _nagios_checks)
{
  checkField()->clear();
  for(ChecksT::const_iterator it=_nagios_checks.begin(), end=_nagios_checks.end();
      it!=end; ++it) { checkField()->addItem(QString::fromStdString(it->second.id)); }
}

void ServiceEditor::setEnableFields(const bool& _enable)
{
  mitems[CHECK_FIELD]->setEnabled(_enable);
  mitems[CHECK_LIST_FIELD]->setEnabled(_enable);
  mitems[ALARM_MSG_FIELD]->setEnabled(_enable);
  mitems[NOTIFICATION_MSG_FIELD]->setEnabled(_enable);
}

bool ServiceEditor::updateNode(NodeListT& _node_map, const QString& _node_id)
{
  NodeListT::iterator node = static_cast<const NodeListT::iterator>(_node_map.find(_node_id));
  if( node != _node_map.end()) {
    node->name = nameField()->text();
    node->type = typeField()->currentIndex();
    node->sev_crule = statusCalcRuleField()->currentIndex();
    node->sev_prule = statusPropRuleField()->currentIndex();
    node->icon = iconField()->currentText();
    node->description = descriptionField()->toPlainText();
    node->alarm_msg  = alarmMsgField()->toPlainText();
    node->notification_msg = notificationMsgField()->toPlainText();
    if(node->type == NodeType::ALARM_NODE)
      node->child_nodes = checkField()->currentText();
    return true;
  }
  return false;
}


bool ServiceEditor::updateNode(NodeListT::iterator& _node)
{
  _node->name = nameField()->text();
  _node->type = typeField()->currentIndex();
  _node->sev_crule = statusCalcRuleField()->currentIndex();
  _node->sev_prule = statusPropRuleField()->currentIndex();
  _node->icon = iconField()->currentText();
  _node->description = descriptionField()->toPlainText();
  _node->alarm_msg  = alarmMsgField()->toPlainText();
  _node->notification_msg = notificationMsgField()->toPlainText();
  if(_node->type == NodeType::ALARM_NODE)
    _node->child_nodes = checkField()->currentText();

  return true;
}

void ServiceEditor::setContent(const NodeListT& _node_map, const QString& _nodeId)
{
  NodeListT::const_iterator node = _node_map.find(_nodeId);
  if( node != _node_map.end())
    setContent(node);
}



void ServiceEditor::setContent(const NodeT& _node)
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
  if(_node.type == NodeType::ALARM_NODE) {
    QListWidget* checks = checkListField();
    QStringList childNodes = _node.child_nodes.split(Parser::CHILD_SEP);
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
  typeField()->addItem( NodeType::toString(NodeType::SERVICE_NODE) );
  typeField()->addItem( NodeType::toString(NodeType::ALARM_NODE) );
  mlayout->addWidget(mitems["typeLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(typeField(),mlayoutRowIndex,1,1,2);
}

void ServiceEditor::loadStatusHandlingFields(void)
{
  StringMapT crules = DashboardBase::calcRules();
  QString defaultRule = CalcRules::label(CalcRules::HighCriticity);
  statusCalcRuleField()->addItem(tr("Calculation rule (Default is %1)").arg(defaultRule), CalcRules::HighCriticity);

  foreach(const QString& rule, crules.keys()) {
    statusCalcRuleField()->addItem(rule, crules.value(rule));
  }
  StringMapT prules = DashboardBase::propRules();
  defaultRule = PropRules::label(PropRules::Unchanged);
  statusPropRuleField()->addItem(tr("Propagation rule (Default is %1)").arg(defaultRule), PropRules::Unchanged);
  foreach(const QString& rule, prules.keys()) {
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
  IconMapT icons = utils::nodeIcons();
  QString header = "-->Select a icon (Default is " + utils::DEFAULT_ICON + ")";
  iconField()->addItem(header, icons.value(utils::DEFAULT_ICON));
  foreach(const QString& label, icons.keys()) {
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
                      "\n * For Nagios this follows the patterns 'host_name/service_name' or just 'host_name'"
                      "\n    E.g. mysql-server.example.com/Current Load, mysql-server.example.com"
                      "\n    When only host_name is set, the data point is associated to ping status."
                      "\n * For Zabbix it follows the pattern 'host_name/trigger_name'"
                      "\n    E.g. Zabbix server/Zabbix http poller processes more than 75% busy"
                      "\n * For Zenoss it follows the patterns 'device_name/component_name' or just 'device_name'"
                      "\n    E.g. localhost/httpd, localhost"
                      "\n    When only device_name is set, the data point is associated to ping status."
                      "\nSee the online documentation for further details."
                      ));
}

void ServiceEditor::loadButtonBox(void)
{
  mlayout->addWidget(buttonBox, mlayoutRowIndex, 2);
}

void ServiceEditor::handleNodeTypeChanged( const QString& _text)
{
  if(_text == NodeType::toString(NodeType::ALARM_NODE)) {
    setEnableFields(true);
  } else {
    setEnableFields(false);
    checkListField()->clearSelection();
    checkField()->setCurrentIndex(0);
  }
}

void ServiceEditor::handleNodeTypeActivated( const QString& _text)
{
  if(_text == NodeType::toString(NodeType::ALARM_NODE)) {
    emit nodeTypeActivated( NodeType::ALARM_NODE );
  } else {
    emit nodeTypeActivated( NodeType::SERVICE_NODE );
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
