/*
 * ServiceEditor.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 13-05-2012												   #
#																		   #
# This file is part of NGRT4N (http://ngrt4n.com).						   #
#																		   #
# NGRT4N is free software: you can redistribute it and/or modify		   #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.									   #
#																		   #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of		   #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	           #
# GNU General Public License for more details.							   #
#																		   #
# You should have received a copy of the GNU General Public License		   #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.		   #
#--------------------------------------------------------------------------#
 */

#include "../include/ServiceEditor.hpp"

ServiceEditor::ServiceEditor(QWidget* _parent )
: QWidget( _parent ),
  settings( new Settings() ),
  layout (new QGridLayout( this ) ),
  buttonBox(  new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close))
  {
	editorItemsList["nameLabel"] = new QLabel("Name") ;
	editorItemsList[NAME_FIELD] = new QLineEdit() ;

	editorItemsList["typeLabel"] = new QLabel("Type") ;
	editorItemsList[TYPE_FIELD] = new QComboBox() ;

	editorItemsList["iconNameLabel"] = new QLabel("Icon");
	editorItemsList[ICON_FIELD] = new QComboBox();

	editorItemsList["priorityLabel"] = new QLabel("Status Aggregation Rule") ;
	editorItemsList[STATUS_CALC_RULE_FIELD] = new QComboBox() ;

	editorItemsList["descriptionLabel"] = new QLabel("Description") ;
	editorItemsList[DESCRIPTION_FIELD] = new QTextEdit() ;

	editorItemsList["alarmMsgLabel"] = new QLabel("Alarm Message") ;
	editorItemsList[ALARM_MSG_FIELD]  = new QTextEdit() ;

	editorItemsList["notificationMsgLabel"] = new QLabel("Notification Message") ;
	editorItemsList[NOTIFICATION_MSG_FIELD] = new QTextEdit() ;

	editorItemsList["lowLevelAlarmsLabel"] = new QLabel("Related Check") ;
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

		if(labelPtr)
		{
			delete labelPtr;
		}
		else if(lineEditPtr)
		{
			delete  lineEditPtr;
		}
		else if (textBlockPtr)
		{
			delete textBlockPtr;
		}
		else if (comboBoxPtr)
		{
			delete comboBoxPtr;
		}
		editorItemsList.remove(key);

		iter = editorItemsList.begin();
	}
	editorItemsList.clear();

	delete buttonBox;
	delete layout;

}

void ServiceEditor::loadStatusFile(void)
{
	Parser parser ;
	NagiosChecksT nagios_checks ;
	parser.parseServiceStatus(settings->value(STATUS_FILE_KEY).toString(), nagios_checks);
	setCheckListField( nagios_checks );
}

void ServiceEditor::setCheckListField(const NagiosChecksT& _nagios_checks)
{
	NagiosChecksT::const_iterator check_it ;
	QStringList check_id_list ;

	checkField()->clear() ;
	check_id_list.clear() ;
	check_id_list.push_back("") ;

	for(check_it = _nagios_checks.begin(); check_it != _nagios_checks.end(); check_it++)
	{
		check_id_list.push_back( check_it->id ) ;
	}

	check_id_list.sort() ;
	checkField()->addItems( check_id_list ) ;
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
	CheckItemList child_node_items;
	NodeListT::iterator it;
	CheckItemList::iterator _it;

	it = static_cast<const NodeListT::iterator>(_node_map.find(_node_id));

	if( it != _node_map.end())
	{
		it->name = nameField()->text() ;

		it->type = typeField()->currentIndex();

		it->status_calc_rule = statusCalcRuleField()->currentIndex();

		it->icon = iconField()->currentText();

		it->description = descriptionField()->toPlainText();

		it->alarm_msg  = alarmMsgField()->toPlainText();

		it->notification_msg = notificationMsgField()->toPlainText();

		if( it->type == ALARM_NODE ) it->child_nodes =  checkField()->currentText() ;

		return true;
	}

	return false;
}


bool ServiceEditor::updateNode(NodeListT::iterator & _node_it)
{
	_node_it->name = nameField()->text() ;

	_node_it->type = typeField()->currentIndex();

	_node_it->status_calc_rule = statusCalcRuleField()->currentIndex();

	_node_it->icon = iconField()->currentText();

	_node_it->description = descriptionField()->toPlainText();

	_node_it->alarm_msg  = alarmMsgField()->toPlainText();

	_node_it->notification_msg = notificationMsgField()->toPlainText();

	if( _node_it->type == ALARM_NODE ) _node_it->child_nodes =  checkField()->currentText() ;

	return true;
}

void ServiceEditor::setContent(const NodeListT & _node_map, const QString& _node_id)
{
	NodeListT::const_iterator it;

	it = _node_map.find(_node_id);

	if( it != _node_map.end())
		setContent( it );
}


void ServiceEditor::setContent(NodeListT::const_iterator _node_it)
{
	QListWidget* lla_widget;
	CheckItemList child_node_items;
	QStringList child_nodes_list;
	QStringList::iterator child_node_it;
	CheckItemList::const_iterator _it;


	nameField()->setText(_node_it->name) ;

	typeField()->setCurrentIndex(_node_it->type) ;

	statusCalcRuleField()->setCurrentIndex(_node_it->status_calc_rule) ;

	iconField()->setCurrentIndex(iconField()->findText((_node_it->icon))) ;

	descriptionField()->setText(_node_it->description) ;

	alarmMsgField()->setText(_node_it->alarm_msg) ;

	notificationMsgField()->setText(_node_it->notification_msg) ;


	lla_widget = checkListField();

	child_nodes_list = _node_it->child_nodes.split( CHILD_NODES_SEP );
	child_node_it = child_nodes_list.begin();

	// TODO

	if ( child_node_it != child_nodes_list.end() )
	{
		checkField()->setCurrentIndex(checkField()->findText((*child_node_it).trimmed(), Qt::MatchExactly)) ;

		child_node_items = lla_widget->findItems((*child_node_it).trimmed(), Qt::MatchExactly);
		_it = child_node_items.begin();

		if( _it != child_node_items.end() )
		{
			// Retrieve the selected check item
			// after this the itemSelectionChanged signal is emited by lla_widget.
			// this signal is intercepted by a function (see addEvent)
			// for filling host- and check- fields

			lla_widget->setItemSelected(*_it, true) ;
		}
	}

	child_nodes_list.clear();
}


void ServiceEditor::layoutEditorComponents(void)
{
	currentLine = 0;
	loadLabelFields(), currentLine++ ;
	loadTypeFields(), currentLine++ ;
	loadPriorityFields(), currentLine++ ;
	loadIconFields(), currentLine++ ;
	loadDescriptionFields(), currentLine++ ;
	loadAlarmMsgFields(), currentLine++ ;
	loadNotificationMsgFields(), currentLine++ ;
	loadCheckField(), currentLine++ ;
	loadButtonBox(), currentLine++ ;
	setEnableFields(false) ;
}


void ServiceEditor::loadLabelFields()
{
	nameField()->setMaxLength( MAX_NODE_NAME ) ;
	layout->addWidget(editorItemsList["nameLabel"], currentLine, 0);
	layout->addWidget(nameField(), currentLine, 1);
}


void ServiceEditor::loadDescriptionFields()
{
	layout->addWidget(editorItemsList["descriptionLabel"], currentLine, 0);
	layout->addWidget(descriptionField(), currentLine, 1);
}


void ServiceEditor::loadTypeFields()
{
	typeField()->addItem( BUSINESS_PROCESS_NODE );
	typeField()->addItem( NATIVE_CHECK_NODE );

	layout->addWidget(editorItemsList["typeLabel"], currentLine, 0);
	layout->addWidget(typeField(), currentLine, 1);
}


void ServiceEditor::loadPriorityFields(void)
{
	statusCalcRuleField()->addItem(HIGH_CRITICITY_CALC_RULE);
	statusCalcRuleField()->addItem(WEIGHTED_CALC_RULE);
	statusCalcRuleField()->setCurrentIndex( 0 ) ;

	layout->addWidget(editorItemsList["priorityLabel"], currentLine, 0);
	layout->addWidget(statusCalcRuleField(), currentLine, 1);
}


void ServiceEditor::loadAlarmMsgFields()
{
	layout->addWidget(editorItemsList["alarmMsgLabel"], currentLine, 0);
	layout->addWidget(alarmMsgField(), currentLine, 1);
}


void ServiceEditor::loadNotificationMsgFields()
{
	layout->addWidget(editorItemsList["notificationMsgLabel"], currentLine, 0);
	layout->addWidget(notificationMsgField(), currentLine, 1);
}


void ServiceEditor::loadIconFields()
{
	QIcon icon("images/normal.png");
	QComboBox* iconsBox = iconField() ;

	iconsBox->addItem(DEFAULT_ICON) ;
	iconsBox->addItem(NETWORK_ICON) ;
	iconsBox->addItem(ROUTER_ICON) ;
	iconsBox->addItem(SWITCH_ICON) ;
	iconsBox->addItem(FIREWALL_ICON) ;

	iconsBox->addItem(SERVER_ICON) ;
	iconsBox->addItem(LINUX_ICON) ;
	iconsBox->addItem(WINDOWS_ICON) ;
	iconsBox->addItem(SOLARIS_ICON) ;
	iconsBox->addItem(WEBSERVER_ICON) ;
	iconsBox->addItem(DBSERVER_ICON) ;

	iconsBox->addItem(STORAGE_ICON) ;
	iconsBox->addItem(FILER_ICON) ;
	iconsBox->addItem(HARDDISK_ICON) ;

	iconsBox->addItem(APP_ICON) ;
	iconsBox->addItem(WEB_ICON) ;
	iconsBox->addItem(DB_ICON) ;
	iconsBox->addItem(PROCESS_ICON) ;
	iconsBox->addItem(LOG_ICON) ;

	iconsBox->addItem(CLOUD_ICON) ;
	iconsBox->addItem(HYPERVISOR_ICON) ;

	iconsBox->addItem(OTH_CHECK_ICON) ;

	layout->addWidget(editorItemsList["iconNameLabel"], currentLine, 0);
	layout->addWidget(iconsBox, currentLine, 1);
}


void ServiceEditor::loadCheckField(void)
{
	layout->addWidget(editorItemsList["lowLevelAlarmsLabel"], currentLine, 0, 2, 1);
	layout->addWidget(checkField(), currentLine, 1) , currentLine++ ;
	checkField()->setEditable( true ) ;
	loadStatusFile() ;
}


void ServiceEditor::loadButtonBox(void)
{
	layout->addWidget(buttonBox, currentLine, 1);
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
	if( _text == NATIVE_CHECK_NODE )
	{
		setEnableFields(true);
	}
	else
	{
		setEnableFields( false ) ;
		checkListField()->clearSelection() ;
		checkField()->setCurrentIndex( 0 ) ;
	}
}

void ServiceEditor::handleNodeTypeActivated( const QString & _text)
{
	if( _text == NATIVE_CHECK_NODE )
	{
		emit nodeTypeActivated( ALARM_NODE ) ;
	}
	else
	{
		emit nodeTypeActivated( SERVICE_NODE ) ;
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
