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
    mlayout(new QGridLayout(this)),
    buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close))
{
  mitems["nameLabel"] = new QLabel(tr("Name"), this);
  mitems[NAME_FIELD] = new QLineEdit(this);

  mitems["typeLabel"] = new QLabel(tr("Type"), this);
  mitems[TYPE_FIELD] = new QComboBox(this);

  mitems["iconNameLabel"] = new QLabel(tr("Icon"), this);
  mitems[ICON_FIELD] = new QComboBox(this);

  mitems["priorityLabel"] = new QLabel(tr("Severity Handling Rules"), this);
  mitems[STATUS_CALC_RULE_FIELD] = new QComboBox(this);
  mitems[STATUS_PROP_RULE_FIELD] = new QComboBox(this);

  mitems["descriptionLabel"] = new QLabel(tr("Description"), this);
  mitems[DESCRIPTION_FIELD] = new QTextEdit(this);

  mitems["alarmMsgLabel"] = new QLabel(tr("Alarm Message"), this);
  mitems[ALARM_MSG_FIELD]  = new QTextEdit();

  mitems["notificationMsgLabel"] = new QLabel(tr("Notification Message"), this);
  mitems[NOTIFICATION_MSG_FIELD] = new QTextEdit(this);

  mitems["lowLevelAlarmsLabel"] = new QLabel(tr("Data Point"), this);
  mitems[CHECK_FIELD] = new QListWidget();

  m_checkFieldsGroup = new QGroupBox(this);

  m_checkSearchFilterField = new QLineEdit(this);
  m_checkSearchFilterField->setPlaceholderText(tr("Search..."));
  m_hostGroupFilterBox = new QComboBox(this);
  m_hostGroupFilterBox->addItem(tr("Select host group"));

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

  delete m_checkFieldsGroup;
  delete m_hostGroupFilterBox;
  delete m_checkSearchFilterField;
  delete buttonBox;
  delete mlayout;

}

void ServiceEditor::layoutEditorComponents(void)
{
  mlayoutRowIndex = 0;
  loadLabelFields();
  loadTypeFields();
  loadStatusHandlingFields();
  loadIconFields();
  loadDescriptionFields();
  loadAlarmMsgFields();
  loadNotificationMsgFields();
  loadCheckField();
  loadButtonBox();
  setEnableFields(false);
}


void ServiceEditor::addEvent(void)
{

  connect(m_checkSearchFilterField, SIGNAL(textEdited(const QString&)), this, SLOT(handleCheckFilter(const QString&)));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(handleSaveClick()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(handleCloseClick()));
  connect(nameField(), SIGNAL(returnPressed ()), this, SLOT(handleReturnPressed() ) );
  connect(typeField(), SIGNAL(currentIndexChanged(const QString&)), this, SLOT(handleNodeTypeChanged( const QString& ) ) );
  connect(typeField(), SIGNAL(activated(const QString&)), this, SLOT(handleNodeTypeActivated( const QString& ) ) );
}


void ServiceEditor::loadChecks(const ChecksT& checks, const QString& srcId)
{
  m_dataPoints.clear();
  for(ChecksT::const_iterator it=checks.begin(), end=checks.end(); it!=end; ++it) {
    m_dataPoints.push_back(QString("%1:%2").arg(srcId, (it->second).id.c_str()));
  }
  if (! m_dataPoints.isEmpty()) {
    m_dataPoints.sort(Qt::CaseInsensitive);
  }
  checkField()->clear();
  checkField()->addItems(m_dataPoints);
}

void ServiceEditor::setEnableFields(const bool& enable)
{
  m_checkFieldsGroup->setEnabled(enable);
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
    if(node->type == NodeType::AlarmNode) {
      QList<QListWidgetItem*> selectedItems = checkField()->selectedItems();
      if (! selectedItems.isEmpty())
        node->child_nodes = selectedItems.at(0)->text();
    }
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
  if(_node->type == NodeType::AlarmNode) {
    //FIXME: factorization required
    QList<QListWidgetItem*> selectedItems = checkField()->selectedItems();
    if (! selectedItems.isEmpty())
      _node->child_nodes = selectedItems.at(0)->text();
  }

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
  if (_node.type == NodeType::AlarmNode) {
    QStringList childNodes = _node.child_nodes.split(ngrt4n::CHILD_SEP.c_str());
    QStringList::iterator childNodeIt = childNodes.begin();
    if (childNodeIt != childNodes.end()) {
      checkId = (*childNodeIt).trimmed();
    }
  }
  QList<QListWidgetItem*> matchs = checkField()->findItems(checkId, Qt::MatchExactly);
  if (matchs.isEmpty()) {
    m_dataPoints.append(checkId);
    checkField()->addItem(checkId);
    matchs = checkField()->findItems(checkId, Qt::MatchExactly);
  }

  if (! matchs.isEmpty())
    checkField()->setCurrentItem(matchs.at(0));
}

void ServiceEditor::loadLabelFields()
{
  ++mlayoutRowIndex;
  nameField()->setMaxLength( MAX_NODE_NAME );
  mlayout->addWidget(mitems["nameLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(nameField(),mlayoutRowIndex,1,1,2);
}


void ServiceEditor::loadDescriptionFields()
{
  ++mlayoutRowIndex;
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
  ++mlayoutRowIndex;
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
  ++mlayoutRowIndex;
  mlayout->addWidget(mitems["alarmMsgLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(alarmMsgField(),mlayoutRowIndex,1,1,2);
}


void ServiceEditor::loadNotificationMsgFields()
{
  ++mlayoutRowIndex;
  mlayout->addWidget(mitems["notificationMsgLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(notificationMsgField(),mlayoutRowIndex,1,1,2);
}


void ServiceEditor::loadIconFields()
{
  ++mlayoutRowIndex;
  IconMapT icons = ngrt4n::nodeIcons();
  QString header = "-->Select a icon (Default is " + ngrt4n::DEFAULT_ICON + ")";
  iconField()->addItem(header, icons.value(ngrt4n::DEFAULT_ICON));
  Q_FOREACH(const QString& label, icons.keys()) {
    QString path = icons.value(label);
    iconField()->addItem(QIcon(path), label, icons.value(path));
  }
  mlayout->addWidget(mitems["iconNameLabel"], mlayoutRowIndex, 0);
  mlayout->addWidget(iconField(), mlayoutRowIndex, 1, 1, 2);
}


void ServiceEditor::loadCheckField(void)
{
  ++mlayoutRowIndex;

  QHBoxLayout* labelLayout =  new QHBoxLayout();
  labelLayout->addWidget(mitems["lowLevelAlarmsLabel"]);
  labelLayout->addWidget(createCheckFieldHelpIcon());

  QGridLayout* fieldsLayout = new QGridLayout();
  fieldsLayout->addWidget(m_hostGroupFilterBox, 0, 0, 1, 1);
  fieldsLayout->addWidget(m_checkSearchFilterField, 0, 1, 1, 1);
  fieldsLayout->addWidget(checkField(), 1, 0, 1, 2, 0);

  m_checkFieldsGroup->setLayout(fieldsLayout);
  mlayout->addLayout(labelLayout, mlayoutRowIndex, 0, 1, 1);
  mlayout->addWidget(m_checkFieldsGroup, mlayoutRowIndex, 1, 1, 2);

  checkField()->setStyleSheet("border: none; background: white;");
  m_checkSearchFilterField->setStyleSheet("border: none; background: white;");
  m_hostGroupFilterBox->setStyleSheet("border: none; background: white;");

  m_hostGroupFilterBox->setEnabled(true);
}

void ServiceEditor::loadButtonBox(void)
{
  ++mlayoutRowIndex;
  mlayout->addWidget(buttonBox, mlayoutRowIndex, 2);
}

void ServiceEditor::handleNodeTypeChanged( const QString& _text)
{
  if(_text == NodeType::toString(NodeType::AlarmNode)) {
    setEnableFields(true);
  } else {
    setEnableFields(false);
    checkField()->setCurrentRow(0);
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


QLabel* ServiceEditor::createCheckFieldHelpIcon(void)
{
  QLabel* label = new QLabel();
  label ->setPixmap(QPixmap(":images/built-in/help.png"));
  label ->setToolTip(tr("This depends on your monitoring configuration:"
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
  return label;
}


void ServiceEditor::handleCheckFilter(const QString& text)
{
  checkField()->clear();
  checkField()->addItems(m_dataPoints.filter(text));
}
