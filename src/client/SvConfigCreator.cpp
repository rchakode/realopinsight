/*
 * SvConfigCreator.cpp
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


#include "SvConfigCreator.hpp"
#include "GraphView.hpp"
#include "Utils.hpp"


SvCreator::SvCreator(const qint32 & _user_role)
    : userRole (_user_role),
      hasToBeSaved (0),
      openedFile(""),
      selectedNode(""),
      selectedNodeId(""),
      settings(new Settings()),
      data(new Struct()),
      mainSplitter(new QSplitter(this)),
      navigationTree(new SvNavigatorTree(true)),
      editor(new ServiceEditor()),
      menuBar(new QMenuBar(this)),
      toolBar(new QToolBar("Tool Bar")),
      nodeContextMenu(new QMenu())
{
    mainSplitter->addWidget(navigationTree);
    mainSplitter->addWidget(editor);
    setCentralWidget(mainSplitter);
    resize();
}

SvCreator::~SvCreator()
{
    /*Don't destroy settings. Otherwise the associated file will be removed*/
    delete data;
    delete navigationTree;
    delete editor;
    delete mainSplitter;
    unloadMenu();
}


void SvCreator::contextMenuEvent(QContextMenuEvent *_event)
{
    QPoint global_pos = _event->globalPos();
    QList<QTreeWidgetItem*> tnodes = navigationTree->selectedItems();

    if ( tnodes.length()) {
        selectedNodeId = tnodes[0]->data(0, QTreeWidgetItem::UserType).toString();
        nodeContextMenu->exec( global_pos );
    }
}

void SvCreator::closeEvent(QCloseEvent * event)
{
    close( true );
    QMainWindow::closeEvent( event );
}

void SvCreator::unloadMenu(void)
{
    menuList.clear();
    subMenuList.clear();
    delete nodeContextMenu;
    delete menuBar;
}

void SvCreator::load(const QString& _filename)
{
    setWindowTitle(tr("%1 Editor | %2").arg(appName).arg(openedFile));
    loadMenu();
    addEvents();
    loadFile(_filename);
    show();
}


void SvCreator::open(void)
{
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("%1 | Select a configuration file").arg(appName),
                                                ".",
                                                tr("Xml files (*.xml);;All files (*)"));

    if(path.length()) loadFile(path);
}


void SvCreator::loadFile(const QString& _filename)
{
    Parser parser;
    if(_filename == NULL) return;
    Utils::clear(*data);
    if (! parser.parseSvConfig(_filename, *data) ) {
        Utils::alert(tr("Unable to open the file %1").arg(_filename));
        exit(1);
    }
    navigationTree->update(data);
    openedFile = _filename;
}

void SvCreator::import(){
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select the Status File %").arg(appName),
                                                ".",
                                                tr("Data files (*.dat);;All files (*)"));
    if(path.length()) editor->loadStatusFile(path);
}

void SvCreator::newBusinessView(void)
{
    NodeT node;
    if( ! close(false) ) {
        Utils::clear(*data);
        node.id = SvNavigatorTree::rootID;
        node.name = "New Business view";
        node.child_nodes.clear();
        node.status = MonitorBroker::UNKNOWN;
        node.icon = GraphView::DEFAULT_ICON;
        node.type = NodeType::SERVICE_NODE;
        node.parent.clear();

        data->nodes[node.id] = node;
        SvNavigatorTree::addNode(data->tree_items, node);
        navigationTree->update(data);

        fillEditorFromService(data->tree_items[node.id]);
        openedFile = "";
        selectedNode = SvNavigatorTree::rootID;
        hasToBeSaved = 1;
    }
}


void SvCreator::newNode(void)
{
    static int count = 1;
    NodeListT::iterator p_node_it;
    NodeT node;

    p_node_it = data->nodes.find( selectedNodeId );
    if(p_node_it == data->nodes.end() || p_node_it->type == NodeType::ALARM_NODE ) {
        Utils::alert(tr("This action not allowed on the target node"));
        return;
    }

    node.id = QTime().currentTime().toString("HHmmsszzz");
    node.parent = selectedNodeId;
    node.name = "sub service " + QString::number(count) , count ++;
    node.type = NodeType::SERVICE_NODE;
    node.status = MonitorBroker::UNKNOWN; // TODO no acknowledged
    node.icon = GraphView::DEFAULT_ICON;
    node.child_nodes = "";

    SvNavigatorTree::addNode(data->tree_items, node, true);
    data->nodes[node.id] = node;
    p_node_it->child_nodes += (p_node_it->child_nodes != "")? Parser::CHILD_NODES_SEP + node.id : node.id;

    navigationTree->setCurrentItem(data->tree_items[node.id]);
    fillEditorFromService(data->tree_items[node.id]);
}


void SvCreator::deleteNode(void)
{
    QMessageBox msg_box;

    msg_box.setText(tr("Do you really want to delete the service and its sub services?"));
    msg_box.setWindowTitle(tr("%1 Editor | Deleting service").arg(appName));

    msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    switch ( msg_box.exec() )
    {
    case QMessageBox::Yes:
        deleteNode( selectedNodeId );
        break;

    default:
        break;
    }

}

void SvCreator::deleteNode(const QString & _node_id)
{
    QStringList ud_services;
    NodeListT::iterator node_it, p_node_it;
    QStringList::iterator uds_it;
    TreeNodeItemListT::iterator p_item, item, end_items;

    node_it =  data->nodes.find( _node_id );
    if( node_it != data->nodes.end() ) {
        if( node_it->type != NodeType::ALARM_NODE &&  node_it->child_nodes != "" )
        {
            ud_services = node_it->child_nodes.split( Parser::CHILD_NODES_SEP );

            for( uds_it = ud_services.begin(); uds_it != ud_services.end(); uds_it++ ) {
                deleteNode (*uds_it);
            }

            ud_services.clear();
        }

        end_items =  data->tree_items.end();
        item = data->tree_items.find( _node_id );
        p_item = data->tree_items.find( node_it->parent );

        if( p_item != end_items && item != end_items ) {
            QRegExp regex;
            regex.setPattern(
                        "|^" + _node_id + Parser::CHILD_NODES_SEP +
                        "|^" + _node_id + "$" +
                        "|" + Parser::CHILD_NODES_SEP  + _node_id
                        );
            p_node_it = data->nodes.find( node_it->parent );
            if (p_node_it != data->nodes.end() ) p_node_it->child_nodes.remove( regex );

            data->nodes.remove( _node_id );
            data->tree_items.remove( _node_id );
            (*p_item)->takeChild( (*p_item)->indexOfChild( (*item) ) );
        }
    }

}


void SvCreator::save(void)
{
    if( selectedNode != "" ) {
        fillEditorFromService(data->tree_items[selectedNode]);
    }

    if ( openedFile == "" ) {
        saveAs();
    } else {
        saveInFile( openedFile );
    }

}

void SvCreator::saveAs(void)
{
    QString config_file;
    config_file = QFileDialog::getSaveFileName(this,
                                               tr("Select the destination path %1").arg(appName),
                                               ".",
                                               tr("Xml files (*.xml);; All files (*)"));
    saveInFile(config_file);
    setWindowTitle(tr("%1 Editor | %2").arg(appName).arg(openedFile));
}

int SvCreator::close( const bool & _close_windows )
{
    if ( ! hasToBeSaved ) {
        if( _close_windows ) qApp->quit();
    }

    QMessageBox mbox;
    mbox.setWindowTitle(tr("%1 | Save change?").arg(appName));
    mbox.setText(tr("The document has been modified.\nDo you want to save your changes?"));

    if( _close_windows ) {
        mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel | QMessageBox::Discard);
    } else {
        mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel | QMessageBox::No);
    }

    switch ( mbox.exec() )
    {
    case QMessageBox::Yes:
        save();

    case QMessageBox::Discard:
        qApp->quit();
        break;

    case QMessageBox::Cancel:
        return 1;
        break;

    default:
        break;
    }

    return 0;
}


void SvCreator::handleSelectedNodeChanged(void)
{
    QList<QTreeWidgetItem*> items;

    items = navigationTree->selectedItems();

    if( items.length() ) fillEditorFromService(*(items.begin()));
}


void SvCreator::handleTreeNodeMoved(QString _node_id)
{
    TreeNodeItemListT::iterator tnode_it;
    NodeListT::iterator node_it, p_node_it;
    QTreeWidgetItem* tnode_p;
    QRegExp regex;

    tnode_it =  data->tree_items.find(_node_id);

    if( tnode_it != data->tree_items.end() ) {

        tnode_p = (*tnode_it)->parent();
        if( tnode_p ) {
            node_it = data->nodes.find(_node_id);

            if( node_it != data->nodes.end() ) {
                /* Remove the node on its old parent's child list */
                regex.setPattern(
                            "|^" + _node_id + Parser::CHILD_NODES_SEP +
                            "|^" + _node_id + "$" +
                            "|" + Parser::CHILD_NODES_SEP  + _node_id
                            );
                p_node_it = data->nodes.find( node_it->parent );
                if( p_node_it != data->nodes.end() ) {
                    p_node_it->child_nodes.remove( regex );
                }

                /* Add the node on its new parent's child list */
                node_it->parent = tnode_p->data(0, QTreeWidgetItem::UserType).toString();
                p_node_it = data->nodes.find( node_it->parent );
                if(p_node_it != data->nodes.end()) {
                    p_node_it->child_nodes += (p_node_it->child_nodes != "")?
                                Parser::CHILD_NODES_SEP + _node_id : _node_id;
                }
            }
        }
    }
}


void SvCreator::handleNodeTypeActivated(qint32 _type)
{
    NodeListT::iterator node_it;


    node_it = data->nodes.find( selectedNode );

    if(node_it != data->nodes.end()) {
        if(_type == NodeType::SERVICE_NODE){
            if ( node_it->type == NodeType::ALARM_NODE ) {
                //TODO A bug has been reported
                node_it->child_nodes.clear();
                if( editor->updateNode(node_it) ) {
                    data->tree_items[selectedNode]->setText(0, node_it->name);
                    hasToBeSaved = 1;
                }
            }

        } else {
            if ( node_it->type == NodeType::SERVICE_NODE && ! node_it->child_nodes.isEmpty() ) {
                editor->typeField()->setCurrentIndex( 0 );
                Utils::alert(tr("This action required that the service has no sub service"));
            } else {
                if(editor->updateNode(node_it)) {
                    data->tree_items[selectedNode]->setText(0, node_it->name);
                    hasToBeSaved = 1;
                }
            }
        }
    }
}

void SvCreator::handleShowOnlineResources(void)
{
    QDesktopServices launcher;
    launcher.openUrl(QUrl("http://www.ngrt4n.com/docs/"));
}

void SvCreator::handleShowAbout(void)
{
    Preferences about(userRole, Preferences::ShowAbout);
    about.exec();
}

void SvCreator::fillEditorFromService(QTreeWidgetItem * _item )
{
    NodeListT::iterator node = data->nodes.find(selectedNode);
    if(node != data->nodes.end()) {
        if(editor->updateNode(node)) {
            data->tree_items[selectedNode]->setText(0, node->name);
            hasToBeSaved = 1;
        }
    }
    selectedNode = _item->data(0, QTreeWidgetItem::UserType).toString();
    node = data->nodes.find(selectedNode);
    if( node != data->nodes.end() ) editor->setContent(node);
}


void SvCreator::handleReturnPressed(void)
{
    NodeListT::iterator node = data->nodes.find(selectedNode);

    if( node != data->nodes.end() ) {
        if( editor->updateNode(node) ) {
            data->tree_items[selectedNode]->setText(0, node->name);
            hasToBeSaved = 1;
        }
    }
}

void SvCreator::saveInFile(const QString& _filename)
{
    if( _filename == NULL) return;
    QTextStream ofile;
    QFile file(_filename);
    if( file.open(QIODevice::WriteOnly) ) {
        ofile.setDevice(&file);

        NodeListT::const_iterator node = data->nodes.find(SvNavigatorTree::rootID);
        if( node == data->nodes.end() ) return;

        ofile << "<ServiceView compat=\"2.0\">" << endl;
        ofile << "\t<Service id=\""<<node->id<<"\" type=\""<<node->type
              << "\" statusCalcRule=\""<<node->status_crule<< "\" statusPropRule=\""<<node->status_prule<< "\">"<< endl
              << "\t\t<Name>"<<node->name<<"</Name>"<<endl
              << "\t\t<Icon>"<<node->icon<<"</Icon>"<<endl
              << "\t\t<Description>"<<node->description<<"</Description>"<<endl
              << "\t\t<AlarmMsg>"<< node->alarm_msg<<"</AlarmMsg>" << endl
              << "\t\t<NotificationMsg>"<<node->notification_msg<<"</NotificationMsg>" << endl
              << "\t\t<SubServices>"<<node->child_nodes<<"</SubServices>" << endl
              << "\t</Service>"<<endl;

        for( node = data->nodes.begin();  node !=  data->nodes.end(); node++) {

            if(  node->id == SvNavigatorTree::rootID ||  node->parent.isEmpty() ) continue;
            ofile << "\t<Service id=\""<<node->id<<"\" type=\""<<node->type
                  << "\" statusCalcRule=\""<<node->status_crule<< "\" statusPropRule=\""<<node->status_prule<< "\">"<< endl
                  << "\t\t<Name>"<<node->name<<"</Name>"<<endl
                  << "\t\t<Icon>"<<node->icon<<"</Icon>"<<endl
                  << "\t\t<Description>"<<node->description<<"</Description>"<<endl
                  << "\t\t<AlarmMsg>"<< node->alarm_msg<<"</AlarmMsg>" << endl
                  << "\t\t<NotificationMsg>"<<node->notification_msg<<"</NotificationMsg>" << endl
                  << "\t\t<SubServices>"<<node->child_nodes<<"</SubServices>" << endl
                  << "\t</Service>"<<endl;
        }

        ofile << "</ServiceView>" << endl;
        hasToBeSaved = 0;
        openedFile = _filename;
        statusBar()->showMessage(_filename%" "%tr("writed"));
    }
    file.close();
}

void SvCreator::resize()
{
    QSize ui_size = qApp->desktop()->screen(0)->size() * 0.80;
    QList<qint32> frames_size;
    frames_size.push_back(ui_size.width() * 0.3);
    frames_size.push_back(ui_size.width() * 0.7);
    mainSplitter->setSizes(frames_size);
    mainSplitter->resize(ui_size);
    QMainWindow::resize(ui_size);
}

void SvCreator::loadMenu(void)
{
    subMenuList["NewNode"] = nodeContextMenu->addAction("&Add service");
    subMenuList["DeleteNode"] = nodeContextMenu->addAction("&Delete");

    menuList["MENU1"] = menuBar->addMenu(tr("&File")),
            subMenuList["NewFile"] = menuList["MENU1"]->addAction("New &File"),
            subMenuList["Open"] = menuList["MENU1"]->addAction(QIcon(":images/built-in/folder.png"), tr("&Open")),
            subMenuList["Save"] = menuList["MENU1"]->addAction(QIcon(":images/built-in/disket.png"), tr("&Save")),
            subMenuList["SaveAs"] = menuList["MENU1"]->addAction(QIcon(":images/built-in/disket.png"), tr("Save &As..."));
    menuList["MENU1"]->addSeparator(),
            subMenuList["Import"] = menuList["MENU1"]->addAction(QIcon(":images/built-in/import.png"), tr("&Import Status File"));
    menuList["MENU1"]->addSeparator(),
            subMenuList["Quit"] = menuList["MENU1"]->addAction(tr("&Quit"));
    menuList["MENU2"] = menuBar->addMenu(tr("&Help")),
            subMenuList["ShowOnlineResources"] = menuList["MENU2"]->addAction(tr("Online &Resources")),
            menuList["MENU2"]->addSeparator(),
            subMenuList["ShowAbout"] = menuList["MENU2"]->addAction(tr("&About %1").arg(appName));

    subMenuList["NewFile"]->setShortcut(QKeySequence::New);
    subMenuList["Open"]->setShortcut(QKeySequence::Open);
    subMenuList["Save"]->setShortcut(QKeySequence::Save);
    subMenuList["SaveAs"]->setShortcut(QKeySequence::SaveAs);
    subMenuList["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
    subMenuList["Quit"]->setShortcut(QKeySequence::Quit);

    toolBar->addAction(subMenuList["Save"]);
    toolBar->addAction(subMenuList["Open"]);
    toolBar->addAction(subMenuList["Import"]);

    setMenuBar(menuBar);
    addToolBar(toolBar);
}

void SvCreator::addEvents(void)
{
    connect( subMenuList["NewFile"],SIGNAL(triggered(bool)),this,SLOT(newBusinessView()));
    connect( subMenuList["NewNode"],SIGNAL(triggered(bool)),this,SLOT(newNode()));
    connect( subMenuList["DeleteNode"],SIGNAL(triggered(bool)),this,SLOT(deleteNode()));
    connect( subMenuList["Open"],SIGNAL(triggered(bool)),this,SLOT(open()));
    connect( subMenuList["Save"],SIGNAL(triggered(bool)),this,SLOT(save()));
    connect( subMenuList["SaveAs"],SIGNAL(triggered(bool)),this,SLOT(saveAs()));
    connect( subMenuList["Import"],SIGNAL(triggered(bool)),this,SLOT(import()));
    connect( subMenuList["Quit"],SIGNAL(triggered(bool)),this,SLOT(close()));
    connect( subMenuList["ShowAbout"],SIGNAL(triggered(bool)),this,SLOT(handleShowAbout()));
    connect( subMenuList["ShowOnlineResources"],SIGNAL(triggered(bool)),this,SLOT(handleShowOnlineResources()));
    connect( editor,SIGNAL(saveClicked()),this,SLOT(save()));
    connect( editor,SIGNAL(closeClicked()),this,SLOT(close()));
    connect( editor,SIGNAL(returnPressed()),this,SLOT(handleReturnPressed()));
    connect( editor,SIGNAL(nodeTypeActivated(qint32)),this,SLOT(handleNodeTypeActivated(qint32)));
    connect(navigationTree,SIGNAL(itemSelectionChanged()),this,SLOT(handleSelectedNodeChanged()) );
    connect(navigationTree,SIGNAL(treeNodeMoved(QString)),this,SLOT(handleTreeNodeMoved(QString)));
}

