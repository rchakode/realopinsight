/*
 * SvConfigCreator.cpp
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


#include "../include/SvConfigCreator.hpp"


SvCreator::SvCreator(const qint32 & _user_role)
: userRole (_user_role),
  hasToBeSaved (0),
  openedFile(""),
  selectedNode(""),
  selectedNodeId(""),
  settings( new Settings() ),
  snavStruct( new Struct() ),
  mainSplitter( new QSplitter(this) ),
  navigationTree( new SvNavigatorTree( true ) ),
  editor( new ServiceEditor() ),
  menuBar( new QMenuBar(this) ),
  toolBar( new QToolBar("toolBar") ),
  nodeContextMenu( new QMenu() )
  {
  }

SvCreator::~SvCreator()
{
	// do not delete settings. Otherwise, the associated will be removed
	delete snavStruct ;
	delete navigationTree ;
	delete editor ;
	delete mainSplitter ;
	unloadMenu() ;
}


void SvCreator::contextMenuEvent(QContextMenuEvent *_event)
{
	QPoint global_pos ;
	QList<QTreeWidgetItem*> tnodes ;

	global_pos = _event->globalPos() ;
	tnodes = navigationTree->selectedItems() ;

	if ( tnodes.length())
	{
		selectedNodeId = tnodes[0]->text(TREE_NODE_ID_COLUMN) ;
		nodeContextMenu->exec( global_pos ) ;
	}
}

void SvCreator::closeEvent(QCloseEvent * event)
{
	close( true ) ;
	QMainWindow::closeEvent( event ) ;
}

void SvCreator::unloadMenu(void)
{
	menuList.clear() ;
	subMenuList.clear() ;
	delete nodeContextMenu ;
	delete menuBar ;
}

void SvCreator::load(const QString& _filename)
{
	loadMenu();
	addEvents() ;
	loadFile(_filename);

	mainSplitter->addWidget( navigationTree ) ;
	mainSplitter->addWidget( editor ) ;
	setCentralWidget( mainSplitter ) ;
	setWindowTitle(openedFile + " - " + APP_SHORT_NAME + " | Monitoring View Configuration Editor") ;
	show();
}


void SvCreator::open(void)
{
	QString root_id, config_file;

	config_file = QFileDialog::getOpenFileName(this,
			"Select view configuration file - " + APP_SHORT_NAME,
			".", "Xml files (*.xml)");

	loadFile( config_file );
}


void SvCreator::loadFile(const QString& _filename)
{
	QString root_id;
	Parser config_parser;

	if(_filename != NULL)
	{
		snavStruct->clear() ;
		config_parser.parseSvConfig( _filename, (*snavStruct) );
		navigationTree->update( snavStruct ) ;
		openedFile = _filename ;
	}
}


void SvCreator::newBusinessView(void)
{
	NodeT node;

	if( ! close(false) )
	{
		snavStruct->clear() ;
		node.id = snavStruct->root_id  = SvNavigatorTree::rootID ;
		node.name = "New Business view" ;
		node.child_nodes.clear() ;
		node.status = NAGIOS_UNKNOWN ;  // TODO no acknowledged
		node.icon = DEFAULT_ICON ;
		node.type = SERVICE_NODE ;
		node.parent.clear() ;  				//root has not parent

		snavStruct->node_list[SvNavigatorTree::rootID] = node;
		SvNavigatorTree::addNode(snavStruct->tree_item_list, node);
		navigationTree->update( snavStruct ) ;

		fillEditorFromService( snavStruct->tree_item_list[node.id] );

		openedFile = "";
		selectedNode = SvNavigatorTree::rootID ;
		hasToBeSaved = 1;
	}

}


void SvCreator::newNode(void)
{
	static int count = 1 ;
	NodeListT::iterator p_node_it ;
	NodeT node;

	p_node_it = snavStruct->node_list.find( selectedNodeId ) ;
	if(p_node_it == snavStruct->node_list.end() || p_node_it->type == ALARM_NODE )
	{
		QMessageBox::warning(this, "Warning! | " + APP_SHORT_NAME,
				"Action not allowed on the target node", QMessageBox::Ok) ;
		return ;
	}

	node.id = "S" + QTime().currentTime().toString("HHmmsszzz");
	node.parent = selectedNodeId ;
	node.name = "sub service " + QString::number(count) , count ++ ;
	node.type = SERVICE_NODE ;
	node.status = NAGIOS_UNKNOWN ; // TODO no acknowledged
	node.icon = DEFAULT_ICON ;
	node.child_nodes = "" ;

	SvNavigatorTree::addNode(snavStruct->tree_item_list, node, true) ;
	snavStruct->node_list[node.id] = node ;
	p_node_it->child_nodes += (p_node_it->child_nodes != "")? CHILD_NODES_SEP + node.id : node.id ;

	navigationTree->setCurrentItem(snavStruct->tree_item_list[node.id]) ;
	fillEditorFromService(snavStruct->tree_item_list[node.id]);
}


void SvCreator::deleteNode(void)
{
	QMessageBox msg_box ;

	msg_box.setText("Would you really want to delete the service and (possibly) its sub services?");
	msg_box.setWindowTitle("Delete service ? - " + APP_SHORT_NAME);

	msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

	switch ( msg_box.exec() )
	{
	case QMessageBox::Yes:
		deleteNode( selectedNodeId ) ;
		break;

	default:
		break;
	}

}

void SvCreator::deleteNode(const QString & _node_id)
{
	QStringList ud_services ;
	NodeListT::iterator node_it, p_node_it ;
	QStringList::iterator uds_it ;
	TreeNodeItemListT::iterator p_item, item, end_item_list ;


	node_it =  snavStruct->node_list.find( _node_id ) ;
	if( node_it != snavStruct->node_list.end() )
	{
		if( node_it->type != ALARM_NODE &&  node_it->child_nodes != "" )
		{
			ud_services = node_it->child_nodes.split( CHILD_NODES_SEP );

			for( uds_it = ud_services.begin(); uds_it != ud_services.end(); uds_it++ )
			{
				deleteNode (*uds_it) ;
			}

			ud_services.clear();
		}

		end_item_list =  snavStruct->tree_item_list.end() ;
		item = snavStruct->tree_item_list.find( _node_id ) ;
		p_item = snavStruct->tree_item_list.find( node_it->parent ) ;

		if( p_item != end_item_list && item != end_item_list )
		{
			QRegExp regex ;
			regex.setPattern(
					"|^" + _node_id + CHILD_NODES_SEP +
					"|^" + _node_id + "$" +
					"|" + CHILD_NODES_SEP  + _node_id
			) ;
			p_node_it = snavStruct->node_list.find( node_it->parent ) ;
			if (p_node_it != snavStruct->node_list.end() ) p_node_it->child_nodes.remove( regex ) ;

			snavStruct->node_list.remove( _node_id ) ;
			snavStruct->tree_item_list.remove( _node_id ) ;
			(*p_item)->takeChild( (*p_item)->indexOfChild( (*item) ) );
		}
	}

}


void SvCreator::save(void)
{
	if( selectedNode != "" )
	{
		fillEditorFromService(snavStruct->tree_item_list[selectedNode]);
	}

	if ( openedFile == "" )
	{
		saveAs();
	}
	else
	{
		saveInFile( openedFile );
	}

}

void SvCreator::saveAs(void)
{
	QString config_file;
	config_file = QFileDialog::getSaveFileName(this, "Select view configuration destination path - " + APP_SHORT_NAME, ".", "Xml files (*.xml)");
	saveInFile(config_file);
	setWindowTitle(openedFile + " - " + APP_SHORT_NAME + " | Monitoring View Builder") ;
}

int SvCreator::close( const bool & _close_windows )
{
	QMessageBox msg_box;

	if ( hasToBeSaved )
	{
		msg_box.setWindowTitle("Save change? - " + APP_SHORT_NAME);
		msg_box.setText("The document has been modified.\n "
				"Do you want to save your changes?");

		if( _close_windows )
		{
			msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel | QMessageBox::Discard);
		}

		else
		{
			msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel | QMessageBox::No);
		}

		switch ( msg_box.exec() )
		{
		case QMessageBox::Yes:
			save( );

		case QMessageBox::Discard:
			qApp->quit();
			break;

		case QMessageBox::Cancel:
			return 1;
			break;

		default:
			break;
		}
	}
	else
	{
		if( _close_windows )
			qApp->quit();
	}

	return 0;
}


void SvCreator::handleSelectedNodeChanged(void)
{
	QList<QTreeWidgetItem*> items ;

	items = navigationTree->selectedItems() ;

	if( items.length() ) fillEditorFromService( *(items.begin()) );
}


void SvCreator::handleTreeNodeMoved(QString _node_id)
{
	TreeNodeItemListT::iterator tnode_it ;
	NodeListT::iterator node_it, p_node_it ;
	QTreeWidgetItem* tnode_p ;
	QRegExp regex ;

	tnode_it =  snavStruct->tree_item_list.find(_node_id) ;

	if( tnode_it != snavStruct->tree_item_list.end() )
	{
		tnode_p = (*tnode_it)->parent() ;

		if( tnode_p )
		{
			node_it = snavStruct->node_list.find(_node_id) ;

			if( node_it != snavStruct->node_list.end() )
			{
				/* Remove the node on its old parent's child list */

				regex.setPattern(
						"|^" + _node_id + CHILD_NODES_SEP +
						"|^" + _node_id + "$" +
						"|" + CHILD_NODES_SEP  + _node_id
				) ;
				p_node_it = snavStruct->node_list.find( node_it->parent ) ;
				if( p_node_it != snavStruct->node_list.end() )
				{
					p_node_it->child_nodes.remove( regex ) ;
				}

				/* Add the node on its new parent's child list */

				node_it->parent = tnode_p->text(TREE_NODE_ID_COLUMN) ;
				p_node_it = snavStruct->node_list.find( node_it->parent ) ;
				if( p_node_it != snavStruct->node_list.end() )
				{
					p_node_it->child_nodes += (p_node_it->child_nodes != "")?
							CHILD_NODES_SEP + _node_id : _node_id ;
				}
			}
		}
	}
}


void SvCreator::handleNodeTypeActivated(qint32 _type)
{
	NodeListT::iterator node_it ;


	node_it = snavStruct->node_list.find( selectedNode ) ;

	if( node_it != snavStruct->node_list.end() )
	{
		if( _type == SERVICE_NODE )
		{
			if ( node_it->type == ALARM_NODE )
			{
				//TODO a bug exists. To be debuged
				node_it->child_nodes.clear() ;
				if( editor->updateNode(node_it) )
				{
					snavStruct->tree_item_list[selectedNode]->setText(0, node_it->name);
					hasToBeSaved = 1 ;
				}
			}

		}
		else
		{
			if ( node_it->type == SERVICE_NODE && ! node_it->child_nodes.isEmpty() )
			{
				QMessageBox::warning(this, "Warning! | " + APP_SHORT_NAME,
						"This action required that the service has no sub service", QMessageBox::Ok) ;
				editor->typeField()->setCurrentIndex( 0 ) ;
			}
			else
			{
				if( editor->updateNode(node_it) )
				{
					snavStruct->tree_item_list[selectedNode]->setText(0, node_it->name);
					hasToBeSaved = 1 ;
				}
			}
		}
	}
}

void SvCreator::handleShowOnlineResources(void)
{
	QDesktopServices app_launcher ;
	app_launcher.openUrl(QUrl("http://www.ngrt4n.com/docs/")) ;
}

void SvCreator::handleShowAbout(void)
{
	PreferencesDialog about(userRole, PreferencesDialog::ShowAbout) ;
	about.exec() ;
}

void SvCreator::fillEditorFromService(QTreeWidgetItem * _service_item )
{
	NodeListT::iterator node_it ;

	node_it = snavStruct->node_list.find(selectedNode) ;

	if( node_it != snavStruct->node_list.end() )
	{
		if( editor->updateNode(node_it) )
		{
			snavStruct->tree_item_list[selectedNode]->setText(0, node_it->name);
			hasToBeSaved = 1 ;
		}
	}
	selectedNode = _service_item->text( TREE_NODE_ID_COLUMN ) ;

	node_it = snavStruct->node_list.find(selectedNode) ;

	if( node_it != snavStruct->node_list.end() ) editor->setContent(node_it);
}


void SvCreator::handleReturnPressed(void)
{
	NodeListT::iterator node_it ;

	node_it = snavStruct->node_list.find(selectedNode) ;

	if( node_it != snavStruct->node_list.end() )
	{
		if( editor->updateNode(node_it) )
		{
			snavStruct->tree_item_list[selectedNode]->setText(0, node_it->name);
			hasToBeSaved = 1 ;
		}
	}
}

void SvCreator::saveInFile(const QString& _filename)
{
	if( _filename != NULL)
	{
		QTextStream file_stream;
		NodeListT::const_iterator node_it;
		QFile file(_filename);
		if( file.open(QIODevice::WriteOnly) )
		{
			file_stream.setDevice(&file);

			node_it = snavStruct->node_list.find(SvNavigatorTree::rootID);

			if( node_it != snavStruct->node_list.end() )
			{
				file_stream << "<ServiceView id=\"" << SvNavigatorTree::rootID  << "\">" << endl
						<< "\t<Name>" <<  node_it->name << "</Name>" << endl
						<< "\t<SubServices>" << node_it->child_nodes << "</SubServices>" << endl;

				for( node_it = snavStruct->node_list.begin();  node_it !=  snavStruct->node_list.end();  node_it++)
				{
					if(  node_it->id != SvNavigatorTree::rootID ||  !  node_it->parent.isEmpty() )
					{
						file_stream << "\t<Service id=\""<<  node_it->id << "\"" << " type=\""<<  node_it->type
								<< "\" statusCalcRule=\""<<  node_it->status_calc_rule << "\">"<< endl
								<< "\t\t<Name>" <<  node_it->name << "</Name>" << endl
								<< "\t\t<Icon>" <<  node_it->icon << "</Icon>" << endl
								<< "\t\t<Description>" <<  node_it->description << "</Description>" << endl
								<< "\t\t<AlarmMsg>" <<  node_it->alarm_msg << "</AlarmMsg>" << endl
								<< "\t\t<NotificationMsg>" <<  node_it->notification_msg << "</NotificationMsg>" << endl
								<< "\t\t<SubServices>" << node_it->child_nodes << "</SubServices>" << endl ;

						if(node_it->type == ALARM_NODE )
						{

							file_stream << "\t\t<Status>" <<  node_it->status << "</Status>" << endl ;
						}
						else
						{

							file_stream << "\t\t<PropagationRule>" <<  node_it->propagation_rule << "</PropagationRule>" << endl ;
						}

						file_stream << "\t</Service>"<< endl;
					}
				}

				file_stream << "</ServiceView>" << endl;
				hasToBeSaved = 0;
				openedFile = _filename;
				qDebug() << _filename <<" writed.";
			}
		}
		file.close();
	}
}



void SvCreator::resize(const qint32 & _windows_width,
		const qint32 & _windows_height,
		const qint32 & _nav_tree_width,
		const qint32 & _editor_width)
{
	QList<qint32> frames_size;
	frames_size.push_back(_nav_tree_width);
	frames_size.push_back(_editor_width);
	mainSplitter->setSizes(frames_size);
	mainSplitter->resize(_windows_width, _windows_height);
}

void SvCreator::loadMenu(void)
{
	QIcon disket_icon, folder_icon ;
	disket_icon.addFile(":images/disket.png");
	folder_icon.addFile(":images/folder.png");

	subMenuList["NewNode"] = nodeContextMenu->addAction("&Add service") ;
	subMenuList["DeleteNode"] = nodeContextMenu->addAction("&Delete") ;

	menuList["MENU1"] = menuBar->addMenu(tr("&File")),
			subMenuList["NewFile"] = menuList["MENU1"]->addAction("New &File"),
			subMenuList["Open"] = menuList["MENU1"]->addAction(folder_icon, "&Open"),
			subMenuList["Save"] = menuList["MENU1"]->addAction(disket_icon, "&Save"),
			subMenuList["SaveAs"] = menuList["MENU1"]->addAction(disket_icon, "Save &As...") ,
			menuList["MENU1"]->addSeparator(),
			subMenuList["Quit"] = menuList["MENU1"]->addAction("&Quit") ;
	menuList["MENU2"] = menuBar->addMenu(tr("&Help")),
			subMenuList["ShowOnlineResources"] = menuList["MENU2"]->addAction("Online &Resources"),
			menuList["MENU2"]->addSeparator(),
	subMenuList["ShowAbout"] = menuList["MENU2"]->addAction("&About " + APP_SHORT_NAME);

	subMenuList["NewFile"]->setShortcut(QKeySequence::New) ;
	subMenuList["Open"]->setShortcut(QKeySequence::Open) ;
	subMenuList["Save"]->setShortcut(QKeySequence::Save) ;
	subMenuList["SaveAs"]->setShortcut(QKeySequence::SaveAs) ;
	subMenuList["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents) ;
	subMenuList["Quit"]->setShortcut(QKeySequence::Quit) ;

	toolBar->addAction(subMenuList["Save"]) ;
	toolBar->addAction(subMenuList["Open"]) ;

	setMenuBar( menuBar ) ;
	addToolBar( toolBar ) ;
}

void SvCreator::addEvents(void)
{
	connect( subMenuList["NewFile"], SIGNAL(triggered(bool)), this, SLOT( newBusinessView() ));
	connect( subMenuList["NewNode"], SIGNAL(triggered(bool)), this, SLOT( newNode() ));
	connect( subMenuList["DeleteNode"], SIGNAL(triggered(bool)), this, SLOT( deleteNode() ));
	connect( subMenuList["Open"], SIGNAL(triggered(bool)), this, SLOT( open() ) );
	connect( subMenuList["Save"], SIGNAL(triggered(bool)), this, SLOT( save() ) );
	connect( subMenuList["SaveAs"], SIGNAL(triggered(bool)), this, SLOT( saveAs() ) );
	connect( subMenuList["Quit"], SIGNAL(triggered(bool)), this, SLOT( close() ) );
	connect( subMenuList["ShowAbout"], SIGNAL( triggered(bool) ), this, SLOT( handleShowAbout() ));
	connect( subMenuList["ShowOnlineResources"], SIGNAL( triggered(bool) ), this, SLOT( handleShowOnlineResources() ));
	connect( editor, SIGNAL(saveClicked()), this, SLOT(save()));
	connect( editor, SIGNAL(closeClicked()), this, SLOT(close()));
	connect( editor, SIGNAL(returnPressed()), this, SLOT(handleReturnPressed()));
	connect( editor, SIGNAL(nodeTypeActivated(qint32)), this, SLOT(handleNodeTypeActivated(qint32)));
	connect(navigationTree, SIGNAL(itemSelectionChanged()), this, SLOT(handleSelectedNodeChanged()) );
	connect(navigationTree, SIGNAL(treeNodeMoved(QString)), this, SLOT(handleTreeNodeMoved(QString)));

}

