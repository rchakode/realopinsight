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
    m_mainLayout(new QGridLayout(this)),
    m_actionButtonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close))
{
  m_fieldWidgets["nameLabel"] = new QLabel(tr("Name"), this);
  m_fieldWidgets[NAME_FIELD] = new QLineEdit(this);

  m_fieldWidgets["typeLabel"] = new QLabel(tr("Type"), this);
  m_fieldWidgets[TYPE_FIELD] = new QComboBox(this);

  m_fieldWidgets["iconNameLabel"] = new QLabel(tr("Icon"), this);
  m_fieldWidgets[ICON_FIELD] = new QComboBox(this);

  m_fieldWidgets["priorityLabel"] = new QLabel(tr("Severity Handling Rules"), this);
  m_fieldWidgets[STATUS_CALC_RULE_FIELD] = new QComboBox(this);
  m_fieldWidgets[STATUS_PROP_RULE_FIELD] = new QComboBox(this);

  m_fieldWidgets["descriptionLabel"] = new QLabel(tr("Description"), this);
  m_fieldWidgets[DESCRIPTION_FIELD] = new QTextEdit(this);

  m_fieldWidgets["alarmMsgLabel"] = new QLabel(tr("Alarm Message"), this);
  m_fieldWidgets[ALARM_MSG_FIELD]  = new QTextEdit();

  m_fieldWidgets["notificationMsgLabel"] = new QLabel(tr("Notification Message"), this);
  m_fieldWidgets[NOTIFICATION_MSG_FIELD] = new QTextEdit(this);

  m_fieldWidgets["lowLevelAlarmsLabel"] = new QLabel(tr("Data Point"), this);
  m_fieldWidgets[CHECK_FIELD] = new QListWidget();

  m_checkFieldsGroup = new QGroupBox(this);

  m_dataPointSearchField = new QLineEdit(this);
  m_dataPointSearchField->setPlaceholderText(tr("Search..."));
  m_hostGroupFilterBox = new QComboBox(this);
  m_hostGroupFilterBox->addItem(tr("Select host group"));

  m_searchDataPointButton = new QPushButton(QIcon(":images/built-in/search_32x28.png"), "", this);
  m_addDataPointButton = new QPushButton(QIcon(":images/built-in/add_32x32.png"), "", this);
  m_dataPointActionButtons = new QStackedWidget(this);
  m_dataPointActionButtons->addWidget(m_searchDataPointButton);
  m_dataPointActionButtons->addWidget(m_addDataPointButton);

  layoutEditorComponents();

  addEvent();
}



ServiceEditor::~ServiceEditor()
{
  QMap<QString, QWidget*>::iterator iter = m_fieldWidgets.begin();

  while (iter != m_fieldWidgets.end()) {
    QString key = iter.key();
    QWidget* widget = m_fieldWidgets[key];
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
    m_fieldWidgets.remove(key);

    iter = m_fieldWidgets.begin();
  }
  m_fieldWidgets.clear();

  delete m_checkFieldsGroup;
  delete m_hostGroupFilterBox;
  delete m_dataPointSearchField;
  delete m_actionButtonBox;
  delete m_mainLayout;
}


void ServiceEditor::addEvent(void)
{
  connect(m_dataPointSearchField, SIGNAL(returnPressed()), this, SLOT(handleDataPointFieldReturnPressed()));
  connect(m_dataPointSearchField, SIGNAL(textEdited(const QString&)), this, SLOT(handleDataPointFilter(const QString&)));
  connect(m_searchDataPointButton, SIGNAL(clicked()), this, SLOT(handleDataPointSearch()));
  connect(m_addDataPointButton, SIGNAL(clicked()), this, SLOT(handleAddDataPointEntry()));
  connect(m_actionButtonBox, SIGNAL(accepted()), this, SLOT(handleSaveClick()));
  connect(m_actionButtonBox, SIGNAL(rejected()), this, SLOT(handleCloseClick()));
  connect(nameField(), SIGNAL(returnPressed ()), this, SLOT(handleReturnPressed() ) );
  connect(typeField(), SIGNAL(currentIndexChanged(const QString&)), this, SLOT(handleNodeTypeChanged( const QString& ) ) );
  connect(typeField(), SIGNAL(activated(const QString&)), this, SLOT(handleNodeTypeActivated( const QString& ) ) );
}


void ServiceEditor::layoutEditorComponents(void)
{
  m_currentRow = 0;
  layoutLabelFields();
  layoutTypeFields();
  layoutStatusHandlingFields();
  layoutIconFields();
  layoutDescriptionFields();
  layoutAlarmMsgFields();
  layoutNotificationMsgFields();
  layoutCheckField();
  layoutButtonBox();
  setEnableFields(false);
}

void ServiceEditor::updateDataPoints(const ChecksT& checks, const QString& srcId)
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
  m_fieldWidgets[ALARM_MSG_FIELD]->setEnabled(enable);
  m_fieldWidgets[NOTIFICATION_MSG_FIELD]->setEnabled(enable);
}

bool ServiceEditor::updateNodeInfo(NodeT& _node)
{
  _node.name             = nameField()->text();
  _node.type             = typeField()->currentIndex();
  _node.sev_crule        = statusCalcRuleField()->currentIndex();
  _node.sev_prule        = statusPropRuleField()->currentIndex();
  _node.icon             = iconField()->currentText();
  _node.description      = descriptionField()->toPlainText();
  _node.alarm_msg        = alarmMsgField()->toPlainText();
  _node.notification_msg = notificationMsgField()->toPlainText();
  if(_node.type == NodeType::AlarmNode) {
    QList<QListWidgetItem*> selectedItems = checkField()->selectedItems();
    if (! selectedItems.isEmpty())
      _node.child_nodes = selectedItems.at(0)->text();
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

  if (_node.type == NodeType::AlarmNode) {
    QStringList childNodes = _node.child_nodes.split(ngrt4n::CHILD_SEP.c_str());
    QStringList::iterator childNodeIt = childNodes.begin();
    if (childNodeIt != childNodes.end()) {
      QString checkId = (*childNodeIt).trimmed();
      QList<QListWidgetItem*> matchs = checkField()->findItems(checkId, Qt::MatchExactly);
      if (! matchs.isEmpty()) {
        checkField()->setCurrentItem(matchs.at(0));
      } else {
        addAndSelectDataPointEntry(checkId);
      }
    }
  }
}

void ServiceEditor::layoutLabelFields()
{
  ++m_currentRow;
  nameField()->setMaxLength( MAX_NODE_NAME );
  m_mainLayout->addWidget(m_fieldWidgets["nameLabel"], m_currentRow, 0);
  m_mainLayout->addWidget(nameField(),m_currentRow,1,1,2);
}


void ServiceEditor::layoutDescriptionFields()
{
  ++m_currentRow;
  m_mainLayout->addWidget(m_fieldWidgets["descriptionLabel"], m_currentRow, 0);
  m_mainLayout->addWidget(descriptionField(),m_currentRow,1,1,2);
}


void ServiceEditor::layoutTypeFields()
{
  typeField()->addItem( NodeType::toString(NodeType::ServiceNode) );
  typeField()->addItem( NodeType::toString(NodeType::AlarmNode) );
  m_mainLayout->addWidget(m_fieldWidgets["typeLabel"], m_currentRow, 0);
  m_mainLayout->addWidget(typeField(),m_currentRow,1,1,2);
}

void ServiceEditor::layoutStatusHandlingFields(void)
{
  ++m_currentRow;
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
  m_mainLayout->addWidget(m_fieldWidgets["priorityLabel"], m_currentRow, 0);
  m_mainLayout->addWidget(statusCalcRuleField(),m_currentRow,1);
  m_mainLayout->addWidget(statusPropRuleField(),m_currentRow,2);
}

void ServiceEditor::layoutAlarmMsgFields()
{
  ++m_currentRow;
  m_mainLayout->addWidget(m_fieldWidgets["alarmMsgLabel"], m_currentRow, 0);
  m_mainLayout->addWidget(alarmMsgField(),m_currentRow,1,1,2);
}


void ServiceEditor::layoutNotificationMsgFields()
{
  ++m_currentRow;
  m_mainLayout->addWidget(m_fieldWidgets["notificationMsgLabel"], m_currentRow, 0);
  m_mainLayout->addWidget(notificationMsgField(),m_currentRow,1,1,2);
}


void ServiceEditor::layoutIconFields()
{
  ++m_currentRow;
  IconMapT icons = ngrt4n::nodeIcons();
  QString header = "-->Select a icon (Default is " + ngrt4n::DEFAULT_ICON + ")";
  iconField()->addItem(header, icons.value(ngrt4n::DEFAULT_ICON));
  Q_FOREACH(const QString& label, icons.keys()) {
    QString path = icons.value(label);
    iconField()->addItem(QIcon(path), label, icons.value(path));
  }
  m_mainLayout->addWidget(m_fieldWidgets["iconNameLabel"], m_currentRow, 0);
  m_mainLayout->addWidget(iconField(), m_currentRow, 1, 1, 2);
}


void ServiceEditor::layoutCheckField(void)
{
  ++m_currentRow;
  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->addWidget(m_fieldWidgets["lowLevelAlarmsLabel"]);
  labelLayout->addWidget(createCheckFieldHelpIcon());

  QGridLayout* fieldsLayout = new QGridLayout();
  fieldsLayout->addWidget(m_hostGroupFilterBox, 0, 0, 1, 1);
  fieldsLayout->addWidget(m_dataPointSearchField, 0, 1, 1, 1);
  fieldsLayout->addWidget(m_dataPointActionButtons, 0, 2, 1, 1);
  m_dataPointActionButtons->setMaximumSize(QSize(24, 24));
  fieldsLayout->addWidget(checkField(), 1, 0, 1, 3);

  m_checkFieldsGroup->setLayout(fieldsLayout);
  m_mainLayout->addLayout(labelLayout, m_currentRow, 0, 1, 1);
  m_mainLayout->addWidget(m_checkFieldsGroup, m_currentRow, 1, 1, 3);

  setCheckFieldsStyle();
  m_hostGroupFilterBox->setEnabled(true);
}

void ServiceEditor::layoutButtonBox(void)
{
  ++m_currentRow;
  m_mainLayout->addWidget(m_actionButtonBox, m_currentRow, 2);
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


void ServiceEditor::handleDataPointFilter(const QString& text)
{
  checkField()->clear();
  checkField()->addItems(m_dataPoints.filter(text));
  if (checkField()->count() == 0)
    m_dataPointActionButtons->setCurrentWidget(m_addDataPointButton);
}

void ServiceEditor::addAndSelectDataPointEntry(const QString& text)
{
  m_dataPoints.append(text);
  checkField()->addItem(text);
  QList<QListWidgetItem*> matchs = checkField()->findItems(text, Qt::MatchExactly);

  if (! matchs.isEmpty())
    checkField()->setCurrentItem(matchs.at(0));

  m_dataPointActionButtons->setCurrentWidget(m_searchDataPointButton);
}


void ServiceEditor::setCheckFieldsStyle(void)
{
  checkField()->setStyleSheet("border: none; background: white;");
  m_dataPointSearchField->setStyleSheet("border: none; background: white;");
  m_hostGroupFilterBox->setStyleSheet("border: none; background: white;");
  m_addDataPointButton->setStyleSheet("border: none;");
  m_searchDataPointButton->setStyleSheet("border: none;");
}



void ServiceEditor::handleDataPointFieldReturnPressed(void)
{
  if (m_dataPointActionButtons->currentWidget() == m_searchDataPointButton) {
    handleDataPointSearch();
  } else {
    handleAddDataPointEntry();
  }
}
