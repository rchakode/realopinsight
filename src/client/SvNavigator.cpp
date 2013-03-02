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
#include "client/utilsClient.hpp"
#include "client/JsHelper.hpp"
#include "client/MkLsHelper.hpp"
#include <QScriptValueIterator>
#include <QSystemTrayIcon>
#include <sstream>
#include <QStatusBar>
#include <QObject>
#include <zmq.h>
#include <iostream>
#include <locale>
#include <memory>

//FIXME: test case unsensitive check id with Nagios, Zabbix, and Zenoss

const QString DEFAULT_TIP_PATTERN(QObject::tr("Service: %1\nDescription: %2\nSeverity: %3\n   Calc. Rule: %4\n   Prop. Rule: %5"));
const QString ALARM_SPECIFIC_TIP_PATTERN(QObject::tr("\nTarget Host: %6\nData Point: %7\nRaw Output: %8\nOther Details: %9"));
const QString SERVICE_OFFLINE_MSG(QObject::tr("Failed to connect to %1"));
const QString DEFAULT_ERROR_MSG("{\"return_code\": \"-1\", \"message\": \""%SERVICE_OFFLINE_MSG%"\"}");
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
    mfilteredMsgConsole (NULL),
    mmainSplitter (new QSplitter(this)),
    mrightSplitter (new QSplitter()),
    mviewPanel (new QTabWidget()),
    mmsgConsolePanel (new QTabWidget()),
    mbrowser (new WebKit()),
    mmap (new GraphView(this)),
    mtree (new SvNavigatorTree()),
    mpreferences (new Preferences(_userRole, Preferences::ChangeMonitoringSettings)),
    mchangePasswdWindow (new Preferences(_userRole, Preferences::ChangePassword)),
    mmsgConsole(new MsgConsole(this)),
    mnodeContextMenu (new QMenu()),
    mzbxHelper(new ZbxHelper()),
    mzbxAuthToken(""),
    mhostLeft(0),
    mznsHelper(new ZnsHelper()),
    misLogged(false),
    mlastError(""),
    mtrayIcon(new QSystemTrayIcon(QIcon(":images/built-in/icon.png")))
{
  setWindowTitle(tr("%1 Operations Console").arg(APP_NAME));
  loadMenus();
  mviewPanel->addTab(mmap, tr("Dashboard"));
  mviewPanel->addTab(mbrowser, tr("Native Web UI"));
  mmsgConsolePanel->addTab(mmsgConsole, tr("Message Console"));
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
  if (mfilteredMsgConsole) delete mfilteredMsgConsole;
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
  delete mpreferences;
  delete mchangePasswdWindow;
  delete mzbxHelper;
  delete mznsHelper;
  delete mtrayIcon;
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
      msubMenus["ShowAbout"] = mmenus["HELP"]->addAction(tr("&About %1").arg(APP_NAME));
  msubMenus["Capture"]->setShortcut(QKeySequence::Save);
  msubMenus["Refresh"]->setShortcut(QKeySequence::Refresh);
  msubMenus["ZoomIn"]->setShortcut(QKeySequence::ZoomIn);
  msubMenus["ZoomOut"]->setShortcut(QKeySequence::ZoomOut);
  msubMenus["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
  msubMenus["Quit"]->setShortcut(QKeySequence::Quit);

  mcontextMenuList["FilterNodeRelatedMessages"] = mnodeContextMenu->addAction(tr("&Filter related messages"));
  mcontextMenuList["CenterOnNode"] = mnodeContextMenu->addAction(tr("Center Graph &On"));
  mcontextMenuList["Cancel"] = mnodeContextMenu->addAction(tr("&Cancel"));

  QToolBar* toolBar = addToolBar(APP_NAME);
  toolBar->setIconSize(QSize(16,16));
  toolBar->addAction(msubMenus["Refresh"]);
  toolBar->addAction(msubMenus["ZoomIn"]);
  toolBar->addAction(msubMenus["ZoomOut"]);
  toolBar->addAction(msubMenus["Capture"]);
  toolBar->addSeparator();
  toolBar->addAction(msubMenus["BrowserBack"]);
  toolBar->addAction(msubMenus["BrowserForward"]);
  toolBar->addAction(msubMenus["BrowserStop"]);
  setMenuBar(menuBar);
}

void SvNavigator::closeEvent(QCloseEvent * event)
{
  if (mfilteredMsgConsole) mfilteredMsgConsole->close();
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
    case MonitorBroker::Zenoss:
    case MonitorBroker::Zabbix:
      !misLogged ? openRpcSession(): postRpcDataRequest();
      break;
    case MonitorBroker::Nagios:
    default:
      mpreferences->useMkls()? runMklsMonitor() : runNagiosMonitor();
      break;
    }
}

void SvNavigator::timerEvent(QTimerEvent *)
{
  startMonitor();
}

void  SvNavigator::updateStatusBar(const QString& msg)
{
  statusBar()->showMessage(msg);
}

void SvNavigator::load(const QString& _file)
{
  if (!_file.isEmpty())
    mconfigFile = utils::getAbsolutePath(_file);
  mactiveFile = mconfigFile;
  QMainWindow::setWindowTitle(tr("%1 Operations Console - %2").arg(APP_NAME, mconfigFile));
  Parser parser;
  parser.parseSvConfig(mconfigFile, *mcoreData);
  mtree->clear();
  mtree->addTopLevelItem(mcoreData->tree_items[SvNavigatorTree::RootId]);
  mmap->load(parser.getDotGraphFile(), mcoreData->bpnodes, mcoreData->cnodes);
  mbrowser->setUrl(mmonitorBaseUrl);
  this->resizeDashboard();
  QMainWindow::show();
  mmap->scaleToFitViewPort();
  mtrayIcon->show();
  mtrayIcon->setToolTip(APP_NAME);
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
  mpreferences->exec();
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
  auto socket = std::unique_ptr<ZmqSocket>(new ZmqSocket(mserverUrl.toStdString(), ZMQ_REQ));
  if(socket->connect())
    socket->makeHandShake();
  if (socket->isConnected2Server()) {
      if (socket->getServerSerial() < 111) {
          utils::alert(tr("The server serial %1 is not supported").arg(socket->getServerSerial()));
          mupdateSucceed = false;
        }
      updateStatusBar(tr("Updating..."));
    } else {
      mupdateSucceed = false;
      QString msg = SERVICE_OFFLINE_MSG.arg(mserverUrl);
      utils::alert(msg);
      updateStatusBar(msg);
    }

  for (auto& cnode : mcoreData->cnodes) {
      if (cnode.child_nodes == "") {
          cnode.severity = MonitorBroker::Unknown;
          mcoreData->check_status_count[cnode.severity]++;
          continue;
        }

      QStringList checks = cnode.child_nodes.split(Parser::CHILD_SEP);
      for(auto cid : checks) {
          QString msg = mserverAuthChain%":"%cid;
          if (mupdateSucceed) {
              socket->send(msg.toStdString());
              msg = QString::fromStdString(socket->recv());
            } else {
              msg = DEFAULT_ERROR_MSG.arg(mserverUrl);
            }
          JsonHelper jsHelper(msg.toStdString());
          if (!jsHelper.getProperty("return_code").toInt32() && socket->isConnected2Server()) {
              cnode.check.status = jsHelper.getProperty("status").toInt32();
              cnode.check.host = jsHelper.getProperty("host").toString().toStdString();
              cnode.check.last_state_change = utils::getCtime(jsHelper.getProperty("lastchange").toUInt32());
              cnode.check.check_command = jsHelper.getProperty("command").toString().toStdString();
              cnode.check.alarm_msg = jsHelper.getProperty("message").toString().toStdString();
            } else {
              cnode.check.status = MonitorBroker::NagiosUnknown;
              cnode.check.last_state_change = UNKNOWN_UPDATE_TIME;
              cnode.check.host = "-";
              cnode.check.check_command = "-";
              cnode.check.alarm_msg = jsHelper.getProperty("message").toString().toStdString();
            }
          cnode.monitored = true;
          computeStatusInfo(cnode);
          updateDashboard(cnode);
          mcoreData->check_status_count[cnode.severity]++;
        }
    }
  socket.reset(NULL);
  finalizeDashboardUpdate();
  return 0;
}

int SvNavigator::runMklsMonitor(void)
{
  MkLsHelper mklsHelper(mserverAddr, mserverPort.toInt());
  if (!mklsHelper.connectToService()) {
      mupdateSucceed = false;
      updateDashboardOnUnknown(mklsHelper.errorString());
      return 1;
    }
  CheckT invalidCheck;
  invalidCheck.status = MonitorBroker::NagiosUnknown;
  invalidCheck.last_state_change = UNKNOWN_UPDATE_TIME;
  invalidCheck.host = "-";
  invalidCheck.check_command = "-";
  invalidCheck.alarm_msg = "Service not found";
  QHashIterator<QString, QStringList> hit(mcoreData->hosts);
  while (hit.hasNext()) {
      hit.next();
      QString host = hit.key();
      if (mklsHelper.loadHostData(host)) {
          foreach (const QString& item, hit.value()) {
              QString cid;
              if (item == "ping") {
                  cid = host;
                } else {
                  cid = ID_PATTERN.arg(host).arg(item);
                }
              //qDebug() << "FINDDDDDDDDDD>> "<<cid;
              CheckListCstIterT chkit;
              if (mklsHelper.findCheck(cid, chkit)) {
                  qDebug() << cid;
                  updateCNodes(*chkit);
                } else {
                  invalidCheck.alarm_msg = tr("Service not found: %1").arg(cid).toStdString();
                  updateCNodes(invalidCheck);
                }
            }
        }
    }
  finalizeDashboardUpdate();
  return 0;
}

void SvNavigator::prepareDashboardUpdate(void)
{
  QMainWindow::setEnabled(false);
  mcoreData->check_status_count[MonitorBroker::Normal] = 0;
  mcoreData->check_status_count[MonitorBroker::Minor] = 0;
  mcoreData->check_status_count[MonitorBroker::Major] = 0;
  mcoreData->check_status_count[MonitorBroker::Critical] = 0;
  mcoreData->check_status_count[MonitorBroker::Unknown] = 0;
  mhostLeft = mcoreData->hosts.size();
  mupdateSucceed = true;
  QString msg = QObject::tr("Connecting to %1...");
  switch(mcoreData->monitor) {
    case MonitorBroker::Nagios:
      msg = msg.arg(mserverUrl);
      break;
    case MonitorBroker::Zabbix:
      msg = msg.arg(mzbxHelper->getApiUri());
      break;
    case MonitorBroker::Zenoss:
      msg = msg.arg(mznsHelper->getApiBaseUrl()); //FIXME: msg.arg(mznsHelper->getApiContextUrl()) crashes
      break;
    default:
      break;
    }
  updateStatusBar(msg);
}

QString SvNavigator::getNodeToolTip(const NodeT& _node)
{
  QString toolTip = DEFAULT_TIP_PATTERN.arg(_node.name,
                                            const_cast<QString&>(_node.description).replace("\n", " "),
                                            utils::criticityToText(_node.severity),
                                            CalcRules::label(_node.sev_crule),
                                            PropRules::label(_node.sev_prule));

  if (_node.type == NodeType::ALARM_NODE) {
      QString msg = "";
      if (_node.severity == MonitorBroker::Normal) {
          msg = const_cast<QString&>(_node.notification_msg).replace("\n", " ");
        } else {
          msg = QString::fromStdString(_node.check.alarm_msg).replace("\n", " ");
        }
      toolTip += ALARM_SPECIFIC_TIP_PATTERN.arg(QString::fromStdString(_node.check.host).replace("\n", " "),
                                                _node.child_nodes,
                                                QString::fromStdString(_node.check.alarm_msg).replace("\n", " "),
                                                msg);
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

void SvNavigator::updateCNodes(const CheckT& check)
{
  for (auto& cnode : mcoreData->cnodes) {
      if (cnode.child_nodes.toStdString() == check.id) {
          cnode.check = check;
          computeStatusInfo(cnode);
          mcoreData->check_status_count[cnode.severity]++;
          updateDashboard(cnode);
          cnode.monitored = true;
        }
    }
}

void SvNavigator::finalizeDashboardUpdate(const bool& enable)
{
  if (!mcoreData->cnodes.isEmpty()) {
      Chart *chart = new Chart;
      QString chartdDetails = chart->update(mcoreData->check_status_count, mcoreData->cnodes.size());
      mmap->updateStatsPanel(chart);
      if (mchart) delete mchart; mchart = chart; mchart->setToolTip(chartdDetails);
      mmsgConsole->sortByColumn(1, Qt::AscendingOrder);
      mmsgConsole->updateColumnWidths(mmsgConsoleSize);
      mupdateInterval = msettings->value(Preferences::UPDATE_INTERVAL_KEY).toInt();
      mupdateInterval = 1000*((mupdateInterval > 0)? mupdateInterval:MonitorBroker::DefaultUpdateInterval);
      mtimer = startTimer(mupdateInterval);
      if (mupdateSucceed) updateStatusBar(tr("Update completed"));
      for (auto& cnode : mcoreData->cnodes) {
          if (!cnode.monitored) {
              cnode.check.status = MonitorBroker::Unknown;
              cnode.check.last_state_change = UNKNOWN_UPDATE_TIME;
              cnode.check.host = "-";
              cnode.check.alarm_msg = tr("Unknown service %1").arg(cnode.child_nodes).toStdString();
              computeStatusInfo(cnode);
              mcoreData->check_status_count[cnode.severity]++;
              updateDashboard(cnode);
              cnode.monitored = true;
            }
          cnode.monitored = false;
        }
    }
  if (!mcoreData->bpnodes.isEmpty())
    updateTrayInfo(mcoreData->bpnodes[SvNavigatorTree::RootId]); //FIXME: avoid searching at each update
  QMainWindow::setEnabled(enable);
}

void SvNavigator::computeStatusInfo(NodeListT::iterator&  _node)
{
  computeStatusInfo(*_node);
}

void SvNavigator::computeStatusInfo(NodeT& _node)
{
  _node.severity = utils::computeCriticity(mcoreData->monitor, _node.check.status);
  _node.prop_sev = utils::computePropCriticity(_node.severity, _node.sev_prule);
  QString statusText = (_node.severity == MonitorBroker::Normal)? _node.notification_msg : _node.alarm_msg;
  if (statusText.trimmed().isEmpty()) {
      return;
    }
  QRegExp regexp(MsgConsole::TAG_HOSTNAME);
  statusText.replace(regexp, _node.check.host.c_str()); //FIXME: problem with '-' host

  auto info = QString(_node.check.id.c_str()).split("/");
  if (info.length() > 1) {
      regexp.setPattern(MsgConsole::TAG_CHECK);
      statusText.replace(regexp, info[1]);
    }
  if (mcoreData->monitor == MonitorBroker::Nagios) { // FIXME: Threshold tag don't work for other monitors
      info = QString(_node.check.check_command.c_str()).split("!");
      if (info.length() >= 3) {
          regexp.setPattern(MsgConsole::TAG_THERESHOLD);
          statusText.replace(regexp, info[1]);
          if (_node.severity == MonitorBroker::Major)
            statusText.replace(regexp, info[2]);
        }
    }
  if (mcoreData->monitor == MonitorBroker::Zabbix) {
      regexp.setPattern(MsgConsole::TAG_HOSTNAME_ZABBIX);
      statusText.replace(regexp, _node.check.host.c_str());
      _node.check.alarm_msg = QString(_node.check.alarm_msg.c_str()).replace(regexp, _node.check.host.c_str()).toStdString();
    }
  _node.check.alarm_msg = statusText.toStdString();
}

void SvNavigator::updateBpNode(const QString& _nodeId)
{
  NodeListT::iterator node;
  if (!utils::findNode(mcoreData, _nodeId, node)) return;

  QStringList nodeIds = node->child_nodes.split(Parser::CHILD_SEP);
  Criticity criticity;
  for (auto nodeId : nodeIds) {
      NodeListT::iterator child;
      if (!utils::findNode(mcoreData, nodeId, child)) continue;
      Criticity cst(static_cast<MonitorBroker::SeverityT>(child->prop_sev));
      if (node->sev_crule == CalcRules::WeightedCriticity) {
          criticity = criticity / cst;
        } else {
          criticity = criticity * cst;
        }
    }
  node->severity = criticity.getValue();
  switch(node->sev_prule) {
    case PropRules::Increased: node->prop_sev = (criticity++).getValue();
      break;
    case PropRules::Decreased: node->prop_sev = (criticity--).getValue();
      break;
    default: node->prop_sev = node->severity;
      break;
    }
  QString toolTip = getNodeToolTip(*node);
  mmap->updateNode(node, toolTip);
  updateNavTreeItemStatus(node, toolTip);
  if (node->id != SvNavigatorTree::RootId) emit hasToBeUpdate(node->parent);
}

void SvNavigator::updateNavTreeItemStatus(const NodeListT::iterator& _node, const QString& _tip)
{
  updateNavTreeItemStatus(*_node, _tip);
}

void SvNavigator::updateNavTreeItemStatus(const NodeT& _node, const QString& _tip)
{
  auto tnode_it = mcoreData->tree_items.find(_node.id);
  if (tnode_it != mcoreData->tree_items.end()) {
      (*tnode_it)->setIcon(0, utils::computeCriticityIcon(_node.severity));
      (*tnode_it)->setToolTip(0, _tip);
    }
}

void SvNavigator::updateMonitoringSettings() {
  mmonitorBaseUrl = msettings->value(Preferences::URL_KEY).toString();
  mserverAuthChain = msettings->value(Preferences::SERVER_PASS_KEY).toString();
  mserverAddr = msettings->value(Preferences::SERVER_ADDR_KEY).toString();
  mserverPort = msettings->value(Preferences::SERVER_PORT_KEY).toString();
  mserverUrl = QString("tcp://%1:%2").arg(mserverAddr, mserverPort);
  mupdateInterval = msettings->value(Preferences::UPDATE_INTERVAL_KEY).toInt() * 1000;
  if (mupdateInterval <= 0) mupdateInterval = MonitorBroker::DefaultUpdateInterval * 1000;
}

void SvNavigator::expandNode(const QString& _nodeId, const bool& _expand, const qint32& _level)
{
  auto node = mcoreData->bpnodes.find(_nodeId);
  if (node == mcoreData->bpnodes.end()) return;
  if (node->child_nodes != "") {
      QStringList  childNodes = node->child_nodes.split(Parser::CHILD_SEP);
      for (auto cid : childNodes) {
          mmap->setNodeVisible(cid, _nodeId, _expand, _level);
        }
    }
}

void SvNavigator::centerGraphOnNode(const QString& _nodeId)
{
  if (_nodeId != "") mselectedNode =  _nodeId;
  mmap->centerOnNode(mselectedNode);
}

void SvNavigator::filterNodeRelatedMsg(void)
{
  if (mfilteredMsgConsole) delete mfilteredMsgConsole;
  mfilteredMsgConsole = new MsgConsole();
  NodeListT::iterator node;
  if (utils::findNode(mcoreData, mselectedNode, node)) {
      filterNodeRelatedMsg(mselectedNode);
      QString title = tr("Messages related to '%2' - %1").arg(APP_NAME, node->name);
      mfilteredMsgConsole->updateColumnWidths(mmsgConsoleSize, true);
      mfilteredMsgConsole->setWindowTitle(title);
    }
  qint32 rh = qMax(mfilteredMsgConsole->getRowCount() * mfilteredMsgConsole->getRowHeight() + 50, 100);
  if (mfilteredMsgConsole->height() > rh) mfilteredMsgConsole->resize(mmsgConsoleSize.width(), rh);
  mfilteredMsgConsole->show();
}

void SvNavigator::filterNodeRelatedMsg(const QString& _nodeId)
{
  NodeListT::iterator node;
  if (utils::findNode(mcoreData, _nodeId, node) &&
      node->child_nodes != "") {
      if (node->type == NodeType::ALARM_NODE) {
          mfilteredMsgConsole->updateNodeMsg(node);
        } else {
          QStringList childIds = node->child_nodes.split(Parser::CHILD_SEP);
          for (auto chkid : childIds) {
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
      msubMenus["Refresh"]->setEnabled(true);
      msubMenus["Capture"]->setEnabled(true);
      msubMenus["ZoomIn"]->setEnabled(true);
      msubMenus["ZoomOut"]->setEnabled(true);
      mmenus["BROWSER"]->setEnabled(false);
      msubMenus["BrowserBack"]->setEnabled(false);
      msubMenus["BrowserForward"]->setEnabled(false);
      msubMenus["BrowserStop"]->setEnabled(false);
      break;
    case 1:
      mmenus["BROWSER"]->setEnabled(true);
      msubMenus["BrowserBack"]->setEnabled(true);
      msubMenus["BrowserForward"]->setEnabled(true);
      msubMenus["BrowserStop"]->setEnabled(true);
      msubMenus["Refresh"]->setEnabled(false);
      msubMenus["Capture"]->setEnabled(false);
      msubMenus["ZoomIn"]->setEnabled(false);
      msubMenus["ZoomOut"]->setEnabled(false);
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

void SvNavigator::resizeDashboard(void)
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

void SvNavigator::closeRpcSession(void)
{
  QStringList params;
  params.push_back(mzbxAuthToken);
  params.push_back(QString::number(ZbxHelper::Logout));
  mzbxHelper->postRequest(ZbxHelper::Logout, params);
}

void SvNavigator::processZbxReply(QNetworkReply* _reply)
{
  _reply->deleteLater();
  if (_reply->error() != QNetworkReply::NoError) {
      updateDashboardOnUnknown(_reply->errorString());
      return;
    }
  QString data = _reply->readAll();
  JsonHelper jsHelper(data.toStdString());
  qint32 tid = jsHelper.getProperty("id").toInt32();
  switch(tid) {
    case ZbxHelper::Login : {
        mzbxAuthToken = jsHelper.getProperty("result").toString();
        if(mzbxAuthToken.isEmpty()) {
            QString errMsg = jsHelper.getProperty("error").property("data").toString();
            if (errMsg.isEmpty()) errMsg = jsHelper.getProperty("error").property("message").toString();
            QString msg = tr("Authentication failed: %1").arg(errMsg);
            updateDashboardOnUnknown(msg);
          } else {
            misLogged = true;
            postRpcDataRequest();
          }
        break;
      }
    case ZbxHelper::Trigger: {
        CheckT check;
        QScriptValueIterator trigger(jsHelper.getProperty("result"));
        while (trigger.hasNext()) {
            trigger.next(); if (trigger.flags()&QScriptValue::SkipInEnumeration) continue;
            QScriptValue triggerData = trigger.value();
            QString triggerName = triggerData.property("description").toString();
            check.check_command = triggerName.toStdString();
            check.status = triggerData.property("value").toInt32();
            if (check.status == MonitorBroker::ZabbixClear) {
                check.alarm_msg = "OK ("+triggerName.toStdString()+")";
              } else {
                check.alarm_msg = triggerData.property("error").toString().toStdString();
                check.status = triggerData.property("priority").toInteger();
              }
            QString targetHost = "";
            QScriptValueIterator host(triggerData.property("hosts"));
            if (host.hasNext()) {
                host.next(); if (host.flags()&QScriptValue::SkipInEnumeration) continue;

                QScriptValue hostData = host.value();
                targetHost = hostData.property("host").toString();
                check.host = targetHost.toStdString();
              }
            QScriptValueIterator item(triggerData.property("items"));
            if (item.hasNext()) {
                item.next(); if (item.flags()&QScriptValue::SkipInEnumeration) continue;
                QScriptValue itemData = item.value();
                check.last_state_change = utils::getCtime(itemData.property("lastclock").toUInt32());
              }
            QString key = ID_PATTERN.arg(targetHost, triggerName);
            check.id = key.toLower().toStdString();
            updateCNodes(check);
          }
        if (--mhostLeft == 0) {
            mupdateSucceed = true;
            finalizeDashboardUpdate();
          }
        break;
      }
    default :
      QString msg = tr("Weird response received from the server");
      utils::alert(msg);
      updateDashboardOnUnknown(msg);
      break;
    }
}

void SvNavigator::processZnsReply(QNetworkReply* _reply)
{
  _reply->deleteLater();
  if (_reply->error() != QNetworkReply::NoError) {
      updateDashboardOnUnknown(_reply->errorString());
      return;
    }
  QVariant cookiesContainer = _reply->header(QNetworkRequest::SetCookieHeader);
  QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(cookiesContainer);
  QString data = _reply->readAll();
  if (data.endsWith("submitted=true")) {
      misLogged = true;
      postRpcDataRequest();
      mznsHelper->cookieJar()->setCookiesFromUrl(cookies, mznsHelper->getApiBaseUrl());
    } else {
      JsonHelper jsonHelper(data.toStdString());
      qint32 tid = jsonHelper.getProperty("tid").toInt32();
      QScriptValue result = jsonHelper.getProperty("result");
      bool reqSucceed = result.property("success").toBool();
      if (!reqSucceed) {
          QString msg = tr("Authentication failed: %1").arg(result.property("msg").toString());
          updateDashboardOnUnknown(msg);
          return;
        }
      if (tid == ZnsHelper::Device) {
          QScriptValueIterator devices(result.property("devices"));
          while(devices.hasNext()) {
              devices.next(); if (devices.flags()&QScriptValue::SkipInEnumeration) continue;

              QScriptValue ditem = devices.value();
              QString duid = ditem.property("uid").toString();
              mznsHelper->postRequest(ZnsHelper::Component,
                                      ZnsHelper::ReQPatterns[ZnsHelper::Component]
                                      .arg(duid, QString::number(ZnsHelper::Component))
                                      .toAscii());

              QString dname = ditem.property("name").toString();
              if (mcoreData->hosts[dname].contains("ping", Qt::CaseInsensitive)) {
                  mznsHelper->postRequest(ZnsHelper::Device,
                                          ZnsHelper::ReQPatterns[ZnsHelper::DeviceInfo]
                                          .arg(duid, QString::number(ZnsHelper::DeviceInfo))
                                          .toAscii());
                }
            }
        } else {
          CheckT check;
          if (tid == ZnsHelper::Component) {
              QScriptValueIterator components(result.property("data"));
              while (components.hasNext()) {
                  components.next(); if (components.flags()&QScriptValue::SkipInEnumeration) continue;
                  QScriptValue citem = components.value();
                  QString cname = citem.property("name").toString();
                  QScriptValue device = citem.property("device");
                  QString duid = device.property("uid").toString();
                  QString dname = ZnsHelper::getDeviceName(duid);
                  QString chkid = ID_PATTERN.arg(dname, cname);
                  check.id = chkid.toLower().toStdString();
                  check.host = dname.toStdString();
                  check.last_state_change = utils::getCtime(device.property("lastChanged").toString(),
                                                            "yyyy/MM/dd hh:mm:ss");
                  QString severity =citem.property("severity").toString();
                  if (!severity.compare("clear", Qt::CaseInsensitive)) {
                      check.status = MonitorBroker::ZenossClear;
                      check.alarm_msg = tr("The %1 component is Up").arg(cname).toStdString();
                    } else {
                      check.status = citem.property("failSeverity").toInt32();
                      check.alarm_msg = citem.property("status").toString().toStdString();
                    }
                  updateCNodes(check);
                }
              if (--mhostLeft == 0) { // FIXME: could be not sufficiant?
                  mupdateSucceed = true;
                  finalizeDashboardUpdate();
                }
            } else if (tid == ZnsHelper::DeviceInfo) {
              QScriptValue devInfo(result.property("data"));
              QString dname = devInfo.property("name").toString();
              check.id = check.host = dname.toLower().toStdString();
              check.status = devInfo.property("status").toBool();
              check.last_state_change = utils::getCtime(devInfo.property("lastChanged").toString(),
                                                        "yyyy/MM/dd hh:mm:ss");
              if (check.status) {
                  check.status = MonitorBroker::ZenossClear;
                  check.alarm_msg = tr("The host '%1' is Up").arg(dname).toStdString();
                } else {
                  check.status = MonitorBroker::ZenossCritical;
                  check.alarm_msg = tr("The host '%1' is Down").arg(dname).toStdString();
                }
              updateCNodes(check);
            } else {
              QString msg = tr("Weird response received from the server");
              utils::alert(msg);
              updateDashboardOnUnknown(msg);
            }
        }
    }
}

QStringList SvNavigator::getAuthInfo(void)
{
  QStringList authInfo = QStringList();
  int pos = mserverAuthChain.indexOf(":");
  if (pos != -1) {
      authInfo.push_back(mserverAuthChain.left(pos));
      authInfo.push_back(mserverAuthChain.mid(pos+1, -1));
    }
  return authInfo;
}

void SvNavigator::openRpcSession(void)
{
  updateDashboardOnUnknown("");
  QStringList authParams = getAuthInfo();
  if (authParams.size() == 2) {
      QUrl znsUrlParams;
      switch(mcoreData->monitor) {
        case MonitorBroker::Zabbix:
          mzbxHelper->setBaseUrl(mmonitorBaseUrl);
          authParams.push_back(QString::number(ZbxHelper::Login));
          mzbxHelper->postRequest(ZbxHelper::Login, authParams);
          break;
        case MonitorBroker::Zenoss:
          mznsHelper->setBaseUrl(mmonitorBaseUrl);
          znsUrlParams.addQueryItem("__ac_name", authParams[0]);
          znsUrlParams.addQueryItem("__ac_password", authParams[1]);
          znsUrlParams.addQueryItem("submitted", "true");
          znsUrlParams.addQueryItem("came_from", mznsHelper->getApiContextUrl());
          mznsHelper->postRequest(ZnsHelper::Login, znsUrlParams.encodedQuery());
          break;
        default:
          break;
        }
    } else {
      updateDashboardOnUnknown(tr("Invalid authentication chain!\n"
                                  "Must follow the pattern login:password"));
    }
}

void SvNavigator::postRpcDataRequest(void) {
  updateStatusBar(tr("Updating..."));
  switch(mcoreData->monitor) {
    case MonitorBroker::Zabbix:
      for (auto host : mcoreData->hosts.keys()) {
          QStringList params;
          params.push_back(mzbxAuthToken);
          params.push_back(host);
          params.push_back(QString::number(ZbxHelper::Trigger));
          mzbxHelper->postRequest(ZbxHelper::Trigger, params);
        }
      break;
    case MonitorBroker::Zenoss:
      mznsHelper->setRouter(ZnsHelper::Device);
      for (auto host : mcoreData->hosts.keys()) {
          mznsHelper->postRequest(ZnsHelper::Device,
                                  ZnsHelper::ReQPatterns[ZnsHelper::Device]
                                  .arg(host, QString::number(ZnsHelper::Device))
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
  if (mcoreData->monitor == MonitorBroker::Zabbix) {
      apiUrl = mzbxHelper->getApiUri();
    } else if (mcoreData->monitor == MonitorBroker::Zenoss) {
      apiUrl =  mznsHelper->getRequestUrl();
    }
  updateDashboardOnUnknown(SERVICE_OFFLINE_MSG.arg(apiUrl%tr(" (error code %1)"), _code));
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
  for (auto& cnode : mcoreData->cnodes) {
      cnode.monitored = true;
      cnode.check.status = MonitorBroker::Unknown;
      cnode.check.last_state_change = UNKNOWN_UPDATE_TIME;
      cnode.check.host = "-";
      cnode.check.check_command = "-";
      cnode.check.alarm_msg = mlastError.toStdString();
      computeStatusInfo(cnode);
      updateDashboard(cnode);
    }
  mcoreData->check_status_count[MonitorBroker::Unknown] = mcoreData->cnodes.size();
  finalizeDashboardUpdate(enable);
}

void SvNavigator::updateTrayInfo(const NodeT& _node)
{
  //FIXME: update once;
  QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;
  if (_node.severity == MonitorBroker::Critical ||
      _node.severity == MonitorBroker::Unknown) {
      icon = QSystemTrayIcon::Critical;
    } else if (_node.severity == MonitorBroker::Minor ||
               _node.severity == MonitorBroker::Major) {
      icon = QSystemTrayIcon::Warning;
    }
  qint32 pbCount = mcoreData->cnodes.size() - mcoreData->check_status_count[MonitorBroker::Normal];
  QString title = APP_NAME%" - "%_node.name;
  QString msg = tr(" - %1 Problem%2\n"
                   " - Level of Impact: %3").arg(QString::number(pbCount), pbCount>1?tr("s"):"",
                                                 utils::criticityToText(_node.severity).toUpper());

  mtrayIcon->showMessage(title, msg, icon);
  mtrayIcon->setToolTip(title%"\n"%msg);
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
  connect(mpreferences, SIGNAL(urlChanged(QString)), mbrowser, SLOT(setUrl(QString)));
  connect(mviewPanel, SIGNAL(currentChanged (int)), this, SLOT(tabChanged(int)));
  connect(mmap, SIGNAL(expandNode(QString, bool, qint32)), this, SLOT(expandNode(const QString &, const bool &, const qint32 &)));
  connect(mtree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(centerGraphOnNode(QTreeWidgetItem *)));
  connect(mzbxHelper, SIGNAL(finished(QNetworkReply*)), this, SLOT(processZbxReply(QNetworkReply*)));
  connect(mzbxHelper, SIGNAL(propagateError(QNetworkReply::NetworkError)), this, SLOT(processRpcError(QNetworkReply::NetworkError)));
  connect(mznsHelper, SIGNAL(finished(QNetworkReply*)), this, SLOT(processZnsReply(QNetworkReply*)));
  connect(mznsHelper, SIGNAL(propagateError(QNetworkReply::NetworkError)), this, SLOT(processRpcError(QNetworkReply::NetworkError)));
}
