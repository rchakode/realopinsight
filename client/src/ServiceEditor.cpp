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
namespace {
  const QString UNCLASSIFIED_HOST_GROUP = QObject::tr("Unclassified Hosts");
  const QString ALL_HOST_GROUPS = QObject::tr("All Hosts");
  }

ServiceEditor::ServiceEditor(QWidget* _parent )
  : QWidget(_parent),
    m_mainLayout(new QGridLayout(this)),
    m_actionButtonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close))
{
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
  connect(nameField(), SIGNAL(returnPressed ()), this, SLOT(handleReturnPressed() ) );
  connect(typeField(), SIGNAL(currentIndexChanged(const QString&)), this, SLOT(handleNodeTypeChanged( const QString& ) ) );
  connect(typeField(), SIGNAL(activated(const QString&)), this, SLOT(handleNodeTypeActivated( const QString& ) ) );
  connect(m_dataPointSearchField, SIGNAL(returnPressed()), this, SLOT(handleDataPointFieldReturnPressed()));
  connect(m_dataPointSearchField, SIGNAL(textEdited(const QString&)), this, SLOT(handleDataPointFilter(const QString&)));
  connect(m_searchDataPointButton, SIGNAL(clicked()), this, SLOT(handleDataPointSearch()));
  connect(m_addDataPointButton, SIGNAL(clicked()), this, SLOT(handleAddDataPointEntry()));
  connect(m_hostGroupFilterBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(handleUpdateDataPointsList()));
  connect(m_actionButtonBox, SIGNAL(accepted()), this, SLOT(handleSaveClick()));
  connect(m_actionButtonBox, SIGNAL(rejected()), this, SLOT(handleCloseClick()));
}


void ServiceEditor::layoutEditorComponents(void)
{
  m_currentRow = 0;
  layoutLabelFields();
  layoutTypeFields();
  layoutStatusCalcFields();
  layoutStatusPropFields();
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
  m_hostGroupFilterBox->clear();

  m_hostGroupFilterBox->addItem(ALL_HOST_GROUPS);

  for(ChecksT::const_iterator it=checks.begin(), end=checks.end(); it!=end; ++it) {
    QString entry = QString("%1:%2").arg(srcId, (it->second).id.c_str());
    QStringList groups = QString::fromStdString((it->second).host_groups).split(ngrt4n::CHILD_SEP.c_str());
    if (groups.isEmpty()) {
      m_dataPoints[UNCLASSIFIED_HOST_GROUP].push_back(entry);
      m_hostGroupFilterBox->addItem(UNCLASSIFIED_HOST_GROUP);
    } else {
      Q_FOREACH(const QString& group, groups) {
        m_dataPoints[group].push_back(entry);
        if (m_hostGroupFilterBox->findText(group) == -1)
          m_hostGroupFilterBox->addItem(group);
      }
    }
  }

  handleUpdateDataPointsList();
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
  statusCalcRuleField()->setCurrentText(CalcRules(_node.sev_crule).toString());
  statusPropRuleField()->setCurrentText(PropRules(_node.sev_prule).toString());
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
  QLineEdit* inputWidget = new QLineEdit(this);
  inputWidget->setMaxLength( MAX_NODE_NAME );
  m_fieldWidgets[NAME_FIELD] = inputWidget;

  ++m_currentRow;
  m_mainLayout->addWidget(new QLabel(tr("Name"), this), m_currentRow, 0);
  m_mainLayout->addWidget(nameField(),m_currentRow, 1, 1, 2);
}


void ServiceEditor::layoutDescriptionFields()
{
  m_fieldWidgets[DESCRIPTION_FIELD] = new QTextEdit(this);

  ++m_currentRow;
  m_mainLayout->addWidget(new QLabel(tr("Description"), this), m_currentRow, 0);
  m_mainLayout->addWidget(descriptionField(),m_currentRow, 1, 1, 2);
}


void ServiceEditor::layoutTypeFields()
{
  QComboBox* inputWidget = new QComboBox(this);
  inputWidget->addItem( NodeType::toString(NodeType::ServiceNode) );
  inputWidget->addItem( NodeType::toString(NodeType::AlarmNode) );
  m_fieldWidgets[TYPE_FIELD] = inputWidget;

  ++m_currentRow;
  m_mainLayout->addWidget(new QLabel(tr("Type"), this), m_currentRow, 0);
  m_mainLayout->addWidget(typeField(), m_currentRow, 1, 1, 2);
}

void ServiceEditor::layoutStatusCalcFields(void)
{
  m_fieldWidgets[STATUS_CALC_RULE_FIELD] = new QComboBox(this);
  statusCalcRuleField()->setInsertPolicy(QComboBox::InsertAtBottom);

  StringMapT rules = DashboardBase::calcRules();
  Q_FOREACH(const QString& rule, rules.keys()) {
    statusCalcRuleField()->addItem(rule, rules.value(rule));
  }

  QComboBox* thresholdInSeverityBox = new QComboBox(this);
  QComboBox* thresholdOutSeverityBox = new QComboBox(this);
  QComboBox* thresholdRulesBox = new QComboBox(this);
  thresholdRulesBox->addItem(tr("Select rule to delete"));

  QMap<qint8, Severity> allSeverities;
  allSeverities.insert(ngrt4n::Normal, Severity(ngrt4n::Normal));
  allSeverities.insert(ngrt4n::Minor, Severity(ngrt4n::Minor));
  allSeverities.insert(ngrt4n::Major, Severity(ngrt4n::Major));
  allSeverities.insert(ngrt4n::Critical, Severity(ngrt4n::Critical));
  allSeverities.insert(ngrt4n::Unknown, Severity(ngrt4n::Unknown));

  Q_FOREACH(const Severity& sev, allSeverities) {
    thresholdInSeverityBox->addItem(sev.toString(), sev.valueString());
    thresholdOutSeverityBox->addItem(sev.toString(), sev.valueString());
  }
  QHBoxLayout* detailsFieldLayout = new QHBoxLayout();
  detailsFieldLayout->addWidget(new QLabel(tr("If threshold of"), this), 0);
  detailsFieldLayout->addWidget(m_failureWeightBox = new WeightBox(WeightThreshold, this));
  detailsFieldLayout->addWidget(thresholdInSeverityBox, 5);
  detailsFieldLayout->addWidget(new QLabel(tr("set to"), this), 1);
  detailsFieldLayout->addWidget(thresholdOutSeverityBox,  5);
  detailsFieldLayout->addWidget(new IconButton(":images/built-in/document-add_32x32.png", this), 1);
  detailsFieldLayout->addWidget(thresholdRulesBox, 5);
  detailsFieldLayout->addWidget(new IconButton(":images/built-in/document-remove_32x32.png", this), 1);

  ++m_currentRow;
  m_mainLayout->addWidget(new QLabel(tr("Severiry Calculation Rule"), this), m_currentRow, 0);
  m_mainLayout->addWidget(statusCalcRuleField(), m_currentRow, 1);
  m_mainLayout->addLayout(detailsFieldLayout, m_currentRow, 2);
}

void ServiceEditor::layoutStatusPropFields(void)
{
  m_fieldWidgets[STATUS_PROP_RULE_FIELD] = new QComboBox(this);
  statusPropRuleField()->setInsertPolicy(QComboBox::InsertAtBottom);
  StringMapT rules = DashboardBase::propRules();
  Q_FOREACH(const QString& rule, rules.keys()) {
    statusPropRuleField()->addItem(rule, rules.value(rule));
  }

  QHBoxLayout* weightFieldLayout = new QHBoxLayout();
  weightFieldLayout->addWidget(new QLabel(tr("Weight Factor"), this), 1);
  weightFieldLayout->addWidget(m_weightBox = new WeightBox(WeightNormalized, this), 10, Qt::AlignLeft);

  ++m_currentRow;
  m_mainLayout->addWidget(new QLabel(tr("Severity Propagation Rule"), this), m_currentRow, 0);
  m_mainLayout->addWidget(statusPropRuleField(), m_currentRow, 1);
  m_mainLayout->addLayout(weightFieldLayout, m_currentRow, 2, Qt::AlignLeft);
}

void ServiceEditor::layoutAlarmMsgFields()
{
  m_fieldWidgets[ALARM_MSG_FIELD] = new QTextEdit();

  ++m_currentRow;
  m_mainLayout->addWidget(new QLabel(tr("Alarm Message"), this), m_currentRow, 0);
  m_mainLayout->addWidget(alarmMsgField(),m_currentRow, 1, 1, 2);
}


void ServiceEditor::layoutNotificationMsgFields()
{
  m_fieldWidgets[NOTIFICATION_MSG_FIELD] = new QTextEdit(this);
  ++m_currentRow;
  m_mainLayout->addWidget(new QLabel(tr("Notification Message"), this), m_currentRow, 0);
  m_mainLayout->addWidget(notificationMsgField(), m_currentRow, 1, 1, 2);
}


void ServiceEditor::layoutIconFields()
{
  m_fieldWidgets[ICON_FIELD] = new QComboBox(this);

  IconMapT icons = ngrt4n::nodeIcons();
  QString header = QString("-->Select a icon (Default is %1)").arg(ngrt4n::DEFAULT_ICON);
  iconField()->addItem(header, icons.value(ngrt4n::DEFAULT_ICON));
  Q_FOREACH(const QString& label, icons.keys()) {
    QString path = icons.value(label);
    iconField()->addItem(QIcon(path), label, icons.value(path));
  }

  ++m_currentRow;
  m_mainLayout->addWidget(new QLabel(tr("Icon"), this), m_currentRow, 0);
  m_mainLayout->addWidget(iconField(), m_currentRow, 1, 1, 2);
}


void ServiceEditor::layoutCheckField(void)
{
  m_fieldWidgets[CHECK_FIELD] = new QListWidget();

  m_checkFieldsGroup = new QGroupBox(this);

  m_dataPointSearchField = new QLineEdit(this);
  m_dataPointSearchField->setPlaceholderText(tr("Set or search data point..."));
  m_hostGroupFilterBox = new QComboBox(this);
  m_hostGroupFilterBox->addItem(tr("Select host group"));

  m_searchDataPointButton = new IconButton(":images/built-in/search_32x28.png", this);
  m_addDataPointButton = new IconButton(":images/built-in/document-add_32x32.png", this);
  m_dataPointActionButtons = new QStackedWidget(this);
  m_dataPointActionButtons->addWidget(m_searchDataPointButton);
  m_dataPointActionButtons->addWidget(m_addDataPointButton);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->addWidget(new QLabel(tr("Data Point"), this));
  labelLayout->addWidget(createCheckFieldHelpIcon());

  QGridLayout* fieldsLayout = new QGridLayout();
  fieldsLayout->addWidget(m_hostGroupFilterBox, 0, 0, 1, 1);
  fieldsLayout->addWidget(m_dataPointSearchField, 0, 1, 1, 1);
  fieldsLayout->addWidget(m_dataPointActionButtons, 0, 2, 1, 1);
  m_dataPointActionButtons->setMaximumSize(QSize(24, 24));
  fieldsLayout->addWidget(checkField(), 1, 0, 1, 3);

  m_checkFieldsGroup->setLayout(fieldsLayout);

  ++m_currentRow;
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
  QString selectedGroup = m_hostGroupFilterBox->currentText();
  if (selectedGroup != ALL_HOST_GROUPS) {
    checkField()->addItems(m_dataPoints[selectedGroup].filter(text));
  } else {
    Q_FOREACH(const QStringList& entries, m_dataPoints) {
      checkField()->addItems(entries.filter(text));
    }
  }
  if (checkField()->count() > 0)
    m_dataPointActionButtons->setCurrentWidget(m_searchDataPointButton);
  else
    m_dataPointActionButtons->setCurrentWidget(m_addDataPointButton);
}

void ServiceEditor::addAndSelectDataPointEntry(const QString& text)
{
  m_dataPoints[UNCLASSIFIED_HOST_GROUP].append(text);
  if (m_hostGroupFilterBox->findText(UNCLASSIFIED_HOST_GROUP) == -1)
    m_hostGroupFilterBox->addItem(UNCLASSIFIED_HOST_GROUP);
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


void ServiceEditor::handleUpdateDataPointsList(void)
{
  checkField()->clear();
  QString selectedGroup = m_hostGroupFilterBox->currentText();
  if (selectedGroup == ALL_HOST_GROUPS) {
    Q_FOREACH(const QStringList& entries, m_dataPoints) {
      checkField()->addItems(entries);
    }
  } else {
    checkField()->addItems(m_dataPoints[ selectedGroup ]);
  }
  // m_dataPoints.sort(Qt::CaseInsensitive);
}
