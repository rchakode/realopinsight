/*
 * SvNavigator.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 24-05-2012                                                  #
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


#include "SvNavigator.hpp"
#include "core/MonitorBroker.hpp"
#include "core/ns.hpp"
#include "utilsClient.hpp"
#include "client/JsHelper.hpp"
#include <QScriptValueIterator>
#include <sstream>
#include <QStatusBar>
#include <QObject>
#include <zmq.h>
#include <iostream>
#include <locale>
#include <memory>

//FIXME: the system crashes on two many file opened

const QString DEFAULT_TIP_PATTERN(QObject::tr("Service: %1\nDescription: %2\nCriticity: %3\n   Calc. Rule: %4\n   Prop. Rule: %5"));
const QString ALARM_SPECIFIC_TIP_PATTERN(QObject::tr("\nTarget Host: %6\nCheck/Trigger ID: %7\nCheck Output: %8\nMore info: %9"));
const QString SERVICE_OFFLINE_MSG(QObject::tr("Failed to connect to %1"));
const QString DEFAULT_ERROR_MSG("{\"return_code\": \"-1\", \"message\": \""%SERVICE_OFFLINE_MSG%"\"}");
const QString ID_PATTERN("%1/%2");
const string UNKNOWN_UPDATE_TIME = utils::getCtime(0);

StringMapT SvNavigator::propRules() {
  StringMapT map;
  map.insert(PropRules::label(PropRules::Unchanged),
             PropRules::toString(PropRules::Unchanged));
  map.insert(PropRules::label(PropRules::Decreased),
             PropRules::toString(PropRules::Decreased));
  map.insert(PropRules::label(PropRules::Increased),
             PropRules::toString(PropRules::Increased));
  return map;
}

StringMapT SvNavigator::calcRules() {
  StringMapT map;
  map.insert(CalcRules::label(CalcRules::HighCriticity),
             CalcRules::toString(CalcRules::HighCriticity));
  map.insert(CalcRules::label(CalcRules::WeightedCriticity),
             CalcRules::toString(CalcRules::WeightedCriticity));
  return map;
}

SvNavigator::SvNavigator(const qint32& _userRole,
                         const QString& _config,
                         QWidget* parent)
  : QMainWindow(parent),
    mcoreData (new CoreDataT()),
    mconfigFile(_config),
    muserRole (_userRole),
    msettings (new Settings()),
    mchart (new Chart()),
    mfilteredMsgPanel (NULL),
    mmainSplitter (new QSplitter(this)),
    mrightSplitter (new QSplitter()),
    mviewPanel (new QTabWidget()),
    mmsgConsolePanel (new QTabWidget()),
    mbrowser (new WebKit()),
    mmap (new GraphView(this)),
    mtree (new SvNavigatorTree()),
    mprefWindow (new Preferences(_userRole, Preferences::ChangeMonitoringSettings)),
    mchangePasswdWindow (new Preferences(_userRole, Preferences::ChangePassword)),
    mmsgConsole(new MsgConsole(this)),
    mnodeContextMenu (new QMenu()),
    mzbxHelper(new ZbxHelper()),
    mzbxAuthToken(""),
    mhostLeft(0),
    mznsHelper(new ZnsHelper()),
    misLogged(false),
    mlastError("")
{
  setWindowTitle(tr("%1 Operations Console").arg(appName));
  loadMenus();
  mviewPanel->addTab(mmap, tr("Dashboard"));
  mviewPanel->addTab(mbrowser, tr("Native Web UI"));
  mmsgConsolePanel->addTab(mmsgConsole, tr("Event Console"));
  mmainSplitter->addWidget(mtree);
  mmainSplitter->addWidget(mrightSplitter);
  mrightSplitter->addWidget(mviewPanel);
  mrightSplitter->addWidget(mmsgConsolePanel);
  mrightSplitter->setOrientation(Qt::Vertical);
  setCentralWidget(mmainSplitter);
  updateMonitoringSettings();
  tabChanged(0);
  addEvents();
}

SvNavigator::~SvNavigator()
{
  if (mfilteredMsgPanel) delete mfilteredMsgPanel;
  delete mmsgConsole;
  delete mchart;
  delete mtree;
  delete mbrowser;
  delete mmap;
  delete mcoreData;
  delete mviewPanel;
  delete mmsgConsolePanel;
  delete mrightSplitter;
  delete mmainSplitter;
  delete mprefWindow;
  delete mchangePasswdWindow;
  delete mzbxHelper;
  delete mznsHelper;
  unloadMenus();
}


void SvNavigator::loadMenus(void)
{
  QMenuBar* menuBar = new QMenuBar();
  mmenus["FILE"] = menuBar->addMenu("&File"),
      msubMenus["Refresh"] = mmenus["FILE"]->addAction(QIcon(":images/built-in/refresh.png"),tr("&Refresh Screen")),
      msubMenus["Capture"] = mmenus["FILE"]->addAction( QIcon(":images/built-in/camera.png"),tr("&Save Map as Image")),
      mmenus["FILE"]->addSeparator(),
      msubMenus["Quit"] = mmenus["FILE"]->addAction(tr("&Quit"));
  mmenus["MAP"] = menuBar->addMenu("&Map"),
      msubMenus["ZoomIn"] = mmenus["MAP"]->addAction(QIcon(":images/built-in/zoomin.png"),tr("Zoom &In")),
      msubMenus["ZoomOut"] = mmenus["MAP"]->addAction(QIcon(":images/built-in/zoomout.png"),tr("Zoom &Out")),
      msubMenus["HideChart"] = mmenus["MAP"]->addAction(tr("Hide &Chart"));
  mmenus["PREFERENCES"] = menuBar->addMenu("&Preferences"),
      msubMenus["ChangePassword"] = mmenus["PREFERENCES"]->addAction(tr("Change &Password")),
      msubMenus["ChangeMonitoringSettings"] = mmenus["PREFERENCES"]->addAction(tr("&Monitoring Settings"));
  mmenus["BROWSER"] = menuBar->addMenu("&Browser"),
      msubMenus["BrowserBack"] = mmenus["BROWSER"]->addAction(QIcon(":images/built-in/browser-back.png"),tr("Bac&k")),
      msubMenus["BrowserForward"] = mmenus["BROWSER"]->addAction(QIcon(":images/built-in/browser-forward.png"),tr("For&ward"));
  msubMenus["BrowserStop"] = mmenus["BROWSER"]->addAction(QIcon(":images/built-in/browser-stop.png"),tr("Sto&p"));
  mmenus["HELP"] = menuBar->addMenu("&Help"),
      msubMenus["ShowOnlineResources"] = mmenus["HELP"]->addAction(tr("Online &Resources")),
      mmenus["HELP"]->addSeparator(),
      msubMenus["ShowAbout"] = mmenus["HELP"]->addAction(tr("&About %1").arg(appName));
  msubMenus["Capture"]->setShortcut(QKeySequence::Save);
  msubMenus["Refresh"]->setShortcut(QKeySequence::Refresh);
  msubMenus["ZoomIn"]->setShortcut(QKeySequence::ZoomIn);
  msubMenus["ZoomOut"]->setShortcut(QKeySequence::ZoomOut);
  msubMenus["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
  msubMenus["Quit"]->setShortcut(QKeySequence::Quit);

  mcontextMenuList["FilterNodeRelatedMessages"] = mnodeContextMenu->addAction(tr("&Filter related messages"));
  mcontextMenuList["CenterOnNode"] = mnodeContextMenu->addAction(tr("Center Graph &On"));
  mcontextMenuList["Cancel"] = mnodeContextMenu->addAction(tr("&Cancel"));

  QToolBar* toolBar = addToolBar(QString(ngrt4n::APP_NAME.c_str()));
  toolBar->setIconSize(QSize(16,16));
  toolBar->addAction(msubMenus["Refresh"]);
  toolBar->addAction(msubMenus["ZoomIn"]);
  toolBar->addAction(msubMenus["ZoomOut"]);
  toolBar->addAction(msubMenus["Capture"]);
  toolBar->addAction(msubMenus["BrowserBack"]);
  toolBar->addAction(msubMenus["BrowserForward"]);
  toolBar->addAction(msubMenus["BrowserStop"]);
  setMenuBar(menuBar);
}

void SvNavigator::closeEvent(QCloseEvent * event)
{
  if (mfilteredMsgPanel)
    mfilteredMsgPanel->close();
  QMainWindow::closeEvent(event);
}


void SvNavigator::contextMenuEvent(QContextMenuEvent * event)
{
  QPoint pos = event->globalPos();
  QList<QTreeWidgetItem*> treeNodes = mtree->selectedItems();
  QGraphicsItem* graphNode = mmap->nodeAtGlobalPos(pos);

  if (treeNodes.length() || graphNode) {
      if (graphNode) {
          QString itemId = graphNode->data(0).toString();
          mselectedNode =  itemId.left(itemId.indexOf(":"));
        }  else {
          mselectedNode = treeNodes[0]->data(0, QTreeWidgetItem::UserType).toString();
        }
      mnodeContextMenu->exec(pos);
    }
}

void SvNavigator::startMonitor()
{
  prepareDashboardUpdate();
  switch(mcoreData->monitor) {
    case MonitorBroker::ZENOSS:
    case MonitorBroker::ZABBIX:
      !misLogged ? openRpcSession(): postRpcDataRequest();
      break;
    case MonitorBroker::NAGIOS:
    default:
      runNagiosMonitor();
      break;
    }
}

void SvNavigator::timerEvent(QTimerEvent *)
{
  startMonitor();
}

void  SvNavigator::updateStatusBar(const QString& msg) {
  statusBar()->showMessage(msg);
}


void SvNavigator::load(const QString& _file)
{
  if ( ! _file.isEmpty()) {
      mconfigFile = utils::getAbsolutePath(_file);
    }
  mactiveFile = mconfigFile;
  Parser parser;
  parser.parseSvConfig(mconfigFile, *mcoreData);
  mtree->clear();
  mtree->addTopLevelItem(mcoreData->tree_items[SvNavigatorTree::rootID]);
  mmap->load(parser.getDotGraphFile(), mcoreData->bpnodes, mcoreData->cnodes);
  mbrowser->setUrl(mmonitorBaseUrl);
  resize();
  show();
  mmap->scaleToFitViewPort();
  setWindowTitle(tr("%1 Operations Console - %2").arg(appName).arg(mconfigFile));
}

void SvNavigator::unloadMenus(void)
{
  msubMenus.clear();
  mmenus.clear();
  delete mnodeContextMenu;
}

void SvNavigator::handleChangePasswordAction(void)
{
  mchangePasswdWindow->exec();
}

void SvNavigator::handleChangeMonitoringSettingsAction(void)
{
  mprefWindow->exec();
  updateMonitoringSettings();
  killTimer(mtimer);
  mtimer = startTimer(mupdateInterval);
  misLogged = false;
  startMonitor();
}

void SvNavigator::handleShowOnlineResources(void)
{
  QDesktopServices appLauncher;
  appLauncher.openUrl(QUrl("http://RealOpInsight.com/"));
}

void SvNavigator::handleShowAbout(void)
{
  Preferences about(muserRole, Preferences::ShowAbout);
  about.exec();
}

int SvNavigator::runNagiosMonitor(void)
{
  auto socket = std::unique_ptr<Socket>(new Socket(ZMQ_REQ));
  socket->connect(mserverUrl.toStdString());
  socket->makeHandShake();
  if (socket->isConnected2Server()) {
      if (socket->getServerSerial() < 110) {
          utils::alert(tr("The server %1 is not supported").arg(socket->getServerSerial()));
          mupdateSucceed = false;
        }
      updateStatusBar(tr("Updating..."));
    } else {
      mupdateSucceed = false;
      QString msg = SERVICE_OFFLINE_MSG.arg(mserverUrl);
      utils::alert(msg);
      updateStatusBar(msg);
    }

  for (auto cnode = mcoreData->cnodes.begin();
       cnode != mcoreData->cnodes.end();
       cnode++) {
      if (cnode->child_nodes == "") {
          cnode->criticity = MonitorBroker::CRITICITY_UNKNOWN;
          mcoreData->check_status_count[cnode->criticity]++;
          continue;
        }

      QStringList checks = cnode->child_nodes.split(Parser::CHILD_SEP);
      foreach(const QString& cid, checks) {
          QString msg = mserverAuthChain%":"%cid;
          if (mupdateSucceed) {
              socket->send(msg.toStdString());
              msg = QString::fromStdString(socket->recv());
            } else {
              msg = DEFAULT_ERROR_MSG.arg(mserverUrl);
            }
          JsonHelper jsHelper(msg.toStdString());
          if (jsHelper.getProperty("return_code").toInt32() == 0
              && socket->isConnected2Server())
            {
              cnode->check.status = jsHelper.getProperty("status").toInt32();
              cnode->check.host = jsHelper.getProperty("host").toString().toStdString();
              cnode->check.last_state_change = utils::getCtime(jsHelper.getProperty("lastchange").toUInt32()); //TODO: to be tested
              cnode->check.check_command = jsHelper.getProperty("command").toString().toStdString();
              cnode->check.alarm_msg = jsHelper.getProperty("message").toString().toStdString();
            } else {
              cnode->check.status = MonitorBroker::NAGIOS_UNKNOWN;
              cnode->check.host = "Unknown";
              cnode->check.last_state_change = UNKNOWN_UPDATE_TIME;
              cnode->check.check_command = "Unknown";
              cnode->check.alarm_msg = jsHelper.getProperty("message").toString().toStdString();
            }
          computeStatusInfo(cnode);
          updateDashboard(cnode);
          mcoreData->check_status_count[cnode->criticity]++;
        }
    }
  socket->disconnect();
  socket.reset(nullptr);
  finalizeDashboardUpdate();
  return 0;
}

void SvNavigator::prepareDashboardUpdate(void)
{
  QMainWindow::setEnabled(false);
  mcoreData->check_status_count[MonitorBroker::CRITICITY_NORMAL] = 0;
  mcoreData->check_status_count[MonitorBroker::CRITICITY_MINOR] = 0;
  mcoreData->check_status_count[MonitorBroker::CRITICITY_MAJOR] = 0;
  mcoreData->check_status_count[MonitorBroker::CRITICITY_HIGH] = 0;
  mcoreData->check_status_count[MonitorBroker::CRITICITY_UNKNOWN] = 0;
  mhostLeft = mcoreData->hosts.size();
  mupdateSucceed = true;
  QString msg = QObject::tr("Connecting to the server (%1)...");
  switch(mcoreData->monitor) {
    case MonitorBroker::NAGIOS:
      msg = msg.arg(mserverUrl);
      break;
    case MonitorBroker::ZABBIX:
      msg = msg.arg(mzbxHelper->getApiUri());
      break;
    case MonitorBroker::ZENOSS:
      msg = msg.arg(mznsHelper->getApiBaseUrl()); //FIXME: msg.arg(mznsHelper->getApiContextUrl()) crashes
      break;
    default:
      break;
    }
  updateStatusBar(msg);
}


QString SvNavigator::getNodeToolTip(const NodeT& _node)
{
  QString toolTip = DEFAULT_TIP_PATTERN.arg(_node.name)
      .arg(const_cast<QString&>(_node.description).replace("\n", " "))
      .arg(utils::statusToString(_node.criticity))
      .arg(CalcRules::label(_node.criticity_crule))
      .arg(PropRules::label(_node.criticity_prule));

  if (_node.type == NodeType::ALARM_NODE) {
      QString msg = "";
      if (_node.criticity == MonitorBroker::CRITICITY_NORMAL) {
          msg = const_cast<QString&>(_node.notification_msg).replace("\n", " ");
        } else {
          msg = QString::fromStdString(_node.check.alarm_msg).replace("\n", " ");
        }
      toolTip += ALARM_SPECIFIC_TIP_PATTERN.arg(QString::fromStdString(_node.check.host).replace("\n", " "))
          .arg(_node.child_nodes)
          .arg(QString::fromStdString(_node.check.alarm_msg).replace("\n", " "))
          .arg(msg);
    }
  return toolTip;
}


void SvNavigator::updateDashboard(NodeListT::iterator& _node)
{
  updateDashboard(*_node);
}

void SvNavigator::updateDashboard(const NodeT& _node)
{
  QString toolTip = getNodeToolTip(_node);
  updateNavTreeItemStatus(_node, toolTip);
  mmap->updateNode(_node, toolTip);
  mmsgConsole->updateNodeMsg(_node);
  emit hasToBeUpdate(_node.parent);
}

void SvNavigator::updateCNodes(const MonitorBroker::CheckT& check) {

  for (auto cnode = mcoreData->cnodes.begin(); cnode != mcoreData->cnodes.end(); cnode++) {
      if (cnode->child_nodes.toStdString().compare(check.id) == 0) {
          cnode->check = check;
          computeStatusInfo(cnode);
          mcoreData->check_status_count[cnode->criticity]++;
          updateDashboard(cnode);
        }
    }
}

void SvNavigator::finalizeDashboardUpdate(const bool& enable)
{
  if (mcoreData->cnodes.size() != 0) {
      Chart *chart = new Chart;
      QString chartdDetails = chart->update(mcoreData->check_status_count, mcoreData->cnodes.size());
      mmap->updateStatsPanel(chart);
      if (mchart) delete mchart; mchart = chart; mchart->setToolTip(chartdDetails);
      mmsgConsole->sortByColumn(1, Qt::AscendingOrder);
      mmsgConsole->updateColumnWidths(mmsgConsoleSize);
      mupdateInterval = msettings->value(Preferences::UPDATE_INTERVAL_KEY).toInt();
      mupdateInterval = 1000*((mupdateInterval > 0)? mupdateInterval:MonitorBroker::DEFAULT_UPDATE_INTERVAL);
      mtimer = startTimer(mupdateInterval);
      if (mupdateSucceed) updateStatusBar(tr("Update completed"));
    }
  QMainWindow::setEnabled(enable);
}

void SvNavigator::computeStatusInfo(NodeListT::iterator&  _node)
{
  computeStatusInfo(*_node);
}


void SvNavigator::computeStatusInfo(NodeT& _node)
{
  _node.criticity = utils::computeCriticity(mcoreData->monitor, _node.check.status);
  _node.prop_criticity = utils::computePropCriticity(_node.criticity, _node.criticity_prule);
  QString statusText = (_node.criticity == MonitorBroker::CRITICITY_NORMAL)? _node.notification_msg : _node.alarm_msg;
  QRegExp regexp(MsgConsole::TAG_HOSTNAME);
  statusText.replace(regexp, _node.check.host.c_str());

  auto info = QString(_node.check.id.c_str()).split("/");
  if (info.length() > 1) {
      regexp.setPattern(MsgConsole::TAG_CHECK);
      statusText.replace(regexp, info[1]);
    }
  if (mcoreData->monitor == MonitorBroker::NAGIOS) { // FIXME: Generalize this to the other monitors
      info = QString(_node.check.check_command.c_str()).split("!");
      if (info.length() >= 3) {
          regexp.setPattern(MsgConsole::TAG_THERESHOLD);
          statusText.replace(regexp, info[1]);
          if (_node.criticity == MonitorBroker::CRITICITY_MAJOR)
            statusText.replace(regexp, info[2]);
        }
    }
  if (mcoreData->monitor == MonitorBroker::ZABBIX) {
      regexp.setPattern(MsgConsole::TAG_HOSTNAME_ZABBIX); //FIXME: do it only for Zabbix
      statusText.replace(regexp, _node.check.host.c_str());
      _node.check.alarm_msg = QString(_node.check.alarm_msg.c_str()).replace(regexp, _node.check.host.c_str()).toStdString();
    }
  if (_node.criticity == MonitorBroker::CRITICITY_NORMAL) {
      _node.notification_msg = statusText;
    } else {
      _node.alarm_msg = statusText;
    }
}

void SvNavigator::updateBpNode(QString _nodeId)
{
  NodeListT::iterator node;
  if (!utils::findNode(mcoreData, _nodeId, node)) return;

  QStringList nodeIds = node->child_nodes.split(Parser::CHILD_SEP);
  Criticity criticity;
  for (auto it = nodeIds.begin(); it != nodeIds.end(); it++) {
      NodeListT::iterator child;
      if (! utils::findNode(mcoreData, *it, child))
        continue;
      Criticity cst(static_cast<MonitorBroker::CriticityT>(child->prop_criticity));
      if (node->criticity_crule == CalcRules::WeightedCriticity) {
          criticity = criticity / cst;
        } else {
          criticity = criticity * cst;
        }
    }
  node->criticity = criticity.getValue();
  switch(node->criticity_prule) {
    case PropRules::Increased: node->prop_criticity = (criticity++).getValue();
      break;
    case PropRules::Decreased: node->prop_criticity = (criticity--).getValue();
      break;
    default: node->prop_criticity = node->criticity;
      break;
    }
  QString toolTip = getNodeToolTip(*node);
  mmap->updateNode(node, toolTip);
  updateNavTreeItemStatus(node, toolTip);
  emit hasToBeUpdate(node->parent);
}


void SvNavigator::updateNavTreeItemStatus(const NodeListT::iterator& _node, const QString& _tip)
{
  updateNavTreeItemStatus(*_node, _tip);
}

void SvNavigator::updateNavTreeItemStatus(const NodeT& _node, const QString& _tip)
{
  auto tnode_it = mcoreData->tree_items.find(_node.id);
  if (tnode_it != mcoreData->tree_items.end()) {
      (*tnode_it)->setIcon(0, utils::getTreeIcon(_node.criticity));
      (*tnode_it)->setToolTip(0, _tip);
    }
}

void SvNavigator::updateMonitoringSettings() {
  mmonitorBaseUrl = msettings->value(Preferences::URL_KEY).toString();
  mserverAuthChain = msettings->value(Preferences::SERVER_PASS_KEY).toString();
  mserverAddr = msettings->value(Preferences::SERVER_ADDR_KEY).toString();
  mserverPort = msettings->value(Preferences::SERVER_PORT_KEY).toString();
  mserverUrl = QString("tcp://%1:%2").arg(mserverAddr).arg(mserverPort);
  mupdateInterval = msettings->value(Preferences::UPDATE_INTERVAL_KEY).toInt() * 1000;
  if (mupdateInterval <= 0) mupdateInterval = MonitorBroker::DEFAULT_UPDATE_INTERVAL * 1000;
}

void SvNavigator::expandNode(const QString& _nodeId, const bool& _expand, const qint32& _level)
{
  auto node = mcoreData->bpnodes.find(_nodeId);
  if (node == mcoreData->bpnodes.end()) return;
  if (node->child_nodes != "") {
      QStringList  childNodes = node->child_nodes.split(Parser::CHILD_SEP);
      for (auto udsIt = childNodes.begin(); udsIt != childNodes.end(); udsIt++) {
          mmap->setNodeVisible(*udsIt, _nodeId, _expand, _level);
        }
    }
}

void SvNavigator::centerGraphOnNode(const QString& _node_id)
{
  if (_node_id != "") mselectedNode =  _node_id;
  mmap->centerOnNode(mselectedNode);
}

void SvNavigator::filterNodeRelatedMsg(void)
{
  if (mfilteredMsgPanel) delete mfilteredMsgPanel;
  mfilteredMsgPanel = new MsgConsole();
  NodeListT::iterator node;
  if (utils::findNode(mcoreData, mselectedNode, node)) {
      filterNodeRelatedMsg(mselectedNode);
      QString title = tr("Messages related to '%2' - %1")
          .arg(appName)
          .arg(node->name);
      mfilteredMsgPanel->updateColumnWidths(mmsgConsoleSize, true);
      mfilteredMsgPanel->setWindowTitle(title);
    }

  mfilteredMsgPanel->show();
}

void SvNavigator::filterNodeRelatedMsg(const QString& _nodeId)
{
  NodeListT::iterator node;
  if (utils::findNode(mcoreData, _nodeId, node) &&
      node->child_nodes != "")  // Warning: take care in short-circuit evaluation!!!
    {
      if (node->type == NodeType::ALARM_NODE) {
          mfilteredMsgPanel->updateNodeMsg(node);
        } else {
          QStringList childIds = node->child_nodes.split(Parser::CHILD_SEP);
          foreach(QString chkid, childIds) {
              filterNodeRelatedMsg(chkid);
            }
        }
    }
}


void SvNavigator::acknowledge(void)
{
  //TODO: To be implemented
}

void SvNavigator::tabChanged(int _index)
{
  switch(_index) {
    case 0:
      msubMenus["Refresh"]->setVisible(true);
      msubMenus["Capture"]->setVisible(true);
      msubMenus["ZoomIn"]->setVisible(true);
      msubMenus["ZoomOut"]->setVisible(true);
      mmenus["BROWSER"]->setEnabled(false);
      msubMenus["BrowserBack"]->setVisible(false);
      msubMenus["BrowserForward"]->setVisible(false);
      msubMenus["BrowserStop"]->setVisible(false);
      break;
    case 1:
      mmenus["BROWSER"]->setEnabled(true);
      msubMenus["BrowserBack"]->setVisible(true);
      msubMenus["BrowserForward"]->setVisible(true);
      msubMenus["BrowserStop"]->setVisible(true);
      msubMenus["Refresh"]->setVisible(false);
      msubMenus["Capture"]->setVisible(false);
      msubMenus["ZoomIn"]->setVisible(false);
      msubMenus["ZoomOut"]->setVisible(false);
      break;
    default:
      break;

    }
}

void SvNavigator::hideChart(void)
{
  if (mmap->hideChart()) {
      msubMenus["HideChart"]->setIcon(QIcon(":images/check.png"));
      return;
    }
  msubMenus["HideChart"]->setIcon(QIcon(""));
}

void SvNavigator::centerGraphOnNode(QTreeWidgetItem * _item)
{
  centerGraphOnNode(_item->data(0, QTreeWidgetItem::UserType).toString());
}

void SvNavigator::resize(void)
{
  const qreal GRAPH_HEIGHT_RATE = 0.50;
  QSize screenSize = qApp->desktop()->screen(0)->size();
  mmsgConsoleSize = QSize(screenSize.width() * 0.80, screenSize.height() * (1.0 - GRAPH_HEIGHT_RATE));

  QList<qint32> framesSize;
  framesSize.push_back(screenSize.width() * 0.20);
  framesSize.push_back(mmsgConsoleSize.width());
  mmainSplitter->setSizes(framesSize);

  framesSize[0] = (screenSize.height() * GRAPH_HEIGHT_RATE);
  framesSize[1] = (mmsgConsoleSize.height());
  mrightSplitter->setSizes(framesSize);

  mmainSplitter->resize(screenSize.width(), screenSize.height() * 0.85);
  QMainWindow::resize(screenSize.width(),  screenSize.height());
}


void SvNavigator::closeZbxSession(void)
{
  QStringList params;
  params.push_back(mzbxAuthToken);
  params.push_back(QString::number(ZbxHelper::LOGOUT));
  mzbxHelper->postRequest(ZbxHelper::LOGOUT, params);
}

void SvNavigator::processZbxReply(QNetworkReply* _reply)
{
  _reply->deleteLater();
  if (_reply->error() != QNetworkReply::NoError) return;

  QString data = _reply->readAll();
  JsonHelper jsHelper(data.toStdString());
  qint32 transaction = jsHelper.getProperty("id").toInt32();
  switch(transaction) {
    case ZbxHelper::LOGIN :
      mzbxAuthToken = jsHelper.getProperty("result").toString();
      if (! mzbxAuthToken.isEmpty())
        misLogged = true;
      postRpcDataRequest();
      break;
    case ZbxHelper::TRIGGER: {
        QScriptValueIterator trigger(jsHelper.getProperty("result"));
        while (trigger.hasNext()) {
            trigger.next();
            if (trigger.flags()&QScriptValue::SkipInEnumeration) continue;
            QScriptValue triggerData = trigger.value();
            MonitorBroker::CheckT check;
            QString triggerName = triggerData.property("description").toString();
            check.check_command = triggerName.toStdString();
            check.status = triggerData.property("value").toInt32();
            if (check.status == MonitorBroker::ZABBIX_CLEAR) {
                check.alarm_msg = triggerName.toStdString(); //FIXME: use another parameter?
              } else {
                check.alarm_msg = triggerData.property("error").toString().toStdString();
                check.status = triggerData.property("priority").toInteger();
              }
            QString targetHost = "";
            QScriptValueIterator host(triggerData.property("hosts"));
            if (host.hasNext()) {
                host.next();
                QScriptValue hostData = host.value();
                targetHost = hostData.property("host").toString();
                check.host = targetHost.toStdString();
              }
            QScriptValueIterator item(triggerData.property("items"));
            if (item.hasNext()) {
                item.next();
                QScriptValue itemData = item.value();
                check.last_state_change = utils::getCtime(itemData.property("lastclock").toUInt32());
              }
            QString key = ID_PATTERN.arg(targetHost).arg(triggerName);
            check.id = key.toStdString();
            updateCNodes(check);
          }
        if (--mhostLeft == 0) {
            mupdateSucceed = true;
            finalizeDashboardUpdate();
          }
        break;
      }
    default :
      utils::alert(tr("Weird response received from the server"));
      exit(1);
      break;
    }
}


void SvNavigator::processZnsReply(QNetworkReply* _reply)
{
  _reply->deleteLater();
  if (_reply->error() != QNetworkReply::NoError) return;

  QVariant cookiesContainer = _reply->header(QNetworkRequest::SetCookieHeader);
  QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(cookiesContainer);
  QString data = _reply->readAll();
  if (data.endsWith("submitted=true")) {
      misLogged = true;
      postRpcDataRequest();
      mznsHelper->cookieJar()->setCookiesFromUrl(cookies, mznsHelper->getApiBaseUrl());
    } else {
      JsonHelper jsonHelper(data.toStdString());
      qint32 transaction = jsonHelper.getProperty("tid").toInt32();
      QScriptValue result = jsonHelper.getProperty("result");
      QString successMsg = result.property("success").toString();
      if (successMsg.compare("true") != 0) {
          qDebug() << data;
          QString msg = result.property("msg").toString();
          if (msg.isEmpty()) msg = "Authentication failed!";
          updateDashboardOnUnknown(msg);
          return;
        }
      if (transaction == ZnsHelper::DEVICE) {
          QScriptValueIterator devices(result.property("devices"));
          while(devices.hasNext()) {
              devices.next();
              if (devices.flags()&QScriptValue::SkipInEnumeration)
                continue;
              QScriptValue item = devices.value();
              QString uid = item.property("uid").toString();
              mznsHelper->postRequest(ZnsHelper::COMPONENT,
                                      ZnsHelper::ReQPatterns[ZnsHelper::COMPONENT]
                                      .arg(uid)
                                      .arg(ZnsHelper::COMPONENT)
                                      .toAscii());
            }
        } else if (transaction == ZnsHelper::COMPONENT) {
          QScriptValueIterator components(result.property("data"));
          while(components.hasNext()) {
              components.next();
              if (components.flags()&QScriptValue::SkipInEnumeration)
                continue;
              MonitorBroker::CheckT check;
              QScriptValue item = components.value();
              QString name = item.property("name").toString();
              QScriptValue device = item.property("device");
              QString duid = device.property("uid").toString();
              QString chkid = ID_PATTERN.arg(ZnsHelper::getDeviceName(duid)).arg(name);
              check.id = chkid.toStdString();
              check.host = device.property("name").toString().toStdString();
              check.last_state_change = utils::getCtime(device.property("lastChanged").toString(),
                                                        "yyyy/MM/dd hh:mm:ss");
              QString severity =item.property("severity").toString();
              if (severity.toLower().compare("clear") == 0) {
                  check.status = MonitorBroker::ZENOSS_CLEAR;
                  check.alarm_msg = "Up";
                } else {
                  check.status = item.property("failSeverity").toInt32();
                  check.alarm_msg = item.property("status").toString().toStdString();
                }
              updateCNodes(check);
            }
          if (--mhostLeft== 0) {
              mupdateSucceed = true;
              finalizeDashboardUpdate();
            }
        } else {
          updateDashboardOnUnknown(tr("Unexpected response received from the server"));
        }
    }
}

void SvNavigator::openRpcSession(void)
{
  updateDashboardOnUnknown("");
  QStringList authParams = getAuthInfo();
  if (authParams.size() == 2) {
      QUrl znsUrlParams;
      switch(mcoreData->monitor) {
        case MonitorBroker::ZABBIX:
          mzbxHelper->setBaseUrl(mmonitorBaseUrl);
          authParams.push_back(QString::number(ZbxHelper::LOGIN));
          mzbxHelper->postRequest(ZbxHelper::LOGIN, authParams);
          break;
        case MonitorBroker::ZENOSS:
          mznsHelper->setBaseUrl(mmonitorBaseUrl);
          znsUrlParams.addQueryItem("__ac_name", authParams[0]);
          znsUrlParams.addQueryItem("__ac_password", authParams[1]);
          znsUrlParams.addQueryItem("submitted", "true");
          znsUrlParams.addQueryItem("came_from", mznsHelper->getApiContextUrl());
          mznsHelper->postRequest(ZnsHelper::LOGIN, znsUrlParams.encodedQuery());
          break;
        default:
          break;
        }
    } else {
      updateDashboardOnUnknown(tr("Invalid authentication chain!\n"
                                  "Must be in the form login:password"));
    }
}

void SvNavigator::postRpcDataRequest(void) {
  updateStatusBar(tr("Updating..."));
  switch(mcoreData->monitor) {
    case MonitorBroker::ZABBIX:
      foreach(const QString& host, mcoreData->hosts.keys()) {
          QStringList params;
          params.push_back(mzbxAuthToken);
          params.push_back(host);
          params.push_back(QString::number(ZbxHelper::TRIGGER));
          mzbxHelper->postRequest(ZbxHelper::TRIGGER, params);
        }
      break;
    case MonitorBroker::ZENOSS:
      mznsHelper->setRouter(ZnsHelper::DEVICE);
      foreach(const QString& host, mcoreData->hosts.keys()) {
          mznsHelper->postRequest(ZnsHelper::DEVICE,
                                  ZnsHelper::ReQPatterns[ZnsHelper::DEVICE]
                                  .arg(host)
                                  .arg(ZnsHelper::DEVICE)
                                  .toAscii());
        }
      break;
    default:
      break;
    }
}


void SvNavigator::processRpcError(QNetworkReply::NetworkError _code)
{
  QString apiUrl = "";
  if (mcoreData->monitor == MonitorBroker::ZABBIX) {
      apiUrl = mzbxHelper->getApiUri();
    } else if (mcoreData->monitor == MonitorBroker::ZENOSS) {
      apiUrl =  mznsHelper->getRequestUrl();
    }
  updateDashboardOnUnknown(SERVICE_OFFLINE_MSG.arg(apiUrl%tr(" (error code %1)").arg(_code)));
}

void SvNavigator::updateDashboardOnUnknown(const QString& msg)
{
  mupdateSucceed = false;
  bool enable = false;
  if (!msg.isEmpty()) {
      enable = true;
      utils::alert(msg);
      updateStatusBar(msg);
      mlastError = msg;
    }
  for (auto cnode = mcoreData->cnodes.begin(); cnode != mcoreData->cnodes.end(); cnode++) {
      //FIXME: clean undefined services on console when the server recome on service
      cnode->check.status = MonitorBroker::CRITICITY_UNKNOWN;
      cnode->check.host = "Unknown";
      cnode->check.last_state_change = UNKNOWN_UPDATE_TIME;
      cnode->check.check_command = "Unknown";
      cnode->check.alarm_msg = mlastError.toStdString();
      cnode->criticity = MonitorBroker::CRITICITY_UNKNOWN;
      computeStatusInfo(cnode);
      updateDashboard(cnode);
    }
  mcoreData->check_status_count[MonitorBroker::CRITICITY_UNKNOWN] = mcoreData->cnodes.size();
  finalizeDashboardUpdate(enable);
}

QStringList SvNavigator::getAuthInfo(void) {

  QStringList authInfo = QStringList();
  int pos = mserverAuthChain.indexOf(":");
  if (pos != -1) {
      authInfo.push_back(mserverAuthChain.left(pos));
      authInfo.push_back(mserverAuthChain.mid(pos+1, -1));
    }
  return authInfo;
}


void SvNavigator::addEvents(void)
{
  connect(this, SIGNAL(hasToBeUpdate(QString)), this, SLOT(updateBpNode(QString)));
  connect(msubMenus["Quit"], SIGNAL(triggered(bool)), qApp, SLOT(quit()));
  connect(msubMenus["Capture"], SIGNAL(triggered(bool)), mmap, SLOT(capture()));
  connect(msubMenus["ZoomIn"], SIGNAL(triggered(bool)), mmap, SLOT(zoomIn()));
  connect(msubMenus["ZoomOut"], SIGNAL(triggered(bool)), mmap, SLOT(zoomOut()));
  connect(msubMenus["HideChart"], SIGNAL(triggered(bool)), this, SLOT(hideChart()));
  connect(msubMenus["Refresh"], SIGNAL(triggered(bool)), this, SLOT(startMonitor()));
  connect(msubMenus["ChangePassword"], SIGNAL(triggered(bool)), this, SLOT(handleChangePasswordAction(void)));
  connect(msubMenus["ChangeMonitoringSettings"], SIGNAL(triggered(bool)), this, SLOT(handleChangeMonitoringSettingsAction(void)));
  connect(msubMenus["ShowAbout"], SIGNAL(triggered(bool)), this, SLOT(handleShowAbout()));
  connect(msubMenus["ShowOnlineResources"], SIGNAL(triggered(bool)), this, SLOT(handleShowOnlineResources()));
  connect(msubMenus["BrowserBack"], SIGNAL(triggered(bool)), mbrowser, SLOT(back()));
  connect(msubMenus["BrowserForward"], SIGNAL(triggered(bool)), mbrowser, SLOT(forward()));
  connect(msubMenus["BrowserStop"], SIGNAL(triggered(bool)), mbrowser, SLOT(stop()));
  connect(mcontextMenuList["FilterNodeRelatedMessages"], SIGNAL(triggered(bool)), this, SLOT(filterNodeRelatedMsg()));
  connect(mcontextMenuList["CenterOnNode"], SIGNAL(triggered(bool)), this, SLOT(centerGraphOnNode()));
  connect(mprefWindow, SIGNAL(urlChanged(QString)), mbrowser, SLOT(setUrl(QString)));
  connect(mviewPanel, SIGNAL(currentChanged (int)), this, SLOT(tabChanged(int)));
  connect(mmap, SIGNAL(expandNode(QString, bool, qint32)), this, SLOT(expandNode(const QString &, const bool &, const qint32 &)));
  connect(mtree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(centerGraphOnNode(QTreeWidgetItem *)));
  connect(mzbxHelper, SIGNAL(finished(QNetworkReply*)), this, SLOT(processZbxReply(QNetworkReply*)));
  connect(mzbxHelper, SIGNAL(propagateError(QNetworkReply::NetworkError)), this, SLOT(processRpcError(QNetworkReply::NetworkError)));
  connect(mznsHelper, SIGNAL(finished(QNetworkReply*)), this, SLOT(processZnsReply(QNetworkReply*)));
  connect(mznsHelper, SIGNAL(propagateError(QNetworkReply::NetworkError)), this, SLOT(processRpcError(QNetworkReply::NetworkError)));
}
