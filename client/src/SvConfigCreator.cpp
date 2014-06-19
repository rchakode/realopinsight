/*
* SvConfigCreator.cpp
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


#include "SvConfigCreator.hpp"
#include "GraphView.hpp"
#include "utilsCore.hpp"
#include "GuiDashboard.hpp"
#include "Auth.hpp"
#include "GuiDialogForms.hpp"
#include <fstream>

namespace {
const QString NAG_SOURCE="Nagios-based source (*.nag.ngrt4n.xml)";
const QString ZBX_SOURCE="Zabbix-based source (*.zbx.ngrt4n.xml)";
const QString ZNS_SOURCE="Zenoss-based source (*.zns.ngrt4n.xml)";
const QString MULTI_SOURCES ="Multi-sources (*.ms.ngrt4n.xml)";
}

SvCreator::SvCreator(const qint32& _userRole)
  : m_userRole (_userRole),
    m_hasLeftUpdates (false),
    m_activeConfig(""),
    m_selectedNode(""),
    m_settings(new Settings()),
    m_cdata(new CoreDataT()),
    m_mainSplitter(new QSplitter(this)),
    m_tree(new SvNavigatorTree(m_cdata, true)),
    m_editor(new ServiceEditor()),
    m_menuBar(new QMenuBar(this)),
    m_toolBar(new QToolBar("Tool Bar")),
    m_nodeContextMenu(new QMenu()),
    m_clipboardData(NULL)
{
  m_mainSplitter->addWidget(m_tree);
  m_mainSplitter->addWidget(m_editor);
  setCentralWidget(m_mainSplitter);
  resize();
  statusBar()->showMessage(tr("Open or edit a file via the File menu"));
}

SvCreator::~SvCreator()
{
  delete m_cdata;
  delete m_tree;
  delete m_editor;
  delete m_mainSplitter;
  if (m_clipboardData) delete m_clipboardData;
  unloadMenu();
}


void SvCreator::contextMenuEvent(QContextMenuEvent*_event)
{
  QPoint pos = _event->globalPos();
  QList<QTreeWidgetItem*> selectedNodes = m_tree->selectedItems();

  if (selectedNodes.length()) {
    m_selectedNode = selectedNodes[0]->data(0, QTreeWidgetItem::UserType).toString();
    m_nodeContextMenu->exec(pos);
  }
}

void SvCreator::closeEvent(QCloseEvent* event)
{
  treatCloseAction(true);
  QMainWindow::closeEvent(event);
}

void SvCreator::unloadMenu(void)
{
  m_menus.clear();
  m_subMenus.clear();
  delete m_nodeContextMenu;
  delete m_menuBar;
}

void SvCreator::load(const QString& _path)
{
  loadMenu();
  addEvents();
  loadFile(_path);
  setWindowTitle(tr("%1 Editor - %2").arg(APP_NAME).arg(m_activeConfig));
  statusBar()->showMessage(tr("Loaded."));
  show();
}


void SvCreator::open(void)
{
  QString path;
  path = QFileDialog::getOpenFileName(this,
                                      tr("%1 | Select target file").arg(APP_NAME),
                                      ".",
                                      tr("%1;;%2;;%3;;%4;;Xml files(*.xml);;All files(*)").arg(NAG_SOURCE,
                                                                                               ZBX_SOURCE,
                                                                                               ZNS_SOURCE,
                                                                                               MULTI_SOURCES));
  if (! path.isNull() && ! path.isEmpty())
    loadFile(path);
}


void SvCreator::loadFile(const QString& _path)
{
  if (_path == NULL) {
    newView();
  } else {
    ngrt4n::clear(*m_cdata);
    Parser parser(_path, m_cdata);
    if (! parser.process(false)) {
      ngrt4n::alert(parser.lastErrorMsg());
      exit(1);
    } else {
      m_root = m_cdata->bpnodes.find(ngrt4n::ROOT_ID);
      m_editor->fillFormWithNodeContent(*m_root);
      m_tree->build();
      fillEditorFromService(m_tree->rootItem());
      m_activeConfig = ngrt4n::getAbsolutePath(_path);
      setWindowTitle(tr("%1 Editor - %2").arg(APP_NAME).arg(m_activeConfig));
    }
  }
}

void SvCreator::fetchSourceList(int type, QMap<QString, SourceT>& sourceInfos)
{
  GuiPreferences preferences(Auth::OpUserRole, Preferences::NoForm);
  preferences.initSourceStatesFromData();
  SourceT srcInfo;
  QStringList sourceList;
  for (int i = 0; i< MAX_SRCS; ++i) {
    if (preferences.loadSource(i, srcInfo)) {
      if (srcInfo.mon_type == type) {
        sourceList.push_back(srcInfo.id);
        sourceInfos.insert(srcInfo.id, srcInfo);
      }
    }
  }
}




void SvCreator::importNagiosChecks(void)
{
  QMap<QString, SourceT> sourceInfos;
  fetchSourceList(ngrt4n::Zabbix, sourceInfos);
  CheckImportationSettingsForm importationSettingForm(sourceInfos.keys(), true);
  if (importationSettingForm.exec() == QDialog::Accepted) {
    QString srcId = importationSettingForm.selectedSource();
    QString path = importationSettingForm.selectedStatusFile();
    SourceT srcInfo = sourceInfos[srcId];

    if (! path.isNull() && !path.isEmpty()) {
      statusBar()->showMessage(tr("Loading checks from %1:%2...").arg(srcId, path));
      ChecksT checks;
      int retcode = parseStatusFile(path, checks);
      treatCheckLoadResults(retcode, srcInfo.id, checks, tr("Error while parsing the file"));
    } else {
      statusBar()->showMessage(tr("No file selected"));
    }
  }
  //  QString path = QFileDialog::getOpenFileName(this,
  //                                              tr("Select a status file | %1").arg(APP_NAME),
  //                                              ".",
  //                                              tr("Data files (*.dat);;All files (*)"));
}

void SvCreator::importZabbixTriggers(void)
{
  QMap<QString, SourceT> sourceInfos;
  fetchSourceList(ngrt4n::Zabbix, sourceInfos);
  CheckImportationSettingsForm importationSettingForm(sourceInfos.keys(), false);
  if (importationSettingForm.exec() == QDialog::Accepted) {
    QString srcId = importationSettingForm.selectedSource();
    QString host = importationSettingForm.selectedHost();
    SourceT srcInfo = sourceInfos[srcId];

    statusBar()->showMessage(tr("Loading triggers from %1:%2...").arg(srcInfo.id, srcInfo.mon_url));

    ChecksT checks;
    ZbxHelper handler(srcInfo.mon_url);
    int retcode = handler.openSession(srcInfo);
    if (retcode == 0) {
      retcode = handler.loadChecks(srcInfo, host, checks);
    }
    treatCheckLoadResults(retcode, srcId, checks, handler.lastError());
  }
}

void SvCreator::importZenossComponents(void)
{
  QMap<QString, SourceT> sourceInfos;
  fetchSourceList(ngrt4n::Zenoss, sourceInfos);
  CheckImportationSettingsForm importationSettingForm(sourceInfos.keys(), false);
  if (importationSettingForm.exec() == QDialog::Accepted) {
    QString srcId = importationSettingForm.selectedSource();
    QString host = importationSettingForm.selectedHost();
    SourceT srcInfo = sourceInfos[srcId];

    statusBar()->showMessage(tr("Loading components from %1:%2...").arg(srcInfo.id, srcInfo.mon_url));

    ChecksT checks;
    ZnsHelper handler(srcInfo.mon_url);
    int retcode = handler.openSession(srcInfo);
    if (retcode == 0) {
      retcode = handler.loadChecks(srcInfo, host, checks);
    }
    treatCheckLoadResults(retcode, srcId, checks, handler.lastError());
  }
}

void SvCreator::treatCheckLoadResults(int retCode, const QString& srcId, const ChecksT& checks, const QString& msg)
{
  if (retCode != 0) {
    statusBar()->showMessage(msg);
    statusBar()->setStyleSheet("background: red;");
  } else {
    m_editor->loadChecks(checks, srcId);
    statusBar()->showMessage(tr("%1 triggers imported").arg(checks.size()));
    statusBar()->setStyleSheet("background: transparent;");
  }
}

void SvCreator::newView(void)
{
  if (treatCloseAction(false) == 0) {
    ngrt4n::clear(*m_cdata);
    m_tree->resetData();
    NodeT* node = createNode(ngrt4n::ROOT_ID, tr("New View"), "");
    m_root = m_cdata->bpnodes.insert(node->id, *node);
    m_tree->addNode(*node);
    m_tree->update();
    m_editor->fillFormWithNodeContent(*node);
    m_selectedNode = node->id;
    m_hasLeftUpdates = true;
    m_activeConfig.clear();
    setWindowTitle(tr("%1 Editor - unsaved document*").arg(APP_NAME));
  }
}


void SvCreator::newNode(void)
{
  static int count = 1;
  NodeT* node = createNode(ngrt4n::genNodeId(), tr("sub service %1").arg(QString::number(count)), m_selectedNode);
  insertFromSelected(*node);
  ++count;
}

NodeT* SvCreator::createNode(const QString& id,
                             const QString& label,
                             const QString& parent)
{
  NodeT* node = new NodeT;
  if (!node) {
    ngrt4n::alert(tr("Out of memory. the application will exit"));
    exit(1);
  }
  node->id = id;
  node->name = label;
  node->parent = parent;
  node->type = NodeType::ServiceNode;
  node->severity = ngrt4n::Unknown;
  node->sev_crule = CalcRules::HighCriticity;
  node->sev_prule = PropRules::Unchanged;
  node->icon = ngrt4n::DEFAULT_ICON;
  node->child_nodes = QString();
  return node;
}

void SvCreator::insertFromSelected(const NodeT& node)
{
  NodeListT::iterator pnode = m_cdata->bpnodes.find(m_selectedNode);
  if (pnode == m_cdata->bpnodes.end() ||
      pnode->type == NodeType::AlarmNode) {
    ngrt4n::alert(tr("This action not allowed on the target node"));
    return;
  }
  pnode->child_nodes += (!(pnode->child_nodes).isEmpty())? QString::fromStdString(ngrt4n::CHILD_SEP)%node.id : node.id;
  QTreeWidgetItem* lastItem = m_tree->addNode(node, true);
  m_cdata->bpnodes.insert(node.id, node);
  m_tree->setCurrentItem(lastItem);
  fillEditorFromService(lastItem);
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
    deleteNode(m_selectedNode);
    break;
  default:
    break;
  }
}

void SvCreator::deleteNode(const QString& _nodeId)
{
  NodeListT::iterator node;
  if (!ngrt4n::findNode(m_cdata, _nodeId, node))
    return;

  QString sep(ngrt4n::CHILD_SEP.c_str());

  if (node->type == NodeType::ServiceNode && node->child_nodes != "") {
    Q_FOREACH(const QString& checkId, node->child_nodes.split(sep)) {
      deleteNode(checkId);
    }
  }

  QTreeWidgetItem* item = m_tree->findNodeItem(_nodeId);
  QTreeWidgetItem* pItem = m_tree->findNodeItem(node->parent);
  if (pItem && item) {
    QRegExp regex("|^" + _nodeId + sep +
                  "|^" + _nodeId + "$" +
                  "|" + sep  + _nodeId);

    NodeListT::iterator pNode = m_cdata->bpnodes.find(node->parent);
    if (pNode != m_cdata->bpnodes.end()) {
      pNode->child_nodes.remove(regex);
    }
    if (node->type == NodeType::AlarmNode) {
      m_cdata->cnodes.remove(_nodeId);
    } else {
      m_cdata->bpnodes.remove(_nodeId);
    }
    m_tree->removeNodeItem(_nodeId);
    QTreeWidgetItem* obsolete = NULL;
    if ((obsolete = pItem->takeChild(pItem->indexOfChild(item))))
      delete obsolete;
  }
}


void SvCreator::copySelected(void)
{
  NodeListIteratorT node;
  if (ngrt4n::findNode(m_cdata, m_selectedNode, node)) {
    if (!m_clipboardData) m_clipboardData = new NodeT;
    *m_clipboardData =*node;
    m_clipboardData->name+=" (Copy)";
    m_clipboardData->child_nodes.clear();
  }
}

void SvCreator::pasteFromSelected(void)
{
  if (m_clipboardData) {
    m_clipboardData->id = ngrt4n::genNodeId();
    m_clipboardData->parent = m_selectedNode;
    insertFromSelected(*m_clipboardData);
  } else {
    ngrt4n::alert(tr("There is no data in the clipboard!"));
  }
}


void SvCreator::save(void)
{
  if (! m_selectedNode.isEmpty()) {
    fillEditorFromService(m_tree->findNodeItem(m_selectedNode));
  }
  if (m_activeConfig.isEmpty()) {
    saveAs();
  } else {
    recordData(m_activeConfig);
  }
}

void SvCreator::saveAs(void)
{
  QString filter;
  QString path = QFileDialog::getSaveFileName(this,
                                              tr("Select the destination file | %1").arg(APP_NAME),
                                              m_activeConfig,
                                              QString("%1;;%2;;%3;;%4;;").arg(NAG_SOURCE,
                                                                              ZBX_SOURCE,
                                                                              ZNS_SOURCE,
                                                                              MULTI_SOURCES),
                                              &filter);

  if (path.isNull()) {
    QString msg = tr("The path is not valid!");
    ngrt4n::alert(msg);
    statusBar()->showMessage(msg);
  } else {
    QFileInfo fileInfo(path);
    if (filter == ZBX_SOURCE) {
      m_cdata->monitor = ngrt4n::Zabbix;
      if (fileInfo.suffix().isEmpty()) path.append(".zbx.ngrt4n.xml");
    } else if (filter == ZNS_SOURCE) {
      m_cdata->monitor = ngrt4n::Zenoss;
      if (fileInfo.suffix().isEmpty()) path.append(".zns.ngrt4n.xml");
    } else if (filter == NAG_SOURCE){
      m_cdata->monitor = ngrt4n::Nagios;
      if (fileInfo.suffix().isEmpty()) path.append(".nag.ngrt4n.xml");
    } else {
      m_cdata->monitor = ngrt4n::Auto;
      if (fileInfo.suffix().isEmpty()) path.append(".ms.ngrt4n.xml");
    }
    recordData(path);
  }
}

int SvCreator::treatCloseAction(const bool& _close)
{
  int ret = 0;
  if (_close || m_hasLeftUpdates) {
    bool enforceClose = _close;
    if (m_hasLeftUpdates) {
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
  QList<QTreeWidgetItem*> items = m_tree->selectedItems();
  if (items.length()) {
    fillEditorFromService(*(items.begin()));
  } else {
    fillEditorFromService(m_tree->rootItem());
  }
}


void SvCreator::handleTreeNodeMoved(QString _node_id)
{
  QString sep(ngrt4n::CHILD_SEP.c_str());

  QTreeWidgetItem* item =  m_tree->findNodeItem(_node_id);
  if (item) {

    QTreeWidgetItem* tnodeP = item->parent();
    if (tnodeP) {
      NodeListT::iterator nodeIt = m_cdata->bpnodes.find(_node_id);

      if (nodeIt != m_cdata->bpnodes.end()) {
        /* Remove the node on its old parent's child list*/
        QRegExp regex ("|^" + _node_id + sep +
                       "|^" + _node_id + "$" +
                       "|" + sep + _node_id);
        NodeListT::iterator pNodeIt = m_cdata->bpnodes.find(nodeIt->parent);
        if (pNodeIt != m_cdata->bpnodes.end()) {
          pNodeIt->child_nodes.remove(regex);
        }

        /* Add the node on its new parent's child list*/
        nodeIt->parent = tnodeP->data(0, QTreeWidgetItem::UserType).toString();
        pNodeIt = m_cdata->bpnodes.find(nodeIt->parent);
        if (pNodeIt != m_cdata->bpnodes.end()) {
          pNodeIt->child_nodes += (pNodeIt->child_nodes != "")? sep + _node_id : _node_id;
        }
      }
    }
  }
}


void SvCreator::handleNodeTypeActivated(qint32 _type)
{
  NodeListT::iterator node = m_cdata->bpnodes.find(m_selectedNode);
  if (node != m_cdata->bpnodes.end()) {
    if (_type == NodeType::ServiceNode) {
      if (node->type == NodeType::AlarmNode) {
        //TODO: A bug has been reported
        node->child_nodes.clear();
        if (m_editor->updateNodeContent(node)) {
          m_tree->findNodeItem(m_selectedNode)->setText(0, node->name);
          m_hasLeftUpdates = true;
          statusBar()->showMessage(m_activeConfig%"*");
          setWindowTitle(tr("%1 Editor - %2*").arg(APP_NAME).arg(m_activeConfig));
        }
      }
    } else {
      if (node->type == NodeType::ServiceNode && ! node->child_nodes.isEmpty()) {
        m_editor->typeField()->setCurrentIndex(0);
        ngrt4n::alert(tr("This action is not permitted for a service having sub service(s)!!!"));
      } else {
        if (m_editor->updateNodeContent(node)) {
          m_tree->findNodeItem(m_selectedNode)->setText(0, node->name);
          m_hasLeftUpdates = true;
          statusBar()->showMessage(m_activeConfig%"*");
          setWindowTitle(tr("%1 Editor - %2*").arg(APP_NAME).arg(m_activeConfig));
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
  GuiPreferences about(m_userRole, Preferences::ShowAbout);
  about.exec();
}

void SvCreator::fillEditorFromService(QTreeWidgetItem* _item)
{
  NodeListT::iterator node;
  if (ngrt4n::findNode(m_cdata, m_selectedNode, node)) {
    if (m_editor->updateNodeContent(node)) {
      m_tree->findNodeItem(m_selectedNode)->setText(0, node->name);
      m_hasLeftUpdates = true;
      statusBar()->showMessage(m_activeConfig%"*");
      setWindowTitle(tr("%1 Editor - %2*").arg(APP_NAME).arg(m_activeConfig));
    }
  }
  m_selectedNode = _item->data(0, QTreeWidgetItem::UserType).toString();
  if (ngrt4n::findNode(m_cdata, m_selectedNode, node)) m_editor->fillFormWithNodeContent(node);
}


void SvCreator::handleReturnPressed(void)
{
  NodeListT::iterator node = m_cdata->bpnodes.find(m_selectedNode);
  if (node != m_cdata->bpnodes.end()) {
    if (m_editor->updateNodeContent(node)) {
      m_tree->findNodeItem(m_selectedNode)->setText(0, node->name);
      m_hasLeftUpdates = true;
      statusBar()->showMessage(m_activeConfig%"*");
      setWindowTitle(tr("%1 Editor - %2*").arg(APP_NAME).arg(m_activeConfig));
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
  m_root = m_cdata->bpnodes.find(ngrt4n::ROOT_ID);
  if (m_root == m_cdata->bpnodes.end()) {
    file.close();
    QString msg =  tr("The hierarchy does not have root");
    ngrt4n::alert(msg);
    statusBar()->showMessage(msg);
  } else {
    QTextStream ofile(&file);
    ofile << "<?xml version=\"1.0\"?>\n"
             "<ServiceView compat=\"2.0\" monitor=\""<< m_cdata->monitor<< "\">\n";
    recordNode(ofile,*m_root);
    Q_FOREACH(const NodeT& service, m_cdata->bpnodes) {
      if (service.id == ngrt4n::ROOT_ID || service.parent.isEmpty())
        continue;
      recordNode(ofile, service);
    }
    Q_FOREACH(const NodeT& service, m_cdata->cnodes) {
      if (service.parent.isEmpty())
        continue;
      recordNode(ofile, service);
    }
    ofile << "</ServiceView>\n";
    file.close();

    m_hasLeftUpdates = false;
    statusBar()->clearMessage();
    m_activeConfig = ngrt4n::getAbsolutePath(_path);
    statusBar()->showMessage(tr("saved %1").arg(m_activeConfig));
    setWindowTitle(tr("%1 Editor - %2").arg(APP_NAME).arg(m_activeConfig));
  }
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
  m_mainSplitter->setSizes(frames_size);
  m_mainSplitter->resize(ui_size);
  QMainWindow::resize(ui_size);
}

void SvCreator::loadMenu(void)
{
  m_menus["FILE"] = m_menuBar->addMenu(tr("&File"));
  m_subMenus["NewFile"] = m_menus["FILE"]->addAction("New &File"),
      m_subMenus["NewFile"]->setShortcut(QKeySequence::New);
  m_subMenus["Open"] = m_menus["FILE"]->addAction(QIcon(":images/built-in/folder.png"), tr("&Open")),
      m_subMenus["Open"]->setShortcut(QKeySequence::Open);
  m_subMenus["Save"] = m_menus["FILE"]->addAction(QIcon(":images/built-in/disket.png"), tr("&Save")),
      m_subMenus["Save"]->setShortcut(QKeySequence::Save);
  m_subMenus["SaveAs"] = m_menus["FILE"]->addAction(QIcon(":images/built-in/disket.png"), tr("Save &As...")),
      m_subMenus["SaveAs"]->setShortcut(QKeySequence::SaveAs);
  m_menus["FILE"]->addSeparator(),
      m_subMenus["ImportNagiosChecks"] = m_menus["FILE"]->addAction(QIcon(":images/built-in/import-nagios.png"), tr("Import Na&gios Checks")),
      m_subMenus["ImportZabbixTriggers"] = m_menus["FILE"]->addAction(QIcon(":images/built-in/import-zabbix.png"), tr("Import Za&bbix Triggers")),
      m_subMenus["ImportZenossComponents"] = m_menus["FILE"]->addAction(QIcon(":images/built-in/import-zenoss.png"), tr("Import Z&enoss Components"));
  m_menus["FILE"]->addSeparator(),
      m_subMenus["Quit"] = m_menus["FILE"]->addAction(tr("&Quit")),
      m_subMenus["Quit"]->setShortcut(QKeySequence::Quit);
  m_menus["EDITION"] = m_menuBar->addMenu(tr("&Edition"));
  m_subMenus["NewNode"] = m_menus["EDITION"]->addAction(tr("&Add sub service")),
      m_nodeContextMenu->addAction(m_subMenus["NewNode"]),
      m_subMenus["NewNode"]->setShortcut(QKeySequence::AddTab);
  m_subMenus["CopySelected"] = m_menus["EDITION"]->addAction(tr("&Copy")),
      m_nodeContextMenu->addAction(m_subMenus["CopySelected"]),
      m_subMenus["CopySelected"]->setShortcut(QKeySequence::Copy);
  m_subMenus["PasteFromSelected"] = m_menus["EDITION"]->addAction(tr("&Paste")),
      m_nodeContextMenu->addAction(m_subMenus["PasteFromSelected"]),
      m_subMenus["PasteFromSelected"]->setShortcut(QKeySequence::Paste);
  m_subMenus["DeleteNode"] = m_menus["EDITION"]->addAction(tr("&Delete")),
      m_subMenus["DeleteNode"]->setShortcut(QKeySequence::Delete),
      m_nodeContextMenu->addAction(m_subMenus["DeleteNode"]);
  m_menus["HELP"] = m_menuBar->addMenu(tr("&Help"));
  m_subMenus["ShowOnlineResources"] = m_menus["HELP"]->addAction(tr("Online &Resources")),
      m_subMenus["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
  m_menus["HELP"]->addSeparator(),
      m_subMenus["ShowAbout"] = m_menus["HELP"]->addAction(tr("&About %1").arg(APP_NAME));
  m_toolBar->addAction(m_subMenus["Save"]);
  m_toolBar->addAction(m_subMenus["Open"]);
  m_toolBar->addAction(m_subMenus["ImportNagiosChecks"]);
  m_toolBar->addAction(m_subMenus["ImportZabbixTriggers"]);
  m_toolBar->addAction(m_subMenus["ImportZenossComponents"]);
  setMenuBar(m_menuBar);
  addToolBar(m_toolBar);
}

int SvCreator::parseStatusFile(const QString& path, ChecksT& checks)
{
  std::ifstream fileStream;
  fileStream.open(path.toStdString(), std::ios_base::in);
  if (! fileStream.good() ) {
    qDebug() << tr("ERROR: Unable to open the file %1").arg(path);
    return -1;
  }

  std::string line;
  while (getline(fileStream, line), ! fileStream.eof()) {

    if(line.find("#") != std::string::npos ) continue;

    if( line.find("hoststatus") == std::string::npos
        && line.find("servicestatus") == std::string::npos )
      continue;

    CheckT check;
    check.status = -1;
    while (getline(fileStream, line), ! fileStream.eof()) {
      size_t pos;
      if (pos = line.find("}"), pos != std::string::npos ) break;
      if (pos = line.find("="), pos == std::string::npos) continue;
      QString param = QString(line.substr(0, pos).c_str()).trimmed();
      std::string value = QString(line.substr(pos+1, std::string::npos).c_str()).trimmed().toStdString();
      if (param == "host_name") {
        check.host = check.id = QString(line.substr(pos+1).c_str()).trimmed().toStdString();
      } else if(param == "service_description") {
        check.id += "/" + value;
      } else if(param == "check_command") {
        check.check_command = value;
      } else if(param == "current_state") {
        check.status = atoi(value.c_str());
      } else if(param == "last_state_change") {
        check.last_state_change = value;
      } else if(param == "plugin_output") {
        check.alarm_msg = value;
      }
    }
    checks[check.id] = check;
  }
  fileStream.close();

  return 0;
}

void SvCreator::addEvents(void)
{
  connect(m_subMenus["NewFile"],SIGNAL(triggered(bool)),this,SLOT(newView()));
  connect(m_subMenus["NewNode"],SIGNAL(triggered(bool)),this,SLOT(newNode()));
  connect(m_subMenus["CopySelected"],SIGNAL(triggered(bool)),this,SLOT(copySelected()));
  connect(m_subMenus["PasteFromSelected"],SIGNAL(triggered(bool)),this,SLOT(pasteFromSelected()));
  connect(m_subMenus["DeleteNode"],SIGNAL(triggered(bool)),this,SLOT(deleteNode()));
  connect(m_subMenus["Open"],SIGNAL(triggered(bool)),this,SLOT(open()));
  connect(m_subMenus["Save"],SIGNAL(triggered(bool)),this,SLOT(save()));
  connect(m_subMenus["SaveAs"],SIGNAL(triggered(bool)),this,SLOT(saveAs()));
  connect(m_subMenus["ImportNagiosChecks"],SIGNAL(triggered(bool)),this,SLOT(importNagiosChecks()));
  connect(m_subMenus["ImportZabbixTriggers"],SIGNAL(triggered(bool)),this,SLOT(importZabbixTriggers()));
  connect(m_subMenus["ImportZenossComponents"],SIGNAL(triggered(bool)),this,SLOT(importZenossComponents()));
  connect(m_subMenus["Quit"],SIGNAL(triggered(bool)),this,SLOT(treatCloseAction()));
  connect(m_subMenus["ShowAbout"],SIGNAL(triggered(bool)),this,SLOT(handleShowAbout()));
  connect(m_subMenus["ShowOnlineResources"],SIGNAL(triggered(bool)),this,SLOT(handleShowOnlineResources()));
  connect(m_editor,SIGNAL(saveClicked()),this,SLOT(save()));
  connect(m_editor,SIGNAL(closeClicked()),this,SLOT(treatCloseAction()));
  connect(m_editor,SIGNAL(returnPressed()),this,SLOT(handleReturnPressed()));
  connect(m_editor,SIGNAL(nodeTypeActivated(qint32)),this,SLOT(handleNodeTypeActivated(qint32)));
  connect(m_tree,SIGNAL(itemSelectionChanged()),this,SLOT(handleSelectedNodeChanged()));
  connect(m_tree,SIGNAL(treeNodeMoved(QString)),this,SLOT(handleTreeNodeMoved(QString)));
}
