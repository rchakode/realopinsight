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
const QString SvCreator::ZenossCompatibleFormat="Zenoss specific format(*.zns.ngrt4n.xml)"; //FIXME: test this extention

SvCreator::SvCreator(const qint32& _user_role)
  : muserRole (_user_role),
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
    mnodeContextMenu(new QMenu())
{
  mainSplitter->addWidget(mtree);
  mainSplitter->addWidget(meditor);
  setCentralWidget(mainSplitter);
  resize();
}

SvCreator::~SvCreator()
{
  delete mcoreData;
  delete mtree;
  delete meditor;
  delete mainSplitter;
  unloadMenu();
}


void SvCreator::contextMenuEvent(QContextMenuEvent *_event)
{
  QPoint pos = _event->globalPos();
  QList<QTreeWidgetItem*> selectedNodes = mtree->selectedItems();

  if (selectedNodes.length()) {
      mselectedNode = selectedNodes[0]->data(0, QTreeWidgetItem::UserType).toString();
      mnodeContextMenu->exec(pos);
    }
}

void SvCreator::closeEvent(QCloseEvent * event)
{
  treatCloseAction(true);
  QMainWindow::closeEvent(event);
}

void SvCreator::unloadMenu(void)
{
  mmenuList.clear();
  msubMenuList.clear();
  delete mnodeContextMenu;
  delete mmenuBar;
}

void SvCreator::load(const QString& _filename)
{
  loadMenu();
  addEvents();
  loadFile(_filename);
  setWindowTitle(tr("%1 Editor - %2").arg(appName).arg(mactiveFile));
  show();
}


void SvCreator::open(void)
{
  QString path = QFileDialog::getOpenFileName(this,
                                              tr("%1 | Select a configuration file").arg(appName),
                                              ".",
                                              tr("%1;;%2;;%3;;Xml files(*.xml);;All files(*)")
                                              .arg(NagiosCompatibleFormat)
                                              .arg(ZabbixCompatibleFormat)
                                              .arg(ZenossCompatibleFormat));

  if (path.length())
    loadFile(path);
}


void SvCreator::loadFile(const QString& _path)
{
  Parser parser;
  if (_path == NULL) return;
  utils::clear(*mcoreData);
  if (! parser.parseSvConfig(_path, *mcoreData)) {
      utils::alert(tr("Unable to open the file '%1'").arg(_path));
      exit(1);
    }
  mtree->update(mcoreData);
  mactiveFile = utils::getAbsolutePath(_path);
}

void SvCreator::import() {
  QString path = QFileDialog::getOpenFileName(this,
                                              tr("Select the Status File %").arg(appName),
                                              ".",
                                              tr("Data files (*.dat);;All files (*)"));
  if (path.length())
    meditor->loadStatusFile(path);
}

void SvCreator::newBusinessView(void)
{
  if (treatCloseAction(false) == 0) {
      utils::clear(*mcoreData);
      NodeT node;
      node.id = SvNavigatorTree::ROOT_ID;
      node.name = "New Business view";
      node.child_nodes.clear();
      node.criticity = MonitorBroker::CriticityUnknown;
      node.icon = GraphView::DEFAULT_ICON;
      node.type = NodeType::SERVICE_NODE;
      node.parent.clear();

      mcoreData->bpnodes[node.id] = node;
      SvNavigatorTree::addNode(mcoreData->tree_items, node);
      mtree->update(mcoreData);

      fillEditorFromService(mcoreData->tree_items[node.id]);
      mactiveFile.clear();
      mselectedNode = SvNavigatorTree::ROOT_ID;
      mhasLeftUpdates = true;
      setWindowTitle(tr("%1 Editor - unsaved document*").arg(appName));
    }
}


void SvCreator::newNode(void)
{
  static int count = 1;
  NodeListT::iterator pNode = mcoreData->bpnodes.find(mselectedNode);
  if (pNode == mcoreData->bpnodes.end() || pNode->type == NodeType::ALARM_NODE) {
      utils::alert(tr("This action not allowed on the target node"));
      return;
    }
  NodeT node;
  node.id = "ngrt4nsrv"+QDateTime::currentDateTime().toString("yyyymmddHHmmsszzz");

  node.parent = mselectedNode;
  node.name = "sub service " + QString::number(count), count ++;
  node.type = NodeType::SERVICE_NODE;
  node.criticity = MonitorBroker::CriticityUnknown;
  node.icon = GraphView::DEFAULT_ICON;
  node.child_nodes = "";

  SvNavigatorTree::addNode(mcoreData->tree_items, node, true);
  mcoreData->bpnodes[node.id] = node;
  pNode->child_nodes += (pNode->child_nodes != "")? Parser::CHILD_SEP + node.id : node.id;

  mtree->setCurrentItem(mcoreData->tree_items[node.id]);
  fillEditorFromService(mcoreData->tree_items[node.id]);
}


void SvCreator::deleteNode(void)
{
  QMessageBox msgBox;

  msgBox.setText(tr("Do you really want to delete the service and its sub services?"));
  msgBox.setWindowTitle(tr("Deleting service - %1 Editor").arg(appName));
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
                                              tr("Select the destination file | %1").arg(appName),
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
  if (!mhasLeftUpdates && _close) qApp->quit();

  QMessageBox mbox;
  mbox.setWindowTitle(tr("Save change? - %1").arg(appName));
  mbox.setText(tr("The document has been modified.\nDo you want to save your changes?"));

  bool enforceClose = _close;
  int ret = 0;
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
  if (enforceClose) qApp->quit();
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
              /* Remove the node on its old parent's child list */
              QRegExp regex ("|^" + _node_id + Parser::CHILD_SEP +
                             "|^" + _node_id + "$" +
                             "|" + Parser::CHILD_SEP  + _node_id);
              NodeListT::iterator pNodeIt = mcoreData->bpnodes.find(nodeIt->parent);
              if (pNodeIt != mcoreData->bpnodes.end()) {
                  pNodeIt->child_nodes.remove(regex);
                }

              /* Add the node on its new parent's child list */
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
                  setWindowTitle(tr("%1 Editor - %2*").arg(appName).arg(mactiveFile));
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
                  setWindowTitle(tr("%1 Editor - %2*").arg(appName).arg(mactiveFile));
                }
            }
        }
    }
}

void SvCreator::handleShowOnlineResources(void)
{
  QDesktopServices launcher;
  launcher.openUrl(QUrl(packageUrl));
}

void SvCreator::handleShowAbout(void)
{
  Preferences about(muserRole, Preferences::ShowAbout);
  about.exec();
}

void SvCreator::fillEditorFromService(QTreeWidgetItem * _item)
{
  NodeListT::iterator node;
  if (utils::findNode(mcoreData, mselectedNode, node)) {
      if (meditor->updateNode(node)) {
          mcoreData->tree_items[mselectedNode]->setText(0, node->name);
          mhasLeftUpdates = true;
          statusBar()->showMessage(mactiveFile%"*");
          setWindowTitle(tr("%1 Editor - %2*").arg(appName).arg(mactiveFile));
        }
    }
  mselectedNode = _item->data(0, QTreeWidgetItem::UserType).toString();
  if (utils::findNode(mcoreData, mselectedNode, node))
    meditor->setContent(node);
}


void SvCreator::handleReturnPressed(void)
{
  NodeListT::iterator node = mcoreData->bpnodes.find(mselectedNode);
  if (node != mcoreData->bpnodes.end()) {
      if (meditor->updateNode(node)) {
          mcoreData->tree_items[mselectedNode]->setText(0, node->name);
          mhasLeftUpdates = true;
          statusBar()->showMessage(mactiveFile%"*");
          setWindowTitle(tr("%1 Editor - %2*").arg(appName).arg(mactiveFile));
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
  NodeListT::const_iterator root = mcoreData->bpnodes.find(SvNavigatorTree::ROOT_ID);
  if (root == mcoreData->bpnodes.end()) {
      file.close();
      QString msg =  tr("The hierarchy does not have root");
      utils::alert(msg);
      statusBar()->showMessage(msg);
      return;
    }

  QTextStream ofile(&file);
  ofile << "<ServiceView compat=\"2.0\" monitor=\""<< mcoreData->monitor<< "\">\n";
  recordNode(ofile, *root);
  foreach(const NodeT& service, mcoreData->bpnodes) {
      if (service.id == SvNavigatorTree::ROOT_ID || service.parent.isEmpty())
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
  setWindowTitle(tr("%1 Editor - %2").arg(appName).arg(mactiveFile));
}

void SvCreator::recordNode(QTextStream& stream, const NodeT& node)
{
  stream << "\t<Service id=\""<<node.id<<"\" type=\""<<node.type
         << "\" statusCalcRule=\""<<node.criticity_crule<< "\" statusPropRule=\""<<node.criticity_prule<< "\">\n"
         << "\t\t<Name>"<<node.name<<"</Name>\n"
         << "\t\t<Icon>"<<node.icon<<"</Icon>\n"
         << "\t\t<Description>"<<node.description<<"</Description>\n"
         << "\t\t<AlarmMsg>"<< node.alarm_msg<<"</AlarmMsg>\n"
         << "\t\t<NotificationMsg>"<<node.notification_msg<<"</NotificationMsg>\n"
         << "\t\t<SubServices>"<<node.child_nodes<<"</SubServices>\n"
         << "\t</Service>\n";
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
  msubMenuList["NewNode"] = mnodeContextMenu->addAction("&Add service");
  msubMenuList["DeleteNode"] = mnodeContextMenu->addAction("&Delete");
  mmenuList["MENU1"] = mmenuBar->addMenu(tr("&File")),
      msubMenuList["NewFile"] = mmenuList["MENU1"]->addAction("New &File"),
      msubMenuList["Open"] = mmenuList["MENU1"]->addAction(QIcon(":images/built-in/folder.png"), tr("&Open")),
      msubMenuList["Save"] = mmenuList["MENU1"]->addAction(QIcon(":images/built-in/disket.png"), tr("&Save")),
      msubMenuList["SaveAs"] = mmenuList["MENU1"]->addAction(QIcon(":images/built-in/disket.png"), tr("Save &As..."));
  mmenuList["MENU1"]->addSeparator(),
      msubMenuList["Import"] = mmenuList["MENU1"]->addAction(QIcon(":images/built-in/import.png"), tr("&Import Status File"));
  mmenuList["MENU1"]->addSeparator(),
      msubMenuList["Quit"] = mmenuList["MENU1"]->addAction(tr("&Quit"));
  mmenuList["MENU2"] = mmenuBar->addMenu(tr("&Help")),
      msubMenuList["ShowOnlineResources"] = mmenuList["MENU2"]->addAction(tr("Online &Resources"));
  mmenuList["MENU2"]->addSeparator(),
      msubMenuList["ShowAbout"] = mmenuList["MENU2"]->addAction(tr("&About %1").arg(appName));
  msubMenuList["NewFile"]->setShortcut(QKeySequence::New);
  msubMenuList["Open"]->setShortcut(QKeySequence::Open);
  msubMenuList["Save"]->setShortcut(QKeySequence::Save);
  msubMenuList["SaveAs"]->setShortcut(QKeySequence::SaveAs);
  msubMenuList["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
  msubMenuList["Quit"]->setShortcut(QKeySequence::Quit);
  mtoolBar->addAction(msubMenuList["Save"]);
  mtoolBar->addAction(msubMenuList["Open"]);
  mtoolBar->addAction(msubMenuList["Import"]);
  setMenuBar(mmenuBar);
  addToolBar(mtoolBar);
}

void SvCreator::addEvents(void)
{
  connect(msubMenuList["NewFile"],SIGNAL(triggered(bool)),this,SLOT(newBusinessView()));
  connect(msubMenuList["NewNode"],SIGNAL(triggered(bool)),this,SLOT(newNode()));
  connect(msubMenuList["DeleteNode"],SIGNAL(triggered(bool)),this,SLOT(deleteNode()));
  connect(msubMenuList["Open"],SIGNAL(triggered(bool)),this,SLOT(open()));
  connect(msubMenuList["Save"],SIGNAL(triggered(bool)),this,SLOT(save()));
  connect(msubMenuList["SaveAs"],SIGNAL(triggered(bool)),this,SLOT(saveAs()));
  connect(msubMenuList["Import"],SIGNAL(triggered(bool)),this,SLOT(import()));
  connect(msubMenuList["Quit"],SIGNAL(triggered(bool)),this,SLOT(treatCloseAction()));
  connect(msubMenuList["ShowAbout"],SIGNAL(triggered(bool)),this,SLOT(handleShowAbout()));
  connect(msubMenuList["ShowOnlineResources"],SIGNAL(triggered(bool)),this,SLOT(handleShowOnlineResources()));
  connect(meditor,SIGNAL(saveClicked()),this,SLOT(save()));
  connect(meditor,SIGNAL(closeClicked()),this,SLOT(treatCloseAction()));
  connect(meditor,SIGNAL(returnPressed()),this,SLOT(handleReturnPressed()));
  connect(meditor,SIGNAL(nodeTypeActivated(qint32)),this,SLOT(handleNodeTypeActivated(qint32)));
  connect(mtree,SIGNAL(itemSelectionChanged()),this,SLOT(handleSelectedNodeChanged()));
  connect(mtree,SIGNAL(treeNodeMoved(QString)),this,SLOT(handleTreeNodeMoved(QString)));
}
