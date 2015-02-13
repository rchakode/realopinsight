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


#include <fstream>
#include "SvConfigCreator.hpp"
#include "GraphView.hpp"
#include "utilsCore.hpp"
#include "Auth.hpp"
#include "GuiDialogForms.hpp"
#include "LsHelper.hpp"
#include "ThresholdHelper.hpp"
#include "GuiPreferences.hpp"
#include "ZbxHelper.hpp"
#include "ZnsHelper.hpp"
#include "PandoraHelper.hpp"

namespace {
const QString NAG_SOURCE     = "Nagios description file (*.nag.ngrt4n.xml)";
const QString ZBX_SOURCE     = "Zabbix description file (*.zbx.ngrt4n.xml)";
const QString ZNS_SOURCE     = "Zenoss description file (*.zns.ngrt4n.xml)";
const QString PANDORA_SOURCE = "Pandora FMS description file (*.pfms.ngrt4n.xml)";
const QString MULTI_SOURCES  = "Multi-source description file (*.ms.ngrt4n.xml)";
const QString CHILD_SEPERATOR(ngrt4n::CHILD_SEP.c_str());
const QString FILE_FILTER =
    QString("%1;;%2;;%3;;%4;;%5;;Xml files(*.xml);;All files(*)")
    .arg(NAG_SOURCE, ZBX_SOURCE, ZNS_SOURCE, PANDORA_SOURCE, MULTI_SOURCES);
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
  showStatusMsg(tr("Open or edit a file via the File menu"), false);
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


void SvCreator::addEvents(void)
{
  connect(m_subMenus["NewFile"],SIGNAL(triggered(bool)),this,SLOT(newView()));
  connect(m_subMenus["NewNode"],SIGNAL(triggered(bool)),this,SLOT(newNode()));
  connect(m_subMenus["CopySelected"],SIGNAL(triggered(bool)),this,SLOT(copySelected()));
  connect(m_subMenus["PasteFromSelected"],SIGNAL(triggered(bool)),this,SLOT(pasteFromSelected()));
  connect(m_subMenus["DeleteNode"],SIGNAL(triggered(bool)),this,SLOT(deleteNode()));
  connect(m_subMenus["Open"],SIGNAL(triggered(bool)),this,SLOT(open()));
  connect(m_subMenus["Save"],SIGNAL(triggered(bool)),this,SLOT(save()));
  connect(m_subMenus["SaveAs"], SIGNAL(triggered(bool)), this, SLOT(saveAs()));
  connect(m_subMenus["ImportNagiosChecks"],SIGNAL(triggered(bool)),this,SLOT(importNagiosChecks()));
  connect(m_subMenus["ImportNagiosLivestatusChecks"],SIGNAL(triggered(bool)),this,SLOT(importNagiosLivestatusChecks()));
  connect(m_subMenus["ImportNagiosBPIConf"],SIGNAL(triggered(bool)),this,SLOT(importNagiosBPIConfig()));
  connect(m_subMenus["ImportZabbixTriggers"],SIGNAL(triggered(bool)),this,SLOT(importZabbixTriggersAsDataPoints()));
  connect(m_subMenus["ImportZabbixITServices"],SIGNAL(triggered(bool)),this,SLOT(importZabbixITServicesAsBusinessViews()));
  connect(m_subMenus["AutomaticImportZabbixTriggers"],SIGNAL(triggered(bool)),this,SLOT(importZabbixTriggersAsBundleBusinessView()));
  connect(m_subMenus["ImportZenossComponents"],SIGNAL(triggered(bool)),this,SLOT(importZenossComponents()));
  connect(m_subMenus["ImportPandoraModules"],SIGNAL(triggered(bool)),this,SLOT(importPandoraModules()));
  connect(m_subMenus["Quit"],SIGNAL(triggered(bool)),this,SLOT(treatCloseAction()));
  connect(m_subMenus["ShowAbout"],SIGNAL(triggered(bool)),this,SLOT(handleShowAbout()));
  connect(m_subMenus["ShowOnlineResources"],SIGNAL(triggered(bool)),this,SLOT(handleShowOnlineResources()));
  connect(m_editor,SIGNAL(saveClicked()),this,SLOT(save()));
  connect(m_editor,SIGNAL(closeClicked()),this,SLOT(treatCloseAction()));
  connect(m_editor,SIGNAL(returnPressed()),this,SLOT(handleReturnPressed()));
  connect(m_editor,SIGNAL(nodeTypeActivated(qint32)),this,SLOT(handleNodeTypeActivated(qint32)));
  connect(m_editor,SIGNAL(errorOccurred(QString)),this, SLOT(handleErrorOccurred(QString)));
  connect(m_tree,SIGNAL(itemSelectionChanged()),this,SLOT(handleSelectedNodeChanged()));
  connect(m_tree,SIGNAL(treeNodeMoved(QString)),this,SLOT(handleTreeNodeMoved(QString)));
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
  showStatusMsg(tr("Loaded."), false);
  show();
}


void SvCreator::open(void)
{
  QString path;
  path = QFileDialog::getOpenFileName(this,
                                      tr("%1 | Select target file").arg(APP_NAME),
                                      ".",
                                      FILE_FILTER);
  if (isValidPath(path)) loadFile(path);
}


void SvCreator::loadFile(const QString& _path)
{
  if (_path == NULL) {
    newView();
  } else {
    Parser parser(_path, m_cdata);
    if (! parser.process(false)) {
      ngrt4n::alert(parser.lastErrorMsg());
      exit(1);
    } else {
      m_activeConfig = ngrt4n::getAbsolutePath(_path);
      refreshUIWidgets();
    }
  }
}

void SvCreator::refreshUIWidgets(void)
{
  m_hasLeftUpdates = true;

  m_root = m_cdata->bpnodes.find(ngrt4n::ROOT_ID);
  m_editor->fillInEditorWithContent(*m_root);
  m_tree->build();
  fillEditorFromService(m_tree->rootItem());
  updateWindowTitle("*");
}

void SvCreator::fetchSourceList(int type, QMap<QString, SourceT>& sourceInfos)
{
  GuiPreferences preferences(Auth::OpUserRole, Preferences::NoForm);
  preferences.updateSourceStates();
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
  fetchSourceList(ngrt4n::Nagios, sourceInfos);
  CheckImportationSettingsForm importationSettingForm(sourceInfos.keys(), true);
  if (importationSettingForm.exec() == QDialog::Accepted) {
    QString srcId = importationSettingForm.selectedSource();
    QString path = importationSettingForm.selectedFile();
    SourceT srcInfo = sourceInfos[srcId];

    if (! isValidPath(path) ) {
      showStatusMsg(tr("Loading checks from %1:%2...").arg(srcId, path), false);
      ChecksT checks;
      int retcode = parseStatusFile(path, checks);
      processCheckLoadResults(retcode, srcInfo.id, checks, tr("Error while parsing the file"));
    } else {
      showStatusMsg(tr("No file selected"), true);
    }
  }
}

void SvCreator::importNagiosLivestatusChecks(void)
{
  QMap<QString, SourceT> sourceInfos;
  fetchSourceList(ngrt4n::Nagios, sourceInfos);
  CheckImportationSettingsForm importationSettingForm(sourceInfos.keys(), false);
  if (importationSettingForm.exec() == QDialog::Accepted) {
    QString srcId = importationSettingForm.selectedSource();
    QString host = importationSettingForm.filter();
    SourceT srcInfo = sourceInfos[srcId];

    showStatusMsg(tr("Loading checks from %1:%2:%3...")
                  .arg(srcInfo.id, srcInfo.ls_addr, QString::number(srcInfo.ls_port)), true);

    ChecksT checks;
    LsHelper handler(srcInfo.ls_addr, srcInfo.ls_port);
    int retcode = handler.setupSocket();
    if (retcode == 0) {
      retcode = handler.loadChecks(host, checks);
    }
    processCheckLoadResults(retcode, srcId, checks, handler.lastError());
  }
}


void SvCreator::importNagiosBPIConfig(void)
{
  QMap<QString, SourceT> sourceInfos;
  fetchSourceList(ngrt4n::Nagios, sourceInfos);
  CheckImportationSettingsForm importationSettingForm(sourceInfos.keys(), true);
  if (importationSettingForm.exec() != QDialog::Accepted) {
    return;
  }


  QFile file(importationSettingForm.selectedFile());
  if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    showStatusMsg(tr("Cannot open file: %1").arg(importationSettingForm.selectedFile()), true);
    return;
  }

  QString sourceId = importationSettingForm.selectedSource();

  bool parsingFailed = false;
  QString line;
  int lineIndex = 0;
  QTextStream streamReader(&file);
  NodeT rootService;
  rootService.id = ngrt4n::ROOT_ID;
  rootService.name = tr("Nagios BPI Services");

  ngrt4n::clearCoreData(*m_cdata);
  while (line = streamReader.readLine(), ! line.isNull()) {

    ++lineIndex;

    line = line.trimmed();
    if ( line.startsWith("#")
         || line.isEmpty()
         || ! line.startsWith("define")) continue;

    if (! line.endsWith("{")) {
      showStatusMsg(tr("Group definition must end with '{' at line %1").arg(QString::number(lineIndex)), true);
      parsingFailed = true;
      break;
    }

    QStringList fields = line.mid(0, line.size() - 1).trimmed().split(" ");
    if (fields.size() != 2) {
      showStatusMsg(tr("Bad group definition at line %1").arg(QString::number(lineIndex)), true);
      parsingFailed = true;
      break;
    }

    QString groudId = fields[1];

    NodeListT::Iterator currentNode = m_cdata->bpnodes.find(groudId);
    if (currentNode == m_cdata->bpnodes.end()) {
      currentNode = m_cdata->bpnodes.insert(groudId, createNode(groudId, "", ""));
    }

    currentNode->sev_crule = CalcRules::Worst;

    // now parse group config
    int groupMembersCount = 0;
    float warningThreshold  = 0;
    float criticalThreshold = 0;
    while (line = streamReader.readLine(), ! line.isNull()) {

      ++lineIndex;

      line = line.trimmed();

      if (line.isEmpty()) continue;
      if (line == "}") break;

      fields = line.split("=");
      if (fields.size() != 2) {
        showStatusMsg(tr("Bad group attribute definition at line %1").arg(QString::number(lineIndex)), true);
        parsingFailed = true;
        break;
      }

      if (fields[0] == "title") {
        currentNode->name = fields[1];
      } else if (fields[0] == "desc") {
        currentNode->description = fields[1];
      } else if (fields[0] == "members") {
        bool hasCluster = false;
        QString members = fields[1];
        groupMembersCount = extractNagiosBPIGroupMembers(currentNode->id, sourceId, members,
                                                         m_cdata->bpnodes, m_cdata->cnodes, currentNode->child_nodes,
                                                         hasCluster);
        if (groupMembersCount < 0) {
          parsingFailed = true;
          break;
        } else {
          currentNode->sev_crule = hasCluster ? CalcRules::Average : CalcRules::Worst;
        }

      } else if (fields[0] == "warning_threshold") {
        warningThreshold = fields[1].toFloat();
      } else if (fields[0] == "critical_threshold") {
        criticalThreshold = fields[1].toFloat();
      } else if (fields[0] == "priority") {
        /// not applicable
      }
    }


    if (parsingFailed) {
      break;
    } else {
      if (groupMembersCount > 0) {
        if (warningThreshold > 0 || criticalThreshold > 0) {
          currentNode->sev_crule = CalcRules::WeightedAverageWithThresholds;
        }
        if (warningThreshold > 0.0) {
          currentNode->thresholdLimits.push_back({warningThreshold / groupMembersCount, ngrt4n::Major, ngrt4n::Major});
        }
        if (warningThreshold > 0.0 ) {
          currentNode->thresholdLimits.push_back({criticalThreshold / groupMembersCount, ngrt4n::Critical, ngrt4n::Critical});
        }
      }
    }
  }

  if (parsingFailed) {
    ngrt4n::clearCoreData(*m_cdata);
  } else {
    attachOrphanedNodesToRoot(m_cdata->bpnodes, rootService);
    attachOrphanedNodesToRoot(m_cdata->cnodes, rootService);
    m_cdata->bpnodes.insert(rootService.id, rootService);
  }
  file.close();
  refreshUIWidgets();
}

void SvCreator::attachOrphanedNodesToRoot(NodeListT& nodes, NodeT& root)
{
  for (NodeListT::Iterator node = nodes.begin(); node != nodes.end(); ++node) {
    if (node->parent.isEmpty()) {
      node->parent = root.id;
      if (! root.child_nodes.isEmpty())
        root.child_nodes.append(ngrt4n::CHILD_Q_SEP);
      root.child_nodes.append(node->id);
    }
  }
}


int SvCreator::extractNagiosBPIGroupMembers(const QString& parentServiceId,
                                            const QString& sourceId,
                                            const QString& bpiGroupMembersChain,
                                            NodeListT& bpnodes,
                                            NodeListT& cnodes,
                                            QString& childNodesChain,
                                            bool& hasCluster)
{
  int childCount = 0;
  hasCluster = false;
  QStringList members = bpiGroupMembersChain.split(",");
  if (! members.isEmpty()) {

    Q_FOREACH(const QString& member, members) {

      if (member.isEmpty()) continue;

      bool isClusterMember = member.endsWith(";&");
      bool isEssentialMember = member.endsWith(";|");
      bool isGroupMember = member.startsWith("$");
      int start = isGroupMember ? 1 : 0;
      int count = (isClusterMember || isEssentialMember) ? member.size() - (start + 2) : member.size() - start;
      if (isClusterMember) hasCluster = true;

      QString memberId = member.mid(start, count);
      QString currentChildNodeId = "";

      if (isGroupMember) {
        NodeListT::Iterator memberNode = bpnodes.find(memberId);
        if (memberNode == bpnodes.end()) {
          memberNode = bpnodes.insert(memberId, createNode(memberId, memberId, parentServiceId));
        } else {
          memberNode->parent = parentServiceId;
        }
        memberNode->weight = isEssentialMember ? ngrt4n::WEIGHT_MAX: ngrt4n::WEIGHT_UNIT;
        currentChildNodeId = memberId.trimmed();
      } else {
        QStringList fields = memberId.split(";");
        if (fields.size() == 2) {
          currentChildNodeId = ngrt4n::genNodeId();
          QString generatedNodeName = QString("%1 on %2").arg(fields[1], fields[0]);
          NodeT cnode = createNode(currentChildNodeId, generatedNodeName, parentServiceId);
          cnode.type = NodeType::ITService;
          cnode.child_nodes = QString("%1:%2/%3").arg(sourceId, fields[0].trimmed(), fields[1].trimmed());
          cnode.weight = isEssentialMember ? ngrt4n::WEIGHT_MAX: ngrt4n::WEIGHT_UNIT;
          cnodes.insert(cnode.id, cnode);
        } else {
          showStatusMsg(tr("Bad service entry %1").arg(memberId), true);
          childCount = -1;
          break;
        }
      }

      if (childNodesChain.isEmpty()) {
        childNodesChain = currentChildNodeId;
      } else {
        childNodesChain += QString::fromStdString(ngrt4n::CHILD_SEP) + currentChildNodeId;
      }
      ++childCount;
    }
  }

  return childCount;
}

void SvCreator::importZabbixTriggersAsDataPoints(void)
{
  QMap<QString, SourceT> sourceInfos;
  fetchSourceList(ngrt4n::Zabbix, sourceInfos);
  CheckImportationSettingsForm importationSettingForm(sourceInfos.keys(), false);
  if (importationSettingForm.exec() == QDialog::Accepted) {
    QString srcId = importationSettingForm.selectedSource();
    QString filter = importationSettingForm.filter();
    SourceT srcInfo = sourceInfos[srcId];

    showStatusMsg(tr("Importing Zabbix triggers from %1:%2...").arg(srcInfo.id, srcInfo.mon_url), false);

    ChecksT checks;
    ZbxHelper handler;
    int retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::GroupFilter);
    processCheckLoadResults(retcode, srcId, checks, handler.lastError());
    if (checks.empty()) {
      int retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::HostFilter);
      processCheckLoadResults(retcode, srcId, checks, handler.lastError());
    }
  }
}

void SvCreator::importZabbixTriggersAsBundleBusinessView(void)
{
  QMap<QString, SourceT> sourceInfos;
  fetchSourceList(ngrt4n::Zabbix, sourceInfos);
  CheckImportationSettingsForm importationSettingForm(sourceInfos.keys(), false);
  if (importationSettingForm.exec() == QDialog::Accepted) {
    QString srcId = importationSettingForm.selectedSource();
    QString filter = importationSettingForm.filter();
    SourceT srcInfo = sourceInfos[srcId];

    showStatusMsg(tr("Importing Zabbix triggers (%1:%2...)").arg(srcInfo.id, srcInfo.mon_url), false);

    ChecksT checks;
    ZbxHelper handler;
    if (handler.loadChecks(srcInfo, checks, filter, ngrt4n::GroupFilter) != 0) {
      showStatusMsg(tr("Group trigger importation failed: %1").arg(handler.lastError()), true);
    } else {
      if (checks.empty()) {
        if (handler.loadChecks(srcInfo, checks, filter, ngrt4n::HostFilter) != 0) {
          showStatusMsg(tr("Host trigger importation failed: %1").arg(handler.lastError()), true);
        }
      }

      // handle results
      if (! checks.empty()) {
        ngrt4n::clearCoreData(*m_cdata);
        m_cdata->monitor = ngrt4n::Auto;

        NodeT root;
        root.id = ngrt4n::ROOT_ID;
        root.name = tr("Zabbix Services");
        root.type = NodeType::BusinessService;

        NodeT hostNode;
        NodeT triggerNode;
        hostNode.type = NodeType::BusinessService;
        triggerNode.type = NodeType::ITService;

        for (ChecksT::ConstIterator check = checks.begin(); check != checks.end(); ++check) {
          hostNode.id = hostNode.name = hostNode.description = QString::fromStdString(check->host);
          hostNode.id.replace(" ", "").replace("/", "");
          hostNode.parent = root.id;

          triggerNode.id = ngrt4n::genNodeId();
          triggerNode.parent = hostNode.id;
          triggerNode.name = QString::fromStdString(check->id);
          triggerNode.child_nodes = QString::fromStdString("%1:%2").arg(srcId, check->id.c_str());

          NodeListIteratorT hostIterPos =  m_cdata->bpnodes.find(hostNode.id);
          if (hostIterPos != m_cdata->bpnodes.end()) {
            hostIterPos->child_nodes.append(ngrt4n::CHILD_Q_SEP).append(triggerNode.id);
          } else {
            hostNode.child_nodes = triggerNode.id;
            if (root.child_nodes.isEmpty()) {
              root.child_nodes = hostNode.id;
            } else {
              root.child_nodes.append(ngrt4n::CHILD_Q_SEP).append(hostNode.id);
            }
            m_cdata->bpnodes.insert(hostNode.id, hostNode);
          }
          m_cdata->cnodes.insert(triggerNode.id, triggerNode);
        }

        // finally insert the root node and update UI widgets
        m_cdata->bpnodes.insert(ngrt4n::ROOT_ID, root);
        refreshUIWidgets();
      }
    }
  }
}


void SvCreator::importZabbixITServicesAsBusinessViews(void)
{
  QMap<QString, SourceT> sourceInfos;
  fetchSourceList(ngrt4n::Zabbix, sourceInfos);
  CheckImportationSettingsForm importationSettingForm(sourceInfos.keys(), false);
  if (importationSettingForm.exec() == QDialog::Accepted) {
    QString srcId = importationSettingForm.selectedSource();
    SourceT srcInfo = sourceInfos[srcId];

    showStatusMsg(tr("Loading IT services from %1:%2...").arg(srcInfo.id, srcInfo.mon_url), false);

    ZbxHelper handler;
    if (handler.loadITServices(srcInfo, *m_cdata) != 0) {
      showStatusMsg(tr("The importation of IT services failed: %1").arg(handler.lastError()), true);
    } else {
      m_activeConfig.clear();
      refreshUIWidgets();
      showStatusMsg(tr("The importation of IT services is completed"), false);
    }
  }
}



void SvCreator::importZenossComponents(void)
{
  QMap<QString, SourceT> sourceInfos;
  fetchSourceList(ngrt4n::Zenoss, sourceInfos);
  CheckImportationSettingsForm importationSettingForm(sourceInfos.keys(), false);
  if (importationSettingForm.exec() == QDialog::Accepted) {
    QString srcId = importationSettingForm.selectedSource();
    QString filter = importationSettingForm.filter();
    SourceT srcInfo = sourceInfos[srcId];

    showStatusMsg(tr("Loading components from %1:%2...").arg(srcInfo.id, srcInfo.mon_url), false);

    ChecksT checks;
    ZnsHelper handler(srcInfo.mon_url);
    int retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::GroupFilter);
    processCheckLoadResults(retcode, srcId, checks, handler.lastError());

    if (checks.empty()) {
      int retcode = handler.loadChecks(srcInfo, checks, filter, ngrt4n::HostFilter);
      processCheckLoadResults(retcode, srcId, checks, handler.lastError());
    }
  }
}

void SvCreator::importPandoraModules(void)
{
  QMap<QString, SourceT> sourceInfos;
  fetchSourceList(ngrt4n::Pandora, sourceInfos);
  CheckImportationSettingsForm importationSettingForm(sourceInfos.keys(), false);
  if (importationSettingForm.exec() == QDialog::Accepted) {
    QString srcId = importationSettingForm.selectedSource();
    QString agentName = importationSettingForm.filter();
    SourceT srcInfo = sourceInfos[srcId];

    showStatusMsg(tr("Loading Pandora agents data from %1:%2...").arg(srcInfo.id, srcInfo.mon_url), false);

    ChecksT checks;
    PandoraHelper handler(srcInfo.mon_url);
    int retcode = handler.loadChecks(srcInfo, checks, agentName);

    processCheckLoadResults(retcode, srcId, checks, handler.lastError());
  }
}

void SvCreator::processCheckLoadResults(int retCode, const QString& srcId, const ChecksT& checks, const QString& msg)
{
  if (retCode != 0) {
    showStatusMsg(msg, true);
  } else {
    m_editor->updateDataPoints(checks, srcId);
    showStatusMsg(tr("%1 entries imported").arg(checks.size()), false);
  }
}


void SvCreator::newView(void)
{
  if (treatCloseAction(false) == 0) {
    ngrt4n::clearCoreData(*m_cdata);
    m_activeConfig.clear();

    m_tree->clearTree();
    NodeT node = createNode(ngrt4n::ROOT_ID, tr("New View"), "");
    m_root = m_cdata->bpnodes.insert(node.id, node);

    refreshUIWidgets();
  }
}

void SvCreator::newNode(void)
{
  static int count = 1;
  QString label = tr("sub service %1").arg(QString::number(count));
  insertFromSelected(createNode(ngrt4n::genNodeId(),label , m_selectedNode));
  ++count;
}

NodeT SvCreator::createNode(const QString& id, const QString& label,const QString& parent)
{
  NodeT node;
  node.id = id;
  node.name = label;
  node.parent = parent;
  node.type = NodeType::BusinessService;
  node.sev = ngrt4n::Unknown;
  node.sev_crule = CalcRules::Worst;
  node.sev_prule = PropRules::Unchanged;
  node.icon = ngrt4n::DEFAULT_ICON;
  node.child_nodes = QString();
  node.thresholdLimits = QVector<ThresholdT>();
  return node;
}

void SvCreator::insertFromSelected(const NodeT& node)
{
  NodeListT::iterator pnode = m_cdata->bpnodes.find(m_selectedNode);
  if (pnode != m_cdata->bpnodes.end() && pnode->type != NodeType::ITService) {
    pnode->child_nodes += (!(pnode->child_nodes).isEmpty())? CHILD_SEPERATOR % node.id : node.id;
    QTreeWidgetItem* lastItem = m_tree->addNode(node, true);
    m_cdata->bpnodes.insert(node.id, node);
    m_tree->setCurrentItem(lastItem);
    fillEditorFromService(lastItem);
  } else {
    ngrt4n::alert(tr("This action is not allowed on the target service"));
  }
}


void SvCreator::deleteNode(void)
{
  QMessageBox msgBox;
  msgBox.setText(tr("Do you really want to delete the service and its sub services?"));
  msgBox.setWindowTitle(tr("Deleting service - %1 Editor").arg(APP_NAME));
  msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::Cancel);
  switch (msgBox.exec()) {
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
  if (! ngrt4n::findNode(m_cdata, _nodeId, node))
    return;

  if (node->type == NodeType::BusinessService
      && ! node->child_nodes.isEmpty()) {
    Q_FOREACH(const QString& checkId, node->child_nodes.split(CHILD_SEPERATOR)) {
      deleteNode(checkId);
    }
  }

  QTreeWidgetItem* item = m_tree->findNodeItem(_nodeId);
  QTreeWidgetItem* pItem = m_tree->findNodeItem(node->parent);
  if (pItem && item) {
    QRegExp regex("|^" + _nodeId + CHILD_SEPERATOR +
                  "|^" + _nodeId + "$" +
                  "|" + CHILD_SEPERATOR  + _nodeId);

    NodeListT::iterator pNode = m_cdata->bpnodes.find(node->parent);
    if (pNode != m_cdata->bpnodes.end()) {
      pNode->child_nodes.remove(regex);
    }
    if (node->type == NodeType::ITService) {
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
    if (! m_clipboardData)
      m_clipboardData = new NodeT;
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
  //FIXME: this change the value of combo box
  if (! m_selectedNode.isEmpty()) {
    fillEditorFromService(m_tree->findNodeItem(m_selectedNode));
  }

  if (m_activeConfig.isEmpty())
    m_activeConfig = selectFileDestinationPath();

  if (! m_activeConfig.isEmpty())
    recordData(m_activeConfig);
  else
    handleInvalidPathError();
}


void SvCreator::saveAs(void)
{
  m_activeConfig = selectFileDestinationPath();
  if (! m_activeConfig.isEmpty()) {
    recordData(m_activeConfig);
  } else {
    handleInvalidPathError();
  }
}

void SvCreator::handleInvalidPathError(void)
{
  QString msg = tr("Invalid path");
  ngrt4n::alert(msg);
  showStatusMsg(msg, true);
}


QString SvCreator::selectFileDestinationPath(void)
{
  QString result = "";

  QString filter;
  QString path = QFileDialog::getSaveFileName(this,
                                              tr("Select the destination file | %1").arg(APP_NAME),
                                              m_activeConfig,
                                              FILE_FILTER,
                                              &filter);

  if (! path.isNull()) {
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
    } else if (filter == PANDORA_SOURCE) {
      m_cdata->monitor = ngrt4n::Pandora;
      if (fileInfo.suffix().isEmpty()) path.append(".pfms.ngrt4n.xml");
    } else {
      m_cdata->monitor = ngrt4n::Auto;
      if (fileInfo.suffix().isEmpty()) path.append(".ms.ngrt4n.xml");
    }

    result = path;
  }

  return result;
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
    if (enforceClose)
      qApp->quit();
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


void SvCreator::handleTreeNodeMoved(const QString& nodeId)
{
  QTreeWidgetItem* item =  m_tree->findNodeItem(nodeId);
  if (item != NULL) {

    QTreeWidgetItem* tnodeP = item->parent();
    if (tnodeP) {
      NodeListT::iterator nodeIt = m_cdata->bpnodes.find(nodeId);

      if (nodeIt != m_cdata->bpnodes.end()) {
        /* Remove the node on its old parent's child list*/
        QRegExp regex ("|^" + nodeId + CHILD_SEPERATOR +
                       "|^" + nodeId + "$" +
                       "|" + CHILD_SEPERATOR + nodeId);
        NodeListT::iterator pNodeIt = m_cdata->bpnodes.find(nodeIt->parent);
        if (pNodeIt != m_cdata->bpnodes.end()) {
          pNodeIt->child_nodes.remove(regex);
        }

        /* Add the node on its new parent's child list*/
        nodeIt->parent = tnodeP->data(0, QTreeWidgetItem::UserType).toString();
        pNodeIt = m_cdata->bpnodes.find(nodeIt->parent);
        if (pNodeIt != m_cdata->bpnodes.end()) {
          pNodeIt->child_nodes += (pNodeIt->child_nodes != "")? CHILD_SEPERATOR + nodeId : nodeId;
        }
      }
    }
  }
}

void SvCreator::handleNodeTypeActivated(qint32 targetType)
{
  NodeListT::iterator node = m_cdata->bpnodes.find(m_selectedNode);
  if (node == m_cdata->bpnodes.end()) {
    return; // nothing to do
  }

  int currentType = node->type;
  if (targetType == NodeType::BusinessService) {
    if (currentType == NodeType::ITService) {
      node->child_nodes.clear();
      if (m_editor->updateNodeInfoFromEditorContents(*node)) {
        m_tree->findNodeItem(m_selectedNode)->setText(0, node->name);
        m_hasLeftUpdates = true;
        updateWindowTitle("*");
      }
    }
  } else { // current type is business service
    if (m_editor->updateNodeInfoFromEditorContents(*node)) {
      m_tree->findNodeItem(m_selectedNode)->setText(0, node->name);
      m_hasLeftUpdates = true;
      updateWindowTitle("*");
    }
  }
}

void SvCreator::updateWindowTitle(const QString& append)
{
  if (! m_activeConfig.isEmpty()) {
    setWindowTitle(tr("%1 Editor - %2%3").arg(APP_NAME, m_activeConfig, append));
  } else {
    setWindowTitle(tr("%1 Editor - %2%3").arg(APP_NAME, m_root->name, append));
  }

  if (! append.isEmpty()) {
    showStatusMsg(tr("* Unsaved changes left"), false);
  }
}

void SvCreator::handleShowOnlineResources(void)
{
  QDesktopServices launcher;
  launcher.openUrl(QUrl(DOCS_URL));
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
    if (m_editor->updateNodeInfoFromEditorContents(*node)) {
      QTreeWidgetItem* selectedNodeItem = m_tree->findNodeItem(m_selectedNode);
      if (selectedNodeItem) {
        selectedNodeItem->setText(0, node->name);
      }
      m_hasLeftUpdates = true;
      showStatusMsg(tr("* Unsaved changes left"), false);
      updateWindowTitle("*");
    }
  }
  m_selectedNode = _item->data(0, QTreeWidgetItem::UserType).toString();
  if (ngrt4n::findNode(m_cdata, m_selectedNode, node))
    m_editor->fillInEditorWithContent(*node);
}


void SvCreator::handleReturnPressed(void)
{
  NodeListT::iterator node = m_cdata->bpnodes.find(m_selectedNode);
  if (node != m_cdata->bpnodes.end()) {
    if (m_editor->updateNodeInfoFromEditorContents(*node)) {
      m_tree->findNodeItem(m_selectedNode)->setText(0, node->name);
      m_hasLeftUpdates = true;
      updateWindowTitle("*");
    }
  }
}

void SvCreator::recordData(const QString& path)
{
  showStatusMsg(tr("Saving in %1...").arg(path), false);

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
    showStatusMsg(tr("Cannot open file: %1").arg(path), true);
    return;
  }
  m_root = m_cdata->bpnodes.find(ngrt4n::ROOT_ID);
  if (m_root == m_cdata->bpnodes.end()) {
    file.close();
    QString msg =  tr("The hierarchy does not have root");
    ngrt4n::alert(msg);
    showStatusMsg(msg, true);
  } else {
    QTextStream outStream(&file);

    outStream << "<?xml version=\"1.0\"?>\n"
              << QString("<ServiceView compat=\"3.1\" monitor=\"%1\">\n").arg( QString::number(m_cdata->monitor) )
              << generateNodeXml(*m_root);

    Q_FOREACH(const NodeT& service, m_cdata->bpnodes) {
      if (service.id != ngrt4n::ROOT_ID && ! service.parent.isEmpty()) {
        outStream << generateNodeXml(service);
      }
    }

    Q_FOREACH(const NodeT& service, m_cdata->cnodes) {
      if (service.parent.isEmpty())
        continue;
      outStream << generateNodeXml(service);
    }

    outStream << "</ServiceView>\n";

    file.close();

    m_hasLeftUpdates = false;
    m_activeConfig = ngrt4n::getAbsolutePath(path);
    showStatusMsg(tr("File saved: %1").arg(m_activeConfig), false);
    setWindowTitle(tr("%1 Editor - %2").arg(APP_NAME).arg(m_activeConfig));
  }
}

QString SvCreator::generateNodeXml(const NodeT& node)
{
  QString xml = QString("<Service id=\"%1\" "
                        " type=\"%2\" "
                        " statusCalcRule=\"%3\" "
                        " statusPropRule=\"%4\" "
                        " weight=\"%5\"> \n"
                        ).arg(node.id,
                              QString::number(node.type),
                              QString::number(node.sev_crule),
                              QString::number(node.sev_prule),
                              QString::number(node.weight));

  xml.append( QString(" <Name>%1</Name>\n").arg(node.name) )
      .append( QString(" <Icon>%1</Icon>\n").arg(node.icon) )
      .append( QString(" <Description>%1</Description>\n").arg(node.description) )
      .append( QString(" <AlarmMsg>%1</AlarmMsg>\n").arg(node.alarm_msg) )
      .append( QString(" <NotificationMsg>%1</NotificationMsg>\n").arg(node.notification_msg) )
      .append( QString(" <SubServices>%1</SubServices>\n").arg(node.child_nodes) ) ;

  if (node.sev_crule == CalcRules::WeightedAverageWithThresholds) {

    xml.append( QString(" <Thresholds>%1</Thresholds>\n").arg(ThresholdHelper::listToData(node.thresholdLimits)) );
  }

  xml.append("</Service>\n");

  return xml;
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
  const QString MENU_FILE="FILE";
  const QString MENU_EDITION="EDITION";
  const QString MENU_IMPORTATION="IMPORTATION";
  const QString MENU_HELP="HELP";
  m_menus[MENU_FILE] = m_menuBar->addMenu(tr("&File"));
  m_subMenus["NewFile"] = m_menus[MENU_FILE]->addAction("New &File"),
      m_subMenus["NewFile"]->setShortcut(QKeySequence::New);
  m_subMenus["Open"] = m_menus[MENU_FILE]->addAction(QIcon(":images/built-in/folder.png"), tr("&Open")),
      m_subMenus["Open"]->setShortcut(QKeySequence::Open);
  m_subMenus["Save"] = m_menus[MENU_FILE]->addAction(QIcon(":images/built-in/disket.png"), tr("&Save")),
      m_subMenus["Save"]->setShortcut(QKeySequence::Save);
  m_subMenus["SaveAs"] = m_menus[MENU_FILE]->addAction(QIcon(":images/built-in/disket.png"), tr("Save &As...")),
      m_subMenus["SaveAs"]->setShortcut(QKeySequence::SaveAs);
  m_menus[MENU_FILE]->addSeparator(),
      m_subMenus["Quit"] = m_menus[MENU_FILE]->addAction(tr("&Quit")),
      m_subMenus["Quit"]->setShortcut(QKeySequence::Quit);

  m_menus[MENU_EDITION] = m_menuBar->addMenu(tr("&Edition"));
  m_subMenus["NewNode"] = m_menus[MENU_EDITION]->addAction(tr("&Add sub service")),
      m_nodeContextMenu->addAction(m_subMenus["NewNode"]),
      m_subMenus["NewNode"]->setShortcut(QKeySequence::AddTab);
  m_subMenus["CopySelected"] = m_menus[MENU_EDITION]->addAction(tr("&Copy")),
      m_nodeContextMenu->addAction(m_subMenus["CopySelected"]),
      m_subMenus["CopySelected"]->setShortcut(QKeySequence::Copy);
  m_subMenus["PasteFromSelected"] = m_menus[MENU_EDITION]->addAction(tr("&Paste")),
      m_nodeContextMenu->addAction(m_subMenus["PasteFromSelected"]),
      m_subMenus["PasteFromSelected"]->setShortcut(QKeySequence::Paste);
  m_subMenus["DeleteNode"] = m_menus[MENU_EDITION]->addAction(tr("&Delete")),
      m_subMenus["DeleteNode"]->setShortcut(QKeySequence::Delete),
      m_nodeContextMenu->addAction(m_subMenus["DeleteNode"]);

  m_menus[MENU_IMPORTATION] = m_menuBar->addMenu(tr("&Importation"));
  m_subMenus["ImportNagiosChecks"] = m_menus[MENU_IMPORTATION]->addAction(QIcon(":images/built-in/import-nagios.png"), tr("Import Na&gios Checks as Data Points")),
      m_subMenus["ImportNagiosLivestatusChecks"] = m_menus[MENU_IMPORTATION]->addAction(QIcon(":images/built-in/import-livestatus.png"), tr("Import Livestatus Checks as Data Points")),
      m_subMenus["ImportNagiosBPIConf"] = m_menus[MENU_IMPORTATION]->addAction(tr("Import Nagios BPI Configuration as Business View"));
  m_menus[MENU_IMPORTATION]->addSeparator(),
      m_subMenus["ImportZabbixTriggers"] = m_menus[MENU_IMPORTATION]->addAction(QIcon(":images/built-in/import-zabbix.png"), tr("Import Za&bbix Triggers as Data Points")),
      m_subMenus["ImportZabbixITServices"] = m_menus[MENU_IMPORTATION]->addAction(tr("Import Zabbix IT Services as Business View"));
  m_subMenus["AutomaticImportZabbixTriggers"] = m_menus[MENU_IMPORTATION]->addAction(tr("Import Zabbix Triggers as Host-based Business View"));
  m_menus[MENU_IMPORTATION]->addSeparator(),
      m_subMenus["ImportZenossComponents"] = m_menus[MENU_IMPORTATION]->addAction(QIcon(":images/built-in/import-zenoss.png"), tr("Import Z&enoss Components"));
  m_menus[MENU_IMPORTATION]->addSeparator(),
      m_subMenus["ImportPandoraModules"] = m_menus[MENU_IMPORTATION]->addAction(QIcon(":images/built-in/import-pandora.png"), tr("Import &Pandora Modules"));

  m_menus[MENU_HELP] = m_menuBar->addMenu(tr("&Help"));
  m_subMenus["ShowOnlineResources"] = m_menus[MENU_HELP]->addAction(tr("Online &Resources")),
      m_subMenus["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
  m_menus[MENU_HELP]->addSeparator(),
      m_subMenus["ShowAbout"] = m_menus[MENU_HELP]->addAction(tr("&About %1").arg(APP_NAME));

  m_toolBar->addAction(m_subMenus["Save"]);
  m_toolBar->addAction(m_subMenus["Open"]);
  m_toolBar->addAction(m_subMenus["ImportNagiosChecks"]);
  m_toolBar->addAction(m_subMenus["ImportNagiosLivestatusChecks"]);
  m_toolBar->addAction(m_subMenus["ImportZabbixTriggers"]);
  m_toolBar->addAction(m_subMenus["ImportZenossComponents"]);
  m_toolBar->addAction(m_subMenus["ImportPandoraModules"]);
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


void SvCreator::handleErrorOccurred(QString msg)
{
  showStatusMsg(msg, true);
}

void SvCreator::showStatusMsg(const QString& msg, bool error)
{
  statusBar()->showMessage(msg);
  if (error)
    statusBar()->setStyleSheet("background: red;");
  else
    statusBar()->setStyleSheet("background: transparent;");
}
