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
#include "utilsClient.hpp"

const QString SvCreator::NagiosCompatibleFormat="Nagios specific format(*.nag.ngrt4n.xml)";
const QString SvCreator::ZabbixCompatibleFormat="Zabbix specific format(*.zbx.ngrt4n.xml)";
const QString SvCreator::ZenossCompatibleFormat="Zenoss specific format(*.zns.ngrt4n.xml)";

SvCreator::SvCreator(const qint32& _userRole)
  : muserRole (_userRole),
    mhasLeftUpdates (false),
    mactiveFile(""),
    mselectedNode(""),
    msettings(new Settings()),
    mcoreData(new CoreDataT()),
    mainSplitter(new QSplitter(this)),
    mtree(new SvNavigatorTree(true)),
    meditor(new ServiceEditor()),
    mmenuBar(new QMenuBar(this)),
    mtoolBar(new QToolBar("Tool Bar")),
    mnodeContextMenu(new QMenu()),
    mclipboardData(NULL)
{
  mainSplitter->addWidget(mtree);
  mainSplitter->addWidget(meditor);
  setCentralWidget(mainSplitter);
  resize();
  statusBar()->showMessage(tr("Open or edit a file via the File menu"));
}

SvCreator::~SvCreator()
{
  delete mcoreData;
  delete mtree;
  delete meditor;
  delete mainSplitter;
  if (mclipboardData) delete mclipboardData;
  unloadMenu();
}


void SvCreator::contextMenuEvent(QContextMenuEvent*_event)
{
  QPoint pos = _event->globalPos();
  QList<QTreeWidgetItem*> selectedNodes = mtree->selectedItems();

  if (selectedNodes.length()) {
      mselectedNode = selectedNodes[0]->data(0, QTreeWidgetItem::UserType).toString();
      mnodeContextMenu->exec(pos);
    }
}

void SvCreator::closeEvent(QCloseEvent* event)
{
  treatCloseAction(true);
  QMainWindow::closeEvent(event);
}

void SvCreator::unloadMenu(void)
{
  mmenuList.clear();
  msubMenus.clear();
  delete mnodeContextMenu;
  delete mmenuBar;
}

void SvCreator::load(const QString& _path)
{
  loadMenu();
  addEvents();
  loadFile(_path);
  setWindowTitle(tr("%1 Editor - %2").arg(APP_NAME).arg(mactiveFile));
  statusBar()->showMessage(tr("Loaded."));
  show();
}


void SvCreator::open(void)
{
  QString path = QFileDialog::getOpenFileName(this,
                                              tr("%1 | Select a configuration file").arg(APP_NAME),
                                              ".",
                                              tr("%1;;%2;;%3;;Xml files(*.xml);;All files(*)")
                                              .arg(NagiosCompatibleFormat)
                                              .arg(ZabbixCompatibleFormat)
                                              .arg(ZenossCompatibleFormat));
  if (!path.isNull() && !path.isEmpty()) loadFile(path);
}


void SvCreator::loadFile(const QString& _path)
{
  if (_path == NULL) {
      newView();
    } else {
      utils::clear(*mcoreData);
      Parser parser;
      if (!parser.parseSvConfig(_path,*mcoreData)) {
          utils::alert(tr("Unable to open the file '%1'").arg(_path));
          exit(1);
        }
      mtree->update(mcoreData);
      mactiveFile = utils::getAbsolutePath(_path);
      setWindowTitle(tr("%1 Editor - %2").arg(APP_NAME).arg(mactiveFile));
    }
}

void SvCreator::import() {
  QString path = QFileDialog::getOpenFileName(this,
                                              tr("Select the Status File %").arg(APP_NAME),
                                              ".",
                                              tr("Data files (*.dat);;All files (*)"));
  if (!path.isNull() && !path.isEmpty()) meditor->loadStatusFile(path);
}

void SvCreator::newView(void)
{
  if (treatCloseAction(false) == 0) {
      utils::clear(*mcoreData);
      NodeT* node = createNode(SvNavigatorTree::RootId, tr("New View"), "");
      mcoreData->bpnodes.insert(node->id,*node);
      SvNavigatorTree::addNode(mcoreData->tree_items,*node);
      mtree->update(mcoreData);
      meditor->setContent(*node);
      mselectedNode = node->id;
      mhasLeftUpdates = true;
      mactiveFile.clear();
      setWindowTitle(tr("%1 Editor - unsaved document*").arg(APP_NAME));
    }
}


void SvCreator::newNode(void)
{
  static int count = 1;
  NodeT* node = createNode(utils::genNodeId(),
                           tr("sub service %1").arg(QString::number(count)),
                           mselectedNode);
  insertFromSelected(*node);
  count++;
}

NodeT* SvCreator::createNode(const QString& id,
                             const QString& label,
                             const QString& parent)
{
  NodeT* node = new NodeT;
  if (!node) {
      utils::alert(tr("Out of memory. the application will exit"));
      exit(1);
    }
  node->id = id;
  node->name = label;
  node->parent = parent;
  node->type = NodeType::SERVICE_NODE;
  node->severity = MonitorBroker::Unknown;
  node->sev_crule = CalcRules::HighCriticity;
  node->sev_prule = PropRules::Unchanged;
  node->icon = GraphView::DEFAULT_ICON;
  node->child_nodes = QString();
  return node;
}

void SvCreator::insertFromSelected(const NodeT& node)
{
  NodeListT::iterator pnode = mcoreData->bpnodes.find(mselectedNode);
  if (pnode == mcoreData->bpnodes.end() ||
      pnode->type == NodeType::ALARM_NODE) {
      utils::alert(tr("This action not allowed on the target node"));
      return;
    }
  pnode->child_nodes += (!(pnode->child_nodes).isEmpty())? Parser::CHILD_SEP%node.id : node.id;
  SvNavigatorTree::addNode(mcoreData->tree_items, node, true);
  mcoreData->bpnodes.insert(node.id, node);
  mtree->setCurrentItem(mcoreData->tree_items[node.id]);
  fillEditorFromService(mcoreData->tree_items[node.id]);
}


void SvCreator::deleteNode(void)
{
  QMessageBox msgBox;
  msgBox.setText(tr("Do you really want to delete the service and its sub services?"));
  msgBox.setWindowTitle(tr("Deleting service - %1 Editor").arg(APP_NAME));
  msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::Cancel);
  switch (msgBox.exec())
    {
    case QMessageBox::Yes:
      deleteNode(mselectedNode);
      break;
    default:
      break;
    }
}

void SvCreator::deleteNode(const QString& _nodeId)
{
  NodeListT::iterator node;
  if (!utils::findNode(mcoreData, _nodeId, node))
    return;

  if (node->type == NodeType::SERVICE_NODE && node->child_nodes != "") {
      foreach(const QString& checkId, node->child_nodes.split(Parser::CHILD_SEP)) {
          deleteNode(checkId);
        }
    }
  TreeNodeItemListT::iterator item = mcoreData->tree_items.find(_nodeId);
  TreeNodeItemListT::iterator pItem = mcoreData->tree_items.find(node->parent);
  if (pItem != mcoreData->tree_items.end() &&
      item != mcoreData->tree_items.end()) {
      QRegExp regex("|^" + _nodeId + Parser::CHILD_SEP +
                    "|^" + _nodeId + "$" +
                    "|" + Parser::CHILD_SEP  + _nodeId);

      NodeListT::iterator pNode = mcoreData->bpnodes.find(node->parent);
      if (pNode != mcoreData->bpnodes.end()) {
          pNode->child_nodes.remove(regex);
        }
      if (node->type == NodeType::ALARM_NODE) {
          mcoreData->cnodes.remove(_nodeId);
        } else {
          mcoreData->bpnodes.remove(_nodeId);
        }
      mcoreData->tree_items.remove(_nodeId);
      QTreeWidgetItem* obsolete = NULL;
      if ((obsolete = (*pItem)->takeChild((*pItem)->indexOfChild(*item))))
        delete obsolete;
    }
}


void SvCreator::copySelected(void)
{
  NodeListIteratorT node;
  if (utils::findNode(mcoreData, mselectedNode, node)) {
      if (!mclipboardData) mclipboardData = new NodeT;
     *mclipboardData =*node;
      mclipboardData->name+=" (Copy)";
      mclipboardData->child_nodes.clear();
    }
}

void SvCreator::pasteFromSelected(void)
{
  if (mclipboardData) {
      mclipboardData->id = utils::genNodeId();
      mclipboardData->parent = mselectedNode;
      insertFromSelected(*mclipboardData);
    } else {
      utils::alert(tr("There is no data in the clipboard!"));
    }
}


void SvCreator::save(void)
{
  if (!mselectedNode.isEmpty()) {
      fillEditorFromService(mcoreData->tree_items[mselectedNode]);
    }
  if (mactiveFile.isEmpty()) {
      saveAs();
    } else {
      recordData(mactiveFile);
    }
}

void SvCreator::saveAs(void)
{
  QString filter;
  QString path = QFileDialog::getSaveFileName(this,
                                              tr("Select the destination file | %1").arg(APP_NAME),
                                              mactiveFile,
                                              QString("%1;;%2;;%3;;")
                                              .arg(NagiosCompatibleFormat)
                                              .arg(ZabbixCompatibleFormat)
                                              .arg(ZenossCompatibleFormat),
                                              &filter);

  if (path.isNull()) {
      QString msg = tr("The path is not valid!");
      utils::alert(msg);
      statusBar()->showMessage(msg);
    } else {
      QFileInfo fileInfo(path);
      if (filter == ZabbixCompatibleFormat) {
          mcoreData->monitor = MonitorBroker::Zabbix;
          if (fileInfo.suffix().isEmpty()) path.append(".zbx.ngrt4n.xml");
        } else if (filter == ZenossCompatibleFormat) {
          mcoreData->monitor = MonitorBroker::Zenoss;
          if (fileInfo.suffix().isEmpty()) path.append(".zns.ngrt4n.xml");
        } else {
          mcoreData->monitor = MonitorBroker::Nagios;
          if (fileInfo.suffix().isEmpty()) path.append(".nag.ngrt4n.xml");
        }
      recordData(path);
    }
}

int SvCreator::treatCloseAction(const bool& _close)
{
  int ret = 0;
  if (_close || mhasLeftUpdates) {
      bool enforceClose = _close;
      if (mhasLeftUpdates) {
          QMessageBox mbox;
          mbox.setWindowTitle(tr("Save change? - %1").arg(APP_NAME));
          mbox.setText(tr("The document has changed.\nDo you want to save the changes?"));
          mbox.setStandardButtons(QMessageBox::Yes|QMessageBox::Cancel|QMessageBox::Discard);
          switch (mbox.exec()) {
            case QMessageBox::Yes:
              save();
              break;
            case QMessageBox::Cancel:
              enforceClose = false;
              ret = 1;
              break;
            case QMessageBox::Discard:
            default:
              break;
            }
        }
      if (enforceClose) qApp->quit();
    }
  return ret;
}


void SvCreator::handleSelectedNodeChanged(void)
{
  QList<QTreeWidgetItem*> items = mtree->selectedItems();
  if (items.length())
    fillEditorFromService(*(items.begin()));
}


void SvCreator::handleTreeNodeMoved(QString _node_id)
{
  TreeNodeItemListT::iterator tnodeIt =  mcoreData->tree_items.find(_node_id);
  if (tnodeIt != mcoreData->tree_items.end()) {

      QTreeWidgetItem* tnodeP = (*tnodeIt)->parent();
      if (tnodeP) {
          NodeListT::iterator nodeIt = mcoreData->bpnodes.find(_node_id);

          if (nodeIt != mcoreData->bpnodes.end()) {
              /* Remove the node on its old parent's child list*/
              QRegExp regex ("|^" + _node_id + Parser::CHILD_SEP +
                             "|^" + _node_id + "$" +
                             "|" + Parser::CHILD_SEP  + _node_id);
              NodeListT::iterator pNodeIt = mcoreData->bpnodes.find(nodeIt->parent);
              if (pNodeIt != mcoreData->bpnodes.end()) {
                  pNodeIt->child_nodes.remove(regex);
                }

              /* Add the node on its new parent's child list*/
              nodeIt->parent = tnodeP->data(0, QTreeWidgetItem::UserType).toString();
              pNodeIt = mcoreData->bpnodes.find(nodeIt->parent);
              if (pNodeIt != mcoreData->bpnodes.end()) {
                  pNodeIt->child_nodes += (pNodeIt->child_nodes != "")?
                        Parser::CHILD_SEP + _node_id : _node_id;
                }
            }
        }
    }
}


void SvCreator::handleNodeTypeActivated(qint32 _type)
{
  NodeListT::iterator node = mcoreData->bpnodes.find(mselectedNode);
  if (node != mcoreData->bpnodes.end()) {
      if (_type == NodeType::SERVICE_NODE) {
          if (node->type == NodeType::ALARM_NODE) {
              //TODO: A bug has been reported
              node->child_nodes.clear();
              if (meditor->updateNode(node)) {
                  mcoreData->tree_items[mselectedNode]->setText(0, node->name);
                  mhasLeftUpdates = true;
                  statusBar()->showMessage(mactiveFile%"*");
                  setWindowTitle(tr("%1 Editor - %2*").arg(APP_NAME).arg(mactiveFile));
                }
            }
        } else {
          if (node->type == NodeType::SERVICE_NODE && ! node->child_nodes.isEmpty()) {
              meditor->typeField()->setCurrentIndex(0);
              utils::alert(tr("This action is not permitted for a service having sub service(s)!!!"));
            } else {
              if (meditor->updateNode(node)) {
                  mcoreData->tree_items[mselectedNode]->setText(0, node->name);
                  mhasLeftUpdates = true;
                  statusBar()->showMessage(mactiveFile%"*");
                  setWindowTitle(tr("%1 Editor - %2*").arg(APP_NAME).arg(mactiveFile));
                }
            }
        }
    }
}

void SvCreator::handleShowOnlineResources(void)
{
  QDesktopServices launcher;
  launcher.openUrl(QUrl(PKG_URL));
}

void SvCreator::handleShowAbout(void)
{
  Preferences about(muserRole, Preferences::ShowAbout);
  about.exec();
}

void SvCreator::fillEditorFromService(QTreeWidgetItem* _item)
{
  NodeListT::iterator node;
  if (utils::findNode(mcoreData, mselectedNode, node)) {
      if (meditor->updateNode(node)) {
          mcoreData->tree_items[mselectedNode]->setText(0, node->name);
          mhasLeftUpdates = true;
          statusBar()->showMessage(mactiveFile%"*");
          setWindowTitle(tr("%1 Editor - %2*").arg(APP_NAME).arg(mactiveFile));
        }
    }
  mselectedNode = _item->data(0, QTreeWidgetItem::UserType).toString();
  if (utils::findNode(mcoreData, mselectedNode, node)) meditor->setContent(node);
}


void SvCreator::handleReturnPressed(void)
{
  NodeListT::iterator node = mcoreData->bpnodes.find(mselectedNode);
  if (node != mcoreData->bpnodes.end()) {
      if (meditor->updateNode(node)) {
          mcoreData->tree_items[mselectedNode]->setText(0, node->name);
          mhasLeftUpdates = true;
          statusBar()->showMessage(mactiveFile%"*");
          setWindowTitle(tr("%1 Editor - %2*").arg(APP_NAME).arg(mactiveFile));
        }
    }
}

void SvCreator::recordData(const QString& _path)
{
  statusBar()->showMessage(tr("saving %1").arg(_path));

  QFile file(_path);
  if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
      statusBar()->showMessage(tr("Unable to open the file '%1'").arg(_path));
      return;
    }
  NodeListT::const_iterator root = mcoreData->bpnodes.find(SvNavigatorTree::RootId);
  if (root == mcoreData->bpnodes.end()) {
      file.close();
      QString msg =  tr("The hierarchy does not have root");
      utils::alert(msg);
      statusBar()->showMessage(msg);
      return;
    }

  QTextStream ofile(&file);
  ofile << "<ServiceView compat=\"2.0\" monitor=\""<< mcoreData->monitor<< "\">\n";
  recordNode(ofile,*root);
  foreach(const NodeT& service, mcoreData->bpnodes) {
      if (service.id == SvNavigatorTree::RootId || service.parent.isEmpty())
        continue;
      recordNode(ofile, service);
    }
  foreach(const NodeT& service, mcoreData->cnodes) {
      if (service.parent.isEmpty())
        continue;
      recordNode(ofile, service);
    }
  ofile << "</ServiceView>\n";
  file.close();

  mhasLeftUpdates = false;
  statusBar()->clearMessage();
  mactiveFile = utils::getAbsolutePath(_path);
  statusBar()->showMessage(tr("saved %1").arg(mactiveFile));
  setWindowTitle(tr("%1 Editor - %2").arg(APP_NAME).arg(mactiveFile));
}

void SvCreator::recordNode(QTextStream& stream, const NodeT& node)
{
  stream << "<Service id=\""<<node.id<<"\" type=\""<<node.type
         << "\" statusCalcRule=\""<<node.sev_crule<< "\" statusPropRule=\""<<node.sev_prule<< "\">\n"
         << " <Name>"<<node.name<<"</Name>\n"
         << " <Icon>"<<node.icon<<"</Icon>\n"
         << " <Description>"<<node.description<<"</Description>\n"
         << " <AlarmMsg>"<< node.alarm_msg<<"</AlarmMsg>\n"
         << " <NotificationMsg>"<<node.notification_msg<<"</NotificationMsg>\n"
         << " <SubServices>"<<node.child_nodes<<"</SubServices>\n"
         << "</Service>\n";
}

void SvCreator::resize()
{
  QSize ui_size = qApp->desktop()->screen(0)->size()* 0.80;
  QList<qint32> frames_size;
  frames_size.push_back(ui_size.width()* 0.3);
  frames_size.push_back(ui_size.width()* 0.7);
  mainSplitter->setSizes(frames_size);
  mainSplitter->resize(ui_size);
  QMainWindow::resize(ui_size);
}

void SvCreator::loadMenu(void)
{
  mmenuList["FILE"] = mmenuBar->addMenu(tr("&File"));
  msubMenus["NewFile"] = mmenuList["FILE"]->addAction("New &File"),
      msubMenus["NewFile"]->setShortcut(QKeySequence::New);
  msubMenus["Open"] = mmenuList["FILE"]->addAction(QIcon(":images/built-in/folder.png"), tr("&Open")),
      msubMenus["Open"]->setShortcut(QKeySequence::Open);
  msubMenus["Save"] = mmenuList["FILE"]->addAction(QIcon(":images/built-in/disket.png"), tr("&Save")),
      msubMenus["Save"]->setShortcut(QKeySequence::Save);
  msubMenus["SaveAs"] = mmenuList["FILE"]->addAction(QIcon(":images/built-in/disket.png"), tr("Save &As...")),
      msubMenus["SaveAs"]->setShortcut(QKeySequence::SaveAs);
  mmenuList["FILE"]->addSeparator(),
      msubMenus["Import"] = mmenuList["FILE"]->addAction(QIcon(":images/built-in/import.png"), tr("&Import Status File"));
  mmenuList["FILE"]->addSeparator(),
      msubMenus["Quit"] = mmenuList["FILE"]->addAction(tr("&Quit")),
      msubMenus["Quit"]->setShortcut(QKeySequence::Quit);
  mmenuList["EDITION"] = mmenuBar->addMenu(tr("&Edition"));
  msubMenus["NewNode"] = mmenuList["EDITION"]->addAction("&Add service"),
      mnodeContextMenu->addAction(msubMenus["NewNode"]),
      msubMenus["NewNode"]->setShortcut(QKeySequence::AddTab);
  msubMenus["CopySelected"] = mmenuList["EDITION"]->addAction("&Copy"),
      mnodeContextMenu->addAction(msubMenus["CopySelected"]),
      msubMenus["CopySelected"]->setShortcut(QKeySequence::Copy);
  msubMenus["PasteFromSelected"] = mmenuList["EDITION"]->addAction("&Paste"),
      mnodeContextMenu->addAction(msubMenus["PasteFromSelected"]),
      msubMenus["PasteFromSelected"]->setShortcut(QKeySequence::Paste);
  msubMenus["DeleteNode"] = mmenuList["EDITION"]->addAction("&Delete"),
      msubMenus["DeleteNode"]->setShortcut(QKeySequence::Delete),
      mnodeContextMenu->addAction(msubMenus["DeleteNode"]);
  mmenuList["HELP"] = mmenuBar->addMenu(tr("&Help"));
  msubMenus["ShowOnlineResources"] = mmenuList["HELP"]->addAction(tr("Online &Resources")),
      msubMenus["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
  mmenuList["HELP"]->addSeparator(),
      msubMenus["ShowAbout"] = mmenuList["HELP"]->addAction(tr("&About %1").arg(APP_NAME));
  mtoolBar->addAction(msubMenus["Save"]);
  mtoolBar->addAction(msubMenus["Open"]);
  mtoolBar->addAction(msubMenus["Import"]);
  setMenuBar(mmenuBar);
  addToolBar(mtoolBar);
}

void SvCreator::addEvents(void)
{
  connect(msubMenus["NewFile"],SIGNAL(triggered(bool)),this,SLOT(newView()));
  connect(msubMenus["NewNode"],SIGNAL(triggered(bool)),this,SLOT(newNode()));
  connect(msubMenus["CopySelected"],SIGNAL(triggered(bool)),this,SLOT(copySelected()));
  connect(msubMenus["PasteFromSelected"],SIGNAL(triggered(bool)),this,SLOT(pasteFromSelected()));
  connect(msubMenus["DeleteNode"],SIGNAL(triggered(bool)),this,SLOT(deleteNode()));
  connect(msubMenus["Open"],SIGNAL(triggered(bool)),this,SLOT(open()));
  connect(msubMenus["Save"],SIGNAL(triggered(bool)),this,SLOT(save()));
  connect(msubMenus["SaveAs"],SIGNAL(triggered(bool)),this,SLOT(saveAs()));
  connect(msubMenus["Import"],SIGNAL(triggered(bool)),this,SLOT(import()));
  connect(msubMenus["Quit"],SIGNAL(triggered(bool)),this,SLOT(treatCloseAction()));
  connect(msubMenus["ShowAbout"],SIGNAL(triggered(bool)),this,SLOT(handleShowAbout()));
  connect(msubMenus["ShowOnlineResources"],SIGNAL(triggered(bool)),this,SLOT(handleShowOnlineResources()));
  connect(meditor,SIGNAL(saveClicked()),this,SLOT(save()));
  connect(meditor,SIGNAL(closeClicked()),this,SLOT(treatCloseAction()));
  connect(meditor,SIGNAL(returnPressed()),this,SLOT(handleReturnPressed()));
  connect(meditor,SIGNAL(nodeTypeActivated(qint32)),this,SLOT(handleNodeTypeActivated(qint32)));
  connect(mtree,SIGNAL(itemSelectionChanged()),this,SLOT(handleSelectedNodeChanged()));
  connect(mtree,SIGNAL(treeNodeMoved(QString)),this,SLOT(handleTreeNodeMoved(QString)));
}
