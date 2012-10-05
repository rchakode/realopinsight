/*
 * ServiceEditor.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */


#include "ServiceEditor.hpp"
#include "Preferences.hpp"
#include "GraphView.hpp"
#include "SvNavigator.hpp"


ServiceEditor::ServiceEditor(QWidget* _parent )
    : QWidget( _parent ),
      settings( new Settings() ),
      layout (new QGridLayout( this ) ),
      buttonBox(  new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close))
{
    editorItemsList["nameLabel"] = new QLabel(tr("Name")) ;
    editorItemsList[NAME_FIELD] = new QLineEdit() ;

    editorItemsList["typeLabel"] = new QLabel(tr("Type")) ;
    editorItemsList[TYPE_FIELD] = new QComboBox() ;

    editorItemsList["iconNameLabel"] = new QLabel(tr("Icon"));
    editorItemsList[ICON_FIELD] = new QComboBox();

    editorItemsList["priorityLabel"] = new QLabel(tr("Status Handling")) ;
    editorItemsList[STATUS_CALC_RULE_FIELD] = new QComboBox() ;
    editorItemsList[STATUS_PROP_RULE_FIELD] = new QComboBox() ;

    editorItemsList["descriptionLabel"] = new QLabel(tr("Description")) ;
    editorItemsList[DESCRIPTION_FIELD] = new QTextEdit() ;

    editorItemsList["alarmMsgLabel"] = new QLabel(tr("Alarm Message")) ;
    editorItemsList[ALARM_MSG_FIELD]  = new QTextEdit() ;

    editorItemsList["notificationMsgLabel"] = new QLabel(tr("Notification Message")) ;
    editorItemsList[NOTIFICATION_MSG_FIELD] = new QTextEdit() ;

    editorItemsList["lowLevelAlarmsLabel"] = new QLabel(tr("Alarm (Check/Trigger)")) ;
    editorItemsList[CHECK_FIELD] = new QComboBox() ;
    editorItemsList[CHECK_LIST_FIELD] = new QListWidget() ;

    layoutEditorComponents();

    addEvent();
}

ServiceEditor::~ServiceEditor()
{
    QMap<QString, QWidget*>::iterator iter = editorItemsList.begin();

    while (iter != editorItemsList.end())
    {
        QString key = iter.key();
        QWidget* widget = editorItemsList[key];
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
        editorItemsList.remove(key);

        iter = editorItemsList.begin();
    }
    editorItemsList.clear();

    delete buttonBox;
    delete layout;

}

void ServiceEditor::loadStatusFile(const QString & path)
{
    MonitorBroker::NagiosChecksT checks ;
    MonitorBroker::loadNagiosCollectedData(path.toStdString(), checks);
    setCheckListField( checks );
}

void ServiceEditor::setCheckListField(const MonitorBroker::NagiosChecksT& _nagios_checks)
{
    checkField()->clear() ;
    for(MonitorBroker::NagiosChecksT::const_iterator it = _nagios_checks.begin(); it != _nagios_checks.end(); it++) {
        checkField()->addItem(QString::fromStdString(it->second.id)) ;
    }
}

void ServiceEditor::setEnableFields(const bool& _enable)
{
    editorItemsList[CHECK_FIELD]->setEnabled(_enable);
    editorItemsList[CHECK_LIST_FIELD]->setEnabled(_enable);
    editorItemsList[ALARM_MSG_FIELD]->setEnabled(_enable);
    editorItemsList[NOTIFICATION_MSG_FIELD]->setEnabled(_enable);
}

bool ServiceEditor::updateNode(NodeListT & _node_map, const QString& _node_id)
{
    NodeListT::iterator node = static_cast<const NodeListT::iterator>(_node_map.find(_node_id));
    if( node != _node_map.end()) {
        node->name = nameField()->text() ;
        node->type = typeField()->currentIndex();
        node->status_crule = statusCalcRuleField()->currentIndex();
        node->status_prule = statusPropRuleField()->currentIndex();
        node->icon = iconField()->currentText();
        node->description = descriptionField()->toPlainText();
        node->alarm_msg  = alarmMsgField()->toPlainText();
        node->notification_msg = notificationMsgField()->toPlainText();
        if( node->type == NodeType::ALARM_NODE ) node->child_nodes = checkField()->currentText() ;
        return true;
    }

    return false;
}


bool ServiceEditor::updateNode(NodeListT::iterator & _node)
{
    _node->name = nameField()->text() ;
    _node->type = typeField()->currentIndex();
    _node->status_crule = statusCalcRuleField()->currentIndex();
    _node->status_prule = statusPropRuleField()->currentIndex();
    _node->icon = iconField()->currentText();
    _node->description = descriptionField()->toPlainText();
    _node->alarm_msg  = alarmMsgField()->toPlainText();
    _node->notification_msg = notificationMsgField()->toPlainText();
    if( _node->type == NodeType::ALARM_NODE ) _node->child_nodes = checkField()->currentText() ;

    return true;
}

void ServiceEditor::setContent(const NodeListT & _node_map, const QString& _nodeId)
{
    NodeListT::const_iterator node = _node_map.find(_nodeId);
    if( node != _node_map.end()) setContent(node);
}


void ServiceEditor::setContent(NodeListT::const_iterator _node)
{
    nameField()->setText(_node->name) ;
    typeField()->setCurrentIndex(_node->type) ;
    statusCalcRuleField()->setCurrentIndex(_node->status_crule);
    statusPropRuleField()->setCurrentIndex(_node->status_prule) ;
    iconField()->setCurrentIndex(iconField()->findText((_node->icon))) ;
    descriptionField()->setText(_node->description) ;
    alarmMsgField()->setText(_node->alarm_msg) ;
    notificationMsgField()->setText(_node->notification_msg) ;

    QString checkId = "";
    if(_node->type == NodeType::ALARM_NODE) {
        QListWidget* checks = checkListField();
        QStringList childNodes = _node->child_nodes.split(Parser::CHILD_NODES_SEP);
        QStringList::iterator childNodeIt = childNodes.begin();
        if (childNodeIt != childNodes.end()) {
            checkId = (*childNodeIt).trimmed();
            CheckItemList checkItems = checks->findItems(checkId, Qt::MatchExactly);
            CheckItemList::const_iterator _it = checkItems.begin();
            if(_it == checkItems.end()) {
                checkField()->addItem(checkId);
                QListWidgetItem* item = new QListWidgetItem(checkId);
                checks->addItem(item);
                checks->setItemSelected(item, true) ;
            } else {
                checks->setItemSelected(*_it, true) ;
            }
        }
        childNodes.clear();
    }
    checkField()->setCurrentIndex(checkField()->findText(checkId, Qt::MatchExactly)) ;
}


void ServiceEditor::layoutEditorComponents(void)
{
    currentLine = 0;
    loadLabelFields(); currentLine++ ;
    loadTypeFields(); currentLine++ ;
    loadStatusHandlingFields(); currentLine++ ;
    loadIconFields(); currentLine++ ;
    loadDescriptionFields(); currentLine++ ;
    loadAlarmMsgFields(); currentLine++ ;
    loadNotificationMsgFields(); currentLine++ ;
    loadCheckField(); currentLine++ ;
    loadButtonBox(); currentLine++ ;
    setEnableFields(false) ;
}


void ServiceEditor::loadLabelFields()
{
    nameField()->setMaxLength( MAX_NODE_NAME ) ;
    layout->addWidget(editorItemsList["nameLabel"], currentLine, 0);
    layout->addWidget(nameField(),currentLine,1,1,2);
}


void ServiceEditor::loadDescriptionFields()
{
    layout->addWidget(editorItemsList["descriptionLabel"], currentLine, 0);
    layout->addWidget(descriptionField(),currentLine,1,1,2);
}


void ServiceEditor::loadTypeFields()
{
    typeField()->addItem( NodeType::toString(NodeType::SERVICE_NODE) );
    typeField()->addItem( NodeType::toString(NodeType::ALARM_NODE) );

    layout->addWidget(editorItemsList["typeLabel"], currentLine, 0);
    layout->addWidget(typeField(),currentLine,1,1,2);
}

void ServiceEditor::loadStatusHandlingFields(void)
{
    ComboBoxItemsT crules = SvNavigator::calcRules();
    QString defaultRule = StatusCalcRules::label(StatusCalcRules::HighCriticity) ;
    statusCalcRuleField()->addItem(tr("Calculation rule (Default is")%" "%defaultRule+")", StatusCalcRules::HighCriticity);

    foreach(const QString & rule, crules.keys()) {
        statusCalcRuleField()->addItem(rule, crules.value(rule));
    }

    ComboBoxItemsT prules = SvNavigator::propRules();
    defaultRule = StatusPropRules::label(StatusPropRules::Unchanged) ;
    statusPropRuleField()->addItem(tr("Propagation rule (Default is")%" "%defaultRule+")", StatusPropRules::Unchanged);
    foreach(const QString & rule, prules.keys()) {
        statusPropRuleField()->addItem(rule, prules.value(rule));
    }
    layout->addWidget(editorItemsList["priorityLabel"], currentLine, 0);
    layout->addWidget(statusCalcRuleField(),currentLine,1);
    layout->addWidget(statusPropRuleField(),currentLine,2);
}

void ServiceEditor::loadAlarmMsgFields()
{
    layout->addWidget(editorItemsList["alarmMsgLabel"], currentLine, 0);
    layout->addWidget(alarmMsgField(),currentLine,1,1,2);
}


void ServiceEditor::loadNotificationMsgFields()
{
    layout->addWidget(editorItemsList["notificationMsgLabel"], currentLine, 0);
    layout->addWidget(notificationMsgField(),currentLine,1,1,2);
}


void ServiceEditor::loadIconFields()
{
    IconMapT icons = GraphView::nodeIcons() ;

    QString header = "-->Select a icon (Default is " + GraphView::DEFAULT_ICON + ")" ;
    iconField()->addItem(header, icons.value(GraphView::DEFAULT_ICON));
    foreach(const QString & label, icons.keys()) {
        QString path = icons.value(label) ;
        iconField()->addItem(QIcon(path), label, icons.value(path));
    }
    layout->addWidget(editorItemsList["iconNameLabel"], currentLine, 0);
    layout->addWidget(iconField(),currentLine, 1, 1, 2);
}


void ServiceEditor::loadCheckField(void)
{
    layout->addWidget(editorItemsList["lowLevelAlarmsLabel"], currentLine, 0, 2, 1);
    layout->addWidget(checkField(), currentLine, 1, 1, 2);
    checkField()->setEditable(true) ;
    //TODO	loadStatusFile() ;
}

void ServiceEditor::loadButtonBox(void)
{
    layout->addWidget(buttonBox, currentLine, 2);
}

void ServiceEditor::handleCloseClick(void)
{
    emit closeClicked();
}

void ServiceEditor::handleSaveClick(void)
{
    emit saveClicked();
}

void ServiceEditor::handleReturnPressed(void)
{
    emit returnPressed() ;
}

void ServiceEditor::handleNodeTypeChanged( const QString & _text)
{
    if( _text == NodeType::toString(NodeType::ALARM_NODE) ) {
        setEnableFields(true);
    } else {
        setEnableFields( false ) ;
        checkListField()->clearSelection() ;
        checkField()->setCurrentIndex( 0 ) ;
    }
}

void ServiceEditor::handleNodeTypeActivated( const QString & _text)
{
    if( _text == NodeType::toString(NodeType::ALARM_NODE) ) {
        emit nodeTypeActivated( NodeType::ALARM_NODE ) ;
    } else {
        emit nodeTypeActivated( NodeType::SERVICE_NODE ) ;
    }
}


void ServiceEditor::addEvent(void)
{
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(handleSaveClick())) ;
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(handleCloseClick())) ;
    connect(nameField(), SIGNAL(returnPressed ()), this, SLOT(handleReturnPressed() ) ) ;
    connect(typeField(), SIGNAL(currentIndexChanged(const QString &)), this, SLOT(handleNodeTypeChanged( const QString & ) ) ) ;
    connect(typeField(), SIGNAL(activated(const QString &)), this, SLOT(handleNodeTypeActivated( const QString & ) ) ) ;
}
