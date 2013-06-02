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
#include "client/LsHelper.hpp"
#include <QScriptValueIterator>
#include <QSystemTrayIcon>
#include <sstream>
#include <QStatusBar>
#include <QObject>
#include <zmq.h>
#include <iostream>
#include <locale>
#include <memory>


namespace {
  const QString DEFAULT_TIP_PATTERN(QObject::tr("Service: %1\nDescription: %2\nSeverity: %3\n   Calc. Rule: %4\n   Prop. Rule: %5"));
  const QString ALARM_SPECIFIC_TIP_PATTERN(QObject::tr("\nTarget Host: %6\nData Point: %7\nRaw Output: %8\nOther Details: %9"));
  const QString SERVICE_OFFLINE_MSG(QObject::tr("Failed to connect to %1 (%2)"));
  const QString JSON_ERROR_MSG("{\"return_code\": \"-1\", \"message\": \""%SERVICE_OFFLINE_MSG%"\"}");
  const string UNKNOWN_UPDATE_TIME = utils::getCtime(0);
}

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
    m_coreData (new CoreDataT()),
    m_configFile(_config),
    m_userRole (_userRole),
    m_settings (new Settings()),
    m_chart (new Chart()),
    m_filteredMsgConsole (NULL),
    m_mainSplitter (new QSplitter(this)),
    m_rightSplitter (new QSplitter()),
    m_viewPanel (new QTabWidget()),
    m_browser (new WebKit()),
    m_map (new GraphView(this)),
    m_tree (new SvNavigatorTree()),
    m_preferences (new Preferences(_userRole, Preferences::ChangeMonitoringSettings)),
    m_changePasswdWindow (new Preferences(_userRole, Preferences::ChangePassword)),
    m_msgConsole(new MsgConsole(this)),
    m_nodeContextMenu (new QMenu()),
    m_zbxHelper(new ZbxHelper()),
    m_zbxAuthToken(""),
    m_hostLeft(0),
    m_znsHelper(new ZnsHelper()),
    m_isLogged(false),
    m_lastErrorMsg(""),
    m_trayIcon(new QSystemTrayIcon(QIcon(":images/built-in/icon.png"))),
    m_showOnlyTroubles(false)
{
  setWindowTitle(tr("%1 Operations Console").arg(APP_NAME));
  loadMenus();
  m_viewPanel->addTab(m_map, tr("Dashboard"));
  m_viewPanel->addTab(m_browser, tr("Web Browser"));
  m_mainSplitter->addWidget(m_tree);
  m_mainSplitter->addWidget(m_rightSplitter);
  m_rightSplitter->addWidget(m_viewPanel);
  m_rightSplitter->addWidget(newMsgConsole());
  m_rightSplitter->setOrientation(Qt::Vertical);
  setCentralWidget(m_mainSplitter);
  tabChanged(0);
  addEvents();
}

SvNavigator::~SvNavigator()
{
  if (m_filteredMsgConsole) delete m_filteredMsgConsole;
  delete m_msgConsole;
  delete m_chart;
  delete m_tree;
  delete m_browser;
  delete m_map;
  delete m_coreData;
  delete m_viewPanel;
  delete m_rightSplitter;
  delete m_mainSplitter;
  delete m_preferences;
  delete m_changePasswdWindow;
  delete m_zbxHelper;
  delete m_znsHelper;
  delete m_trayIcon;
  unloadMenus();
}

void SvNavigator::loadMenus(void)
{
  QMenuBar* menuBar = new QMenuBar();
  QToolBar* toolBar = addToolBar(APP_NAME);
  m_menus["FILE"] = menuBar->addMenu(tr("&File")),
      m_subMenus["Refresh"] = m_menus["FILE"]->addAction(QIcon(":images/built-in/refresh.png"),tr("&Refresh Screen")),
      m_subMenus["Capture"] = m_menus["FILE"]->addAction(QIcon(":images/built-in/camera.png"),tr("&Save Map as Image"));
  m_menus["FILE"]->addSeparator(),
      m_subMenus["Quit"] = m_menus["FILE"]->addAction(tr("&Quit")),
      m_subMenus["Capture"]->setShortcut(QKeySequence::Save),
      m_subMenus["Refresh"]->setShortcut(QKeySequence::Refresh),
      m_subMenus["Quit"]->setShortcut(QKeySequence::Quit);
  m_menus["CONSOLE"] = menuBar->addMenu(tr("&Console")),
      m_subMenus["ZoomIn"] = m_menus["CONSOLE"]->addAction(QIcon(":images/built-in/zoomin.png"),tr("Map Zoom &In")),
      m_subMenus["ZoomOut"] = m_menus["CONSOLE"]->addAction(QIcon(":images/built-in/zoomout.png"),tr("Map Zoom &Out")),
      m_subMenus["HideChart"] = m_menus["CONSOLE"]->addAction(tr("Hide &Chart")),
      m_subMenus["ZoomIn"]->setShortcut(QKeySequence::ZoomIn),
      m_subMenus["ZoomOut"]->setShortcut(QKeySequence::ZoomOut);
  m_menus["CONSOLE"]->addSeparator(),
      m_subMenus["FullScreen"] = m_menus["CONSOLE"]->addAction(QIcon(":images/built-in/fullscreen.png"),tr("&Full Screen")),
      m_subMenus["FullScreen"]->setCheckable(true);
  m_menus["CONSOLE"]->addSeparator(),
      m_subMenus["TroubleView"] = m_menus["CONSOLE"]->addAction(QIcon(":images/built-in/alert-circle.png"),tr("&Show only trouble messages")),
      m_subMenus["TroubleView"]->setCheckable(true),
      m_subMenus["IncreaseMsgFont"] = m_menus["CONSOLE"]->addAction(QIcon(":images/built-in/incr-font-size.png"),tr("&Increase message &font")),
      m_subMenus["IncreaseMsgFont"]->setCheckable(true);
  m_menus["PREFERENCES"] = menuBar->addMenu(tr("&Preferences")),
      m_subMenus["ChangePassword"] = m_menus["PREFERENCES"]->addAction(tr("Change &Password")),
      m_subMenus["ChangeMonitoringSettings"] = m_menus["PREFERENCES"]->addAction(QIcon(":images/built-in/system-preferences.png"),tr("&Monitoring Settings")),
      m_subMenus["ChangeMonitoringSettings"]->setShortcut(QKeySequence::Preferences);
  m_menus["BROWSER"] = menuBar->addMenu(tr("&Browser")),
      m_subMenus["BrowserBack"] = m_menus["BROWSER"]->addAction(QIcon(":images/built-in/browser-back.png"),tr("Bac&k")),
      m_subMenus["BrowserForward"] = m_menus["BROWSER"]->addAction(QIcon(":images/built-in/browser-forward.png"),tr("For&ward"));
  m_subMenus["BrowserStop"] = m_menus["BROWSER"]->addAction(QIcon(":images/built-in/browser-stop.png"),tr("Sto&p"));
  m_menus["HELP"] = menuBar->addMenu(tr("&Help")),
      m_subMenus["ShowOnlineResources"] = m_menus["HELP"]->addAction(tr("Online &Resources")),
      m_menus["HELP"]->addSeparator(),
      m_subMenus["ShowAbout"] = m_menus["HELP"]->addAction(tr("&About %1").arg(APP_NAME)),
      m_subMenus["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
  m_contextMenuList["FilterNodeRelatedMessages"] = m_nodeContextMenu->addAction(tr("&Filter related messages")),
      m_contextMenuList["CenterOnNode"] = m_nodeContextMenu->addAction(tr("Center Graph &On")),
      m_contextMenuList["Cancel"] = m_nodeContextMenu->addAction(tr("&Cancel"));
  toolBar->setIconSize(QSize(16,16)),
      toolBar->addAction(m_subMenus["Refresh"]),
      toolBar->addAction(m_subMenus["ZoomIn"]),
      toolBar->addAction(m_subMenus["ZoomOut"]),
      toolBar->addAction(m_subMenus["Capture"]),
      toolBar->addSeparator(),
      toolBar->addAction(m_subMenus["BrowserBack"]),
      toolBar->addAction(m_subMenus["BrowserForward"]),
      toolBar->addAction(m_subMenus["BrowserStop"]),
      toolBar->addSeparator(),
      toolBar->addAction(m_subMenus["FullScreen"]);
  QMainWindow::setMenuBar(menuBar);
}

void SvNavigator::closeEvent(QCloseEvent * event)
{
  if (m_filteredMsgConsole) m_filteredMsgConsole->close();
  QMainWindow::closeEvent(event);
}

void SvNavigator::contextMenuEvent(QContextMenuEvent * event)
{
  QPoint pos = event->globalPos();
  QList<QTreeWidgetItem*> treeNodes = m_tree->selectedItems();
  QGraphicsItem* graphNode = m_map->nodeAtGlobalPos(pos);
  if (treeNodes.length() || graphNode) {
    if (graphNode) {
      QString itemId = graphNode->data(0).toString();
          m_selectedNode =  itemId.left(itemId.indexOf(":"));
    }  else {
          m_selectedNode = treeNodes[0]->data(0, QTreeWidgetItem::UserType).toString();
    }
      m_nodeContextMenu->exec(pos);
  }
}

void SvNavigator::startMonitor()
{
  prepareUpdate(0);
  switch(m_coreData->monitor) {
    case MonitorBroker::Zenoss:
    case MonitorBroker::Zabbix:
      !m_isLogged ? openRpcSession(0): requestRpcData();
      break;
    case MonitorBroker::Nagios:
    default:
      m_preferences->useLs()? runLivestatusUpdate(0) : runNagiosUpdate(0);
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

void SvNavigator::load(const QString& _file, int srcId)
{
  if (!_file.isEmpty()) m_configFile = utils::getAbsolutePath(_file);
  m_activeFile = m_configFile;
  QMainWindow::setWindowTitle(tr("%1 Operations Console - %2").arg(APP_NAME, m_configFile));
  Parser parser;
  parser.parseSvConfig(m_configFile, *m_coreData);
  m_tree->clear();
  m_tree->addTopLevelItem(m_coreData->tree_items[SvNavigatorTree::RootId]);
  m_map->load(parser.getDotGraphFile(), m_coreData->bpnodes, m_coreData->cnodes);
  m_browser->setUrl(m_sources[srcId].mon_url);
  this->resizeDashboard();
  QMainWindow::show();
  m_map->scaleToFitViewPort();
  m_trayIcon->show();
  m_trayIcon->setToolTip(APP_NAME);
}

void SvNavigator::unloadMenus(void)
{
  m_subMenus.clear();
  m_menus.clear();
  delete m_nodeContextMenu;
}

void SvNavigator::handleChangePasswordAction(void)
{
  m_changePasswdWindow->exec();
}

void SvNavigator::handleChangeMonitoringSettingsAction(void)
{
  m_preferences->exec();
  startMonitor();
}

void SvNavigator::handleShowOnlineResources(void)
{
  QDesktopServices appLauncher;
  appLauncher.openUrl(QUrl("http://RealOpInsight.com/"));
}

void SvNavigator::handleShowAbout(void)
{
  Preferences about(m_userRole, Preferences::ShowAbout);
  about.exec();
}

void SvNavigator::toggleFullScreen(bool _toggled)
{
  if (_toggled)
    showFullScreen();
  else
    showNormal();
}

void SvNavigator::toggleTroubleView(bool _toggled)
{
  m_msgConsole->setEnabled(false);
  m_showOnlyTroubles = _toggled;
  if (m_showOnlyTroubles) {
      m_msgConsole->clearNormalMsg();
  } else {
      for (auto it = m_coreData->cnodes.begin(), end = m_coreData->cnodes.end();
           it != end; it++) m_msgConsole->updateNodeMsg(it);
      m_msgConsole->sortByColumn(1);
  }
  m_msgConsole->setEnabled(true);
}

void SvNavigator::toggleIncreaseMsgFont(bool _toggled)
{
  if (_toggled) {
      QFont df =  m_msgConsole->font();
      m_msgConsole->setFont(QFont(df.family(), 16));
  } else {
      m_msgConsole->setFont(QFont());
  }
  m_msgConsole->updateEntriesSize(m_msgConsoleSize);
  m_msgConsole->resizeRowsToContents();
}

void SvNavigator::runNagiosUpdate(int srcId)
{
  CheckT invalidCheck;
  invalidCheck.status = MonitorBroker::NagiosUnknown;
  invalidCheck.last_state_change = UNKNOWN_UPDATE_TIME;
  invalidCheck.host = "-";
  invalidCheck.check_command = "-";
  invalidCheck.alarm_msg = "Error occured";

  //FIXME: use isLogged?
  std::string uri = QString("tcp://%1:%2").arg(m_sources[srcId].ls_addr,
                                               QString::number(m_sources[srcId].ls_port)).toStdString();
  auto socket = std::unique_ptr<ZmqSocket>(new ZmqSocket(uri, ZMQ_REQ));
  if(socket->connect())
    socket->makeHandShake();
  if (socket->isConnected2Server()) {
    if (socket->getServerSerial() < 110) {
      utils::alert(tr("The server serial %1 is not supported").arg(socket->getServerSerial()));
          m_updateSucceed = false;
    }
    updateStatusBar(tr("Updating..."));
  } else {
      m_updateSucceed = false;
    invalidCheck.alarm_msg = socket->getErrorMsg();
    QString socketError(invalidCheck.alarm_msg.c_str());
    utils::alert(socketError);
    updateStatusBar(socketError);
  }

  for (NodeListIteratorT cnode = m_coreData->cnodes.begin();
       cnode != m_coreData->cnodes.end(); cnode++) {
    if (cnode->child_nodes == "") {
      cnode->severity = MonitorBroker::Unknown;
          m_coreData->check_status_count[cnode->severity]++;
      continue;
    }

    QStringList ids = cnode->child_nodes.split(Parser::CHILD_SEP);
    foreach (const QString& cid, ids) {
          QString msg = m_sources[srcId].auth%":"%cid;
          if (m_updateSucceed) {
        socket->send(msg.toStdString());
        JsonHelper jsHelper(socket->recv());
        cnode->check.status = (jsHelper.getProperty("return_code").toInt32()!=0)? MonitorBroker::NagiosUnknown:
                                                                                  jsHelper.getProperty("status").toInt32();
        cnode->check.host = jsHelper.getProperty("host").toString().toStdString();
        cnode->check.last_state_change = utils::getCtime(jsHelper.getProperty("lastchange").toUInt32());
        cnode->check.check_command = jsHelper.getProperty("command").toString().toStdString();
        cnode->check.alarm_msg = jsHelper.getProperty("message").toString().toStdString();
      } else {
        cnode->check = invalidCheck;
      }
      cnode->monitored = true;
      computeStatusInfo(cnode);
      updateDashboard(cnode);
          m_coreData->check_status_count[cnode->severity]++;
    }
  }
  socket.reset(NULL);
  finalizeDashboardUpdate();
}



void SvNavigator::runLivestatusUpdate(int srcId)
{
  // FIXME:use isLogged?
  LsHelper mklsHelper(m_sources[srcId].ls_addr, m_sources[srcId].ls_port);
  if (!mklsHelper.connectToService()) {
      m_updateSucceed = false;
      m_lastErrorMsg = mklsHelper.errorString();
    updateDashboardOnUnknown();
      return;
  }
  CheckT invalidCheck;
  invalidCheck.status = MonitorBroker::NagiosUnknown;
  invalidCheck.last_state_change = UNKNOWN_UPDATE_TIME;
  invalidCheck.host = "-";
  invalidCheck.check_command = "-";
  invalidCheck.alarm_msg = "Service not found";
  QHashIterator<QString, QStringList> hit(m_coreData->hosts);
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
        CheckListCstIterT chkit;
        if (mklsHelper.findCheck(cid, chkit)) {
          updateCNodes(*chkit);
        } else {
          invalidCheck.id = cid.toStdString();
          invalidCheck.alarm_msg = tr("Service not found (%1)").arg(cid).toStdString();
          updateCNodes(invalidCheck);
        }
      }
    }
  }
  finalizeDashboardUpdate();
}


void SvNavigator::runZabbixZenossUpdate(int srcId)
{
  updateDashboardOnUnknown();
  openRpcSession(srcId);
  if (m_isLogged) requestRpcData();
}


void SvNavigator::prepareUpdate(int srcId)
{
  QMainWindow::setEnabled(false);
  resetSettings();
  m_coreData->check_status_count[MonitorBroker::Normal] = 0;
  m_coreData->check_status_count[MonitorBroker::Minor] = 0;
  m_coreData->check_status_count[MonitorBroker::Major] = 0;
  m_coreData->check_status_count[MonitorBroker::Critical] = 0;
  m_coreData->check_status_count[MonitorBroker::Unknown] = 0;
  m_hostLeft = m_coreData->hosts.size();
  m_updateSucceed = true;

  QString msg = QObject::tr("Connecting to %1...");
  SourceListT::Iterator source = m_sources.find(srcId);
  if (source != m_sources.end()) {
      switch(m_coreData->monitor) {
        case MonitorBroker::Nagios:
          msg = msg.arg(QString("tcp://%1:%2").arg(source->ls_addr, QString::number(source->ls_port)));
          break;
        case MonitorBroker::Zabbix:
          msg = msg.arg(source->zbx_handler->getApiEndpoint());
          break;
        case MonitorBroker::Zenoss:
          msg = msg.arg(source->zns_handler->getApiBaseEndpoint()); //FIXME: msg.arg(mznsHelper->getApiContextUrl()) crashes
          break;
        default:
          break;
        }
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
    toolTip += ALARM_SPECIFIC_TIP_PATTERN.arg(QString::fromStdString(_node.check.host).replace("\n", " "),
                                              _node.child_nodes,
                                              QString::fromStdString(_node.check.alarm_msg),
                                              _node.actual_msg);
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
  m_map->updateNode(_node, toolTip);
  if (!m_showOnlyTroubles ||
      (m_showOnlyTroubles && _node.severity != MonitorBroker::Normal))
    m_msgConsole->updateNodeMsg(_node);
  emit hasToBeUpdate(_node.parent);
}

void SvNavigator::updateCNodes(const CheckT& check)
{
  for (NodeListIteratorT cnode = m_coreData->cnodes.begin();
       cnode != m_coreData->cnodes.end(); cnode++) {
    if (cnode->child_nodes.toLower() == QString::fromStdString(check.id).toLower()) {
      cnode->check = check;
      computeStatusInfo(cnode);
          m_coreData->check_status_count[cnode->severity]++;
      updateDashboard(cnode);
      cnode->monitored = true;
    }
  }
}

void SvNavigator::finalizeDashboardUpdate(const bool& enable)
{
  if (!m_coreData->cnodes.isEmpty()) {
    Chart *chart = new Chart;
      QString chartdDetails = chart->update(m_coreData->check_status_count, m_coreData->cnodes.size());
      m_map->updateStatsPanel(chart);
      if (m_chart) delete m_chart; m_chart = chart; m_chart->setToolTip(chartdDetails);
      m_msgConsole->sortByColumn(1, Qt::AscendingOrder);
      m_msgConsole->updateEntriesSize(m_msgConsoleSize); //FIXME: Take care of message wrapping
      resetInterval();
      m_timer = startTimer(m_interval);
      if (m_updateSucceed) updateStatusBar(tr("Update completed"));
      for (NodeListIteratorT cnode = m_coreData->cnodes.begin(), end = m_coreData->cnodes.end();
         cnode != end; cnode++) {
      if (!cnode->monitored) {
        cnode->check.status = MonitorBroker::Unknown;
        cnode->check.last_state_change = UNKNOWN_UPDATE_TIME;
        cnode->check.host = "-";
        cnode->check.alarm_msg = tr("Unknown service (%1)").arg(cnode->child_nodes).toStdString();
        computeStatusInfo(cnode);
              m_coreData->check_status_count[cnode->severity]++;
        updateDashboard(cnode);
        cnode->monitored = true;
      }
      cnode->monitored = false;
    }
  }
  //FIXME: Do this while avoiding searching at each update
  if (!m_coreData->bpnodes.isEmpty()) updateTrayInfo(m_coreData->bpnodes[SvNavigatorTree::RootId]);
  QMainWindow::setEnabled(enable);
}

void SvNavigator::computeStatusInfo(NodeListT::iterator&  _node)
{
  computeStatusInfo(*_node);
}

void SvNavigator::computeStatusInfo(NodeT& _node)
{
  QRegExp regexp;
  _node.severity = utils::computeCriticity(m_coreData->monitor, _node.check.status);
  _node.prop_sev = utils::computePropCriticity(_node.severity, _node.sev_prule);
  _node.actual_msg = QString::fromStdString(_node.check.alarm_msg);
  if (_node.check.host == "-") return;
  if (m_coreData->monitor == MonitorBroker::Zabbix) {
    regexp.setPattern(MsgConsole::TAG_ZABBIX_HOSTNAME);
    _node.actual_msg.replace(regexp, _node.check.host.c_str());
    regexp.setPattern(MsgConsole::TAG_ZABBIX_HOSTNAME2);
    _node.actual_msg.replace(regexp, _node.check.host.c_str());
  }
  if (_node.severity == MonitorBroker::Normal) {
    if (_node.notification_msg.isEmpty())  {
      return ;
    } else {
      _node.actual_msg = _node.notification_msg;
    }
  } else if (_node.alarm_msg.isEmpty())  {
    return ;
  } else {
    _node.actual_msg = _node.alarm_msg;
  }
  regexp.setPattern(MsgConsole::TAG_HOSTNAME);
  _node.actual_msg.replace(regexp, _node.check.host.c_str());
  auto info = QString(_node.check.id.c_str()).split("/");
  if (info.length() > 1) {
    regexp.setPattern(MsgConsole::TAG_CHECK);
    _node.actual_msg.replace(regexp, info[1]);
  }
  if (m_coreData->monitor == MonitorBroker::Nagios) {
    info = QString(_node.check.check_command.c_str()).split("!");
    if (info.length() >= 3) {
      regexp.setPattern(MsgConsole::TAG_THERESHOLD);
      _node.actual_msg.replace(regexp, info[1]);
      if (_node.severity == MonitorBroker::Major)
        _node.actual_msg.replace(regexp, info[2]);
    }
  }
}

void SvNavigator::updateBpNode(const QString& _nodeId)
{
  NodeListT::iterator node;
  if (!utils::findNode(m_coreData, _nodeId, node)) return;

  QStringList nodeIds = node->child_nodes.split(Parser::CHILD_SEP);
  Criticity criticity;
  foreach (const QString& nodeId, nodeIds) {
    NodeListT::iterator child;
      if (!utils::findNode(m_coreData, nodeId, child)) continue;
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
  m_map->updateNode(node, toolTip);
  updateNavTreeItemStatus(node, toolTip);
  if (node->id != SvNavigatorTree::RootId) emit hasToBeUpdate(node->parent);
}

void SvNavigator::updateNavTreeItemStatus(const NodeListT::iterator& _node, const QString& _tip)
{
  updateNavTreeItemStatus(*_node, _tip);
}

void SvNavigator::updateNavTreeItemStatus(const NodeT& _node, const QString& _tip)
{
  auto tnode_it = m_coreData->tree_items.find(_node.id);
  if (tnode_it != m_coreData->tree_items.end()) {
    (*tnode_it)->setIcon(0, utils::computeCriticityIcon(_node.severity));
    (*tnode_it)->setToolTip(0, _tip);
  }
}

void SvNavigator::expandNode(const QString& _nodeId, const bool& _expand, const qint32& _level)
{
  auto node = m_coreData->bpnodes.find(_nodeId);
  if (node == m_coreData->bpnodes.end()) return;
  if (node->child_nodes != "") {
    QStringList  childNodes = node->child_nodes.split(Parser::CHILD_SEP);
    foreach (const auto& cid, childNodes) {
          m_map->setNodeVisible(cid, _nodeId, _expand, _level);
    }
  }
}

void SvNavigator::centerGraphOnNode(const QString& _nodeId)
{
  if (_nodeId != "") m_selectedNode =  _nodeId;
  m_map->centerOnNode(m_selectedNode);
}

void SvNavigator::filterNodeRelatedMsg(void)
{
  if (m_filteredMsgConsole) delete m_filteredMsgConsole;
  m_filteredMsgConsole = new MsgConsole();
  NodeListT::iterator node;
  if (utils::findNode(m_coreData, m_selectedNode, node)) {
      filterNodeRelatedMsg(m_selectedNode);
    QString title = tr("Messages related to '%2' - %1").arg(APP_NAME, node->name);
      m_filteredMsgConsole->updateEntriesSize(m_msgConsoleSize, true);
      m_filteredMsgConsole->setWindowTitle(title);
  }
  qint32 rh = qMax(m_filteredMsgConsole->getRowCount() * m_filteredMsgConsole->rowHeight(0) + 50, 100);
  if (m_filteredMsgConsole->height() > rh) m_filteredMsgConsole->resize(m_msgConsoleSize.width(), rh);
  m_filteredMsgConsole->sortByColumn(1, Qt::AscendingOrder);
  m_filteredMsgConsole->show();
}

void SvNavigator::filterNodeRelatedMsg(const QString& _nodeId)
{
  NodeListT::iterator node;
  if (utils::findNode(m_coreData, _nodeId, node) &&
      node->child_nodes != "") {
    if (node->type == NodeType::ALARM_NODE) {
          m_filteredMsgConsole->updateNodeMsg(node);
    } else {
      QStringList childIds = node->child_nodes.split(Parser::CHILD_SEP);
      foreach (const QString& chkid, childIds) {
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
      m_subMenus["Refresh"]->setEnabled(true);
      m_subMenus["Capture"]->setEnabled(true);
      m_subMenus["ZoomIn"]->setEnabled(true);
      m_subMenus["ZoomOut"]->setEnabled(true);
      m_menus["BROWSER"]->setEnabled(false);
      m_subMenus["BrowserBack"]->setEnabled(false);
      m_subMenus["BrowserForward"]->setEnabled(false);
      m_subMenus["BrowserStop"]->setEnabled(false);
      break;
    case 1:
      m_menus["BROWSER"]->setEnabled(true);
      m_subMenus["BrowserBack"]->setEnabled(true);
      m_subMenus["BrowserForward"]->setEnabled(true);
      m_subMenus["BrowserStop"]->setEnabled(true);
      m_subMenus["Refresh"]->setEnabled(false);
      m_subMenus["Capture"]->setEnabled(false);
      m_subMenus["ZoomIn"]->setEnabled(false);
      m_subMenus["ZoomOut"]->setEnabled(false);
      break;
    default:
      break;

  }
}

void SvNavigator::hideChart(void)
{
  if (m_map->hideChart()) {
      m_subMenus["HideChart"]->setIcon(QIcon(":images/check.png"));
    return;
  }
  m_subMenus["HideChart"]->setIcon(QIcon(""));
}

void SvNavigator::centerGraphOnNode(QTreeWidgetItem * _item)
{
  centerGraphOnNode(_item->data(0, QTreeWidgetItem::UserType).toString());
}

void SvNavigator::resizeDashboard(void)
{
  const qreal GRAPH_HEIGHT_RATE = 0.50;
  QSize screenSize = qApp->desktop()->screen(0)->size();
  m_msgConsoleSize = QSize(screenSize.width() * 0.80, screenSize.height() * (1.0 - GRAPH_HEIGHT_RATE));

  QList<qint32> framesSize;
  framesSize.push_back(screenSize.width() * 0.20);
  framesSize.push_back(m_msgConsoleSize.width());
  m_mainSplitter->setSizes(framesSize);

  framesSize[0] = (screenSize.height() * GRAPH_HEIGHT_RATE);
  framesSize[1] = (m_msgConsoleSize.height());
  m_rightSplitter->setSizes(framesSize);

  m_mainSplitter->resize(screenSize.width(), screenSize.height() * 0.85);
  QMainWindow::resize(screenSize.width(),  screenSize.height());
}

void SvNavigator::processZbxReply(QNetworkReply* _reply)
{
  _reply->deleteLater();
  QNetworkReply::NetworkError errcode = _reply->error();
  if (errcode != QNetworkReply::NoError) {
      m_lastErrorMsg = _reply->errorString();
    processRpcError(errcode);
    return;
  }
  QString data = _reply->readAll();
  JsonHelper jsHelper(data);
  m_lastErrorMsg = jsHelper.getProperty("error").property("data").toString();
  if (m_lastErrorMsg.isEmpty()) m_lastErrorMsg = jsHelper.getProperty("error").property("message").toString();
  if (!m_lastErrorMsg.isEmpty()) {
    updateDashboardOnUnknown();
    return;
  }
  qint32 tid = jsHelper.getProperty("id").toInt32();
  switch(tid) {
    case ZbxHelper::Login: {
        m_zbxAuthToken = jsHelper.getProperty("result").toString();
        if (!m_zbxAuthToken.isEmpty()) {
            m_isLogged = true;
      }
      break;
    }
    case ZbxHelper::ApiVersion: {
        m_zbxHelper->updateTrid(jsHelper.getProperty("result").toString());
      break;
    }
    case ZbxHelper::Trigger:
    case ZbxHelper::TriggerV18: {
      QScriptValueIterator trigger(jsHelper.getProperty("result"));
      CheckT check;
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
        if (tid == ZbxHelper::TriggerV18) {
          check.last_state_change = utils::getCtime(triggerData.property("lastchange").toUInt32());
        } else {
          QScriptValueIterator item(triggerData.property("items"));
          if (item.hasNext()) {
            item.next(); if (item.flags()&QScriptValue::SkipInEnumeration) continue;
            QScriptValue itemData = item.value();
            check.last_state_change = utils::getCtime(itemData.property("lastclock").toUInt32());
          }
        }
        QString key = ID_PATTERN.arg(targetHost, triggerName);
        check.id = key.toStdString();
        updateCNodes(check);
      }
        if (--m_hostLeft == 0) {
            m_updateSucceed = true;
        finalizeDashboardUpdate();
      }
      break;
    }
    default :
      m_lastErrorMsg = tr("Weird response received from the server");
      updateDashboardOnUnknown();
      qDebug() << data;
      break;
  }
}

void SvNavigator::processZnsReply(QNetworkReply* _reply)
{
  _reply->deleteLater();

  if (_reply->error() != QNetworkReply::NoError) {
      m_lastErrorMsg = _reply->errorString();
      processRpcError(_reply->error());
    return;
  }

  QVariant cookiesContainer = _reply->header(QNetworkRequest::SetCookieHeader);
  QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(cookiesContainer);
  QString data = _reply->readAll();
  if (data.endsWith("submitted=true")) {
      m_znsHelper->cookieJar()->setCookiesFromUrl(cookies, m_znsHelper->getApiBaseEndpoint());
      m_isLogged = true;
  } else {
      JsonHelper jsonHelper(data);
    qint32 tid = jsonHelper.getProperty("tid").toInt32();
    QScriptValue result = jsonHelper.getProperty("result");
    bool reqSucceed = result.property("success").toBool();
    if (!reqSucceed) {
          m_lastErrorMsg = tr("Authentication failed: %1").arg(result.property("msg").toString());
      updateDashboardOnUnknown();
      return;
    }
    if (tid == ZnsHelper::Device) {
      QScriptValueIterator devices(result.property("devices"));
      while(devices.hasNext()) {
        devices.next(); if (devices.flags()&QScriptValue::SkipInEnumeration) continue;

        QScriptValue ditem = devices.value();
        QString duid = ditem.property("uid").toString();
              QNetworkReply* reply = m_znsHelper->postRequest(ZnsHelper::Component,
                                                              ZnsHelper::ReqPatterns[ZnsHelper::Component]
                                                              .arg(duid, QString::number(ZnsHelper::Component))
                                                              .toAscii());
              processZnsReply(reply);

        QString dname = ditem.property("name").toString();
              if (m_coreData->hosts[dname].contains("ping", Qt::CaseInsensitive)) {
                  reply = m_znsHelper->postRequest(ZnsHelper::Device,
                                                   ZnsHelper::ReqPatterns[ZnsHelper::DeviceInfo]
                                                   .arg(duid, QString::number(ZnsHelper::DeviceInfo))
                                                   .toAscii());
                  processZnsReply(reply);
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
          check.id = chkid.toStdString();
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
              if (--m_hostLeft == 0) { // FIXME: could be not sufficiant?
                  m_updateSucceed = true;
          finalizeDashboardUpdate();
        }
      } else if (tid == ZnsHelper::DeviceInfo) {
        QScriptValue devInfo(result.property("data"));
        QString dname = devInfo.property("name").toString();
        check.id = check.host = dname.toStdString();
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
              m_lastErrorMsg = tr("Weird response received from the server");
        updateDashboardOnUnknown();
      }
    }
  }
}

QStringList SvNavigator::getAuthInfo(int srcId)
{
  SourceListT::Iterator source = m_sources.find(srcId);
  if (source != m_sources.end()) {
      return getAuthInfo(source->auth);
    }
  return QStringList();
}

QStringList SvNavigator::getAuthInfo(const QString& authString)
{
  QStringList authInfo;
  int pos = authString.indexOf(":");
  if (pos != -1) {
      authInfo.push_back(authString.left(pos));
      authInfo.push_back(authString.mid(pos+1, -1));
  }
  return authInfo;
}



void SvNavigator::openRpcSession(int srcId)
{
  SourceListT::Iterator source = m_sources.find(srcId);
  if (source == m_sources.end()) {
      m_isLogged = false;
      return;
    }
  openRpcSession(*source);
}


void SvNavigator::openRpcSession(const SourceT& src)
{
  QStringList authParams = getAuthInfo(src.auth);
  if (authParams.size() != 2 && m_coreData->monitor != MonitorBroker::Nagios) {
      m_lastErrorMsg = tr("Invalid authentication chain!\nMust follow the pattern login:password");
      updateDashboardOnUnknown();
      return;
    }
  QUrl znsUrlParams;
  switch(m_coreData->monitor) {
    case MonitorBroker::Nagios:
      if (m_preferences->useLs()) {
          m_isLogged = src.ls_handler->connectToService();
        } else {
          if(src.d4n_handler->connect()) {
              src.d4n_handler->makeHandShake();
              m_isLogged = src.d4n_handler->isConnected2Server();
            } else {
              m_isLogged = false;
            }
        }
      break;
    case MonitorBroker::Zabbix: {
        src.zbx_handler->setBaseUrl(src.mon_url);
        authParams.push_back(QString::number(ZbxHelper::Login));
        QNetworkReply* reply = m_zbxHelper->postRequest(ZbxHelper::Login, authParams);
        processZbxReply(reply);
        if (m_isLogged) {
            // Get API version
            QStringList params;
            params.push_back(m_zbxAuthToken);
            params.push_back(QString::number(ZbxHelper::ApiVersion));
            reply = src.zbx_handler->postRequest(ZbxHelper::ApiVersion, params);
            processZbxReply(reply);
            if (src.zbx_handler->getTrid() == -1) m_isLogged = false; // Means problem while retrieving API version
          }
        break;
      }
    case MonitorBroker::Zenoss: {
        src.zns_handler->setBaseUrl(src.mon_url);
        znsUrlParams.addQueryItem("__ac_name", authParams[0]);
        znsUrlParams.addQueryItem("__ac_password", authParams[1]);
        znsUrlParams.addQueryItem("submitted", "true");
        znsUrlParams.addQueryItem("came_from", src.zns_handler->getApiContextEndpoint());
        QNetworkReply* reply = src.zns_handler->postRequest(ZnsHelper::Login, znsUrlParams.encodedQuery());
        processZnsReply(reply);
      }
      break;
    default:
      break;
    }
}


void SvNavigator::requestRpcData(void) {
  updateStatusBar(tr("Updating..."));
  switch(m_coreData->monitor) {
    case MonitorBroker::Zabbix: {
        int trid = m_zbxHelper->getTrid();
        foreach (const QString& host, m_coreData->hosts.keys()) {
        QStringList params;
            params.push_back(m_zbxAuthToken);
        params.push_back(host);
        params.push_back(QString::number(trid));
            QNetworkReply* reply = m_zbxHelper->postRequest(trid, params);
            processZbxReply(reply);
      }
      break;
    }
    case MonitorBroker::Zenoss: {
        m_znsHelper->setRouterEndpoint(ZnsHelper::Device);
        foreach (const QString& host, m_coreData->hosts.keys()) {
            QNetworkReply* reply = m_znsHelper->postRequest(ZnsHelper::Device,
                                                            ZnsHelper::ReqPatterns[ZnsHelper::Device]
                                                            .arg(host, QString::number(ZnsHelper::Device))
                                                            .toAscii());
            processZbxReply(reply);
          }
      }
      break;
    default:
      break;
  }
}

void SvNavigator::processRpcError(QNetworkReply::NetworkError _code)
{
  QString apiUrl = "";
  if (m_coreData->monitor == MonitorBroker::Zabbix) {
      apiUrl = m_zbxHelper->getApiEndpoint();
    } else if (m_coreData->monitor == MonitorBroker::Zenoss) {
      apiUrl =  m_znsHelper->getRequestEndpoint();
  }
  switch (_code) {
    case QNetworkReply::RemoteHostClosedError:
      m_lastErrorMsg = SERVICE_OFFLINE_MSG.arg(apiUrl, tr("The connection has been closed by the remote host"));
      break;
    case QNetworkReply::HostNotFoundError:
      m_lastErrorMsg = SERVICE_OFFLINE_MSG.arg(apiUrl, tr("Host not found"));
      break;
    case QNetworkReply::ConnectionRefusedError:
      m_lastErrorMsg = SERVICE_OFFLINE_MSG.arg(apiUrl, tr("Connection refused"));
      break;
    case QNetworkReply::SslHandshakeFailedError:
      mlastErrorMsg = tr("SSL Handshake failed");
      break;
    case QNetworkReply::TimeoutError:
      mlastErrorMsg = tr("Timeout exceeded");
      break;
    default:
      m_lastErrorMsg = SERVICE_OFFLINE_MSG.arg(apiUrl, tr("Unknown error: code %1").arg(_code));
  }
  updateDashboardOnUnknown();
}

void SvNavigator::updateDashboardOnUnknown()
{
  m_updateSucceed = false;
  bool enable = false;
  if (!m_lastErrorMsg.isEmpty()) {
    enable = true;
      utils::alert(m_lastErrorMsg);
      updateStatusBar(m_lastErrorMsg);
  }
  for (NodeListIteratorT cnode = m_coreData->cnodes.begin();
       cnode != m_coreData->cnodes.end(); cnode++) {
    cnode->monitored = true;
    cnode->check.status = MonitorBroker::Unknown;
    cnode->check.last_state_change = UNKNOWN_UPDATE_TIME;
    cnode->check.host = "-";
    cnode->check.check_command = "-";
      cnode->check.alarm_msg = m_lastErrorMsg.toStdString();
    computeStatusInfo(cnode);
    updateDashboard(cnode);
  }
  m_lastErrorMsg.clear();
  m_coreData->check_status_count[MonitorBroker::Unknown] = m_coreData->cnodes.size();
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
  qint32 pbCount = m_coreData->cnodes.size() - m_coreData->check_status_count[MonitorBroker::Normal];
  QString title = APP_NAME%" - "%_node.name;
  QString msg = tr(" - %1 Problem%2\n"
                   " - Level of Impact: %3").arg(QString::number(pbCount), pbCount>1?tr("s"):"",
                                                 utils::criticityToText(_node.severity).toUpper());

  m_trayIcon->showMessage(title, msg, icon);
  m_trayIcon->setToolTip(title%"\n"%msg);
}

QTabWidget* SvNavigator::newMsgConsole()
{
  QTabWidget* msgConsole(new QTabWidget());
  QHBoxLayout* lyt(new QHBoxLayout());
  QToolBar* tlbar (new QToolBar());
  QGroupBox* wdgsGrp(new QGroupBox());
  tlbar->addAction(m_subMenus["TroubleView"]);
  tlbar->addAction(m_subMenus["IncreaseMsgFont"]);
  tlbar->setOrientation(Qt::Vertical);
  lyt->addWidget(m_msgConsole, Qt::AlignLeft);
  lyt->addWidget(tlbar, Qt::AlignRight);
  lyt->setMargin(0);
  lyt->setContentsMargins(QMargins(0, 0, 0, 0));
  wdgsGrp->setLayout(lyt);
  msgConsole->addTab(wdgsGrp, tr("Message Console"));
  return msgConsole;
}

void SvNavigator::resetSettings(void)
{
  m_isLogged = false;
  for (SourceListT::Iterator it = m_sources.begin(), end = m_sources.end();
       it != end; it++) {
      if (it->d4n_handler) delete it->d4n_handler;
      if (it->ls_handler) delete it->ls_handler;
      if (it->zbx_handler) delete it->zbx_handler;
      if (it->zns_handler) delete it->zns_handler;
    }
  m_sources.clear(); //FIXME: risk of memory leak with apiHandler
  SourceT src;
  for (int i=0; i< MAX_SRCS; i++) {
      if (m_preferences->isSetSource(i)) {
          m_settings->loadSource(i, src);
          switch (src.mon_type) {
            case MonitorBroker::Nagios:
              if (m_preferences->useLs()) {
                  src.ls_handler = new LsHelper(src.ls_addr, src.ls_port);
                } else {
                  std::string uri = QString("tcp://%1:%2").arg(src.ls_addr,
                                                               QString::number(src.ls_port)).toStdString();
                  src.d4n_handler = new ZmqSocket(uri, ZMQ_REQ);
                }
              break;
            case MonitorBroker::Zabbix:
              src.zbx_handler = new ZbxHelper();
              break;
            case MonitorBroker::Zenoss:
              src.zns_handler = new ZnsHelper();
              break;
            default:
              break;
            }
          m_sources.insert(i, src);
        }
    }
  resetInterval();
}

void SvNavigator::resetInterval()
{
  m_interval = 1000 * m_settings->getUpdateInterval();
  killTimer(m_timer);
  m_timer = startTimer(m_interval);
}

void SvNavigator::checkSourcesAvailability()
{
  for (SourceListT::Iterator src, end = m_sources.end();
       src != end; src++) {
      switch (src->mon_type)
        {
        case MonitorBroker::Nagios:
          break;
        case MonitorBroker::Zabbix:
          break;
        case MonitorBroker::Zenoss:
          break;
        default:
          break;
        }
    }
}


void SvNavigator::addEvents(void)
{
  connect(this, SIGNAL(hasToBeUpdate(QString)), this, SLOT(updateBpNode(QString)));
  connect(m_subMenus["Quit"], SIGNAL(triggered(bool)), qApp, SLOT(quit()));
  connect(m_subMenus["Capture"], SIGNAL(triggered(bool)), m_map, SLOT(capture()));
  connect(m_subMenus["ZoomIn"], SIGNAL(triggered(bool)), m_map, SLOT(zoomIn()));
  connect(m_subMenus["ZoomOut"], SIGNAL(triggered(bool)), m_map, SLOT(zoomOut()));
  connect(m_subMenus["HideChart"], SIGNAL(triggered(bool)), this, SLOT(hideChart()));
  connect(m_subMenus["Refresh"], SIGNAL(triggered(bool)), this, SLOT(startMonitor()));
  connect(m_subMenus["ChangePassword"], SIGNAL(triggered(bool)), this, SLOT(handleChangePasswordAction(void)));
  connect(m_subMenus["ChangeMonitoringSettings"], SIGNAL(triggered(bool)), this, SLOT(handleChangeMonitoringSettingsAction(void)));
  connect(m_subMenus["ShowAbout"], SIGNAL(triggered(bool)), this, SLOT(handleShowAbout()));
  connect(m_subMenus["ShowOnlineResources"], SIGNAL(triggered(bool)), this, SLOT(handleShowOnlineResources()));
  connect(m_subMenus["BrowserBack"], SIGNAL(triggered(bool)), m_browser, SLOT(back()));
  connect(m_subMenus["BrowserForward"], SIGNAL(triggered(bool)), m_browser, SLOT(forward()));
  connect(m_subMenus["BrowserStop"], SIGNAL(triggered(bool)), m_browser, SLOT(stop()));
  connect(m_subMenus["FullScreen"], SIGNAL(toggled(bool)), this, SLOT(toggleFullScreen(bool)));
  connect(m_subMenus["TroubleView"], SIGNAL(toggled(bool)), this, SLOT(toggleTroubleView(bool)));
  connect(m_subMenus["IncreaseMsgFont"], SIGNAL(toggled(bool)), this, SLOT(toggleIncreaseMsgFont(bool)));
  connect(m_contextMenuList["FilterNodeRelatedMessages"], SIGNAL(triggered(bool)), this, SLOT(filterNodeRelatedMsg()));
  connect(m_contextMenuList["CenterOnNode"], SIGNAL(triggered(bool)), this, SLOT(centerGraphOnNode()));
  connect(m_preferences, SIGNAL(urlChanged(QString)), m_browser, SLOT(setUrl(QString)));
  connect(m_viewPanel, SIGNAL(currentChanged (int)), this, SLOT(tabChanged(int)));
  connect(m_map, SIGNAL(expandNode(QString, bool, qint32)), this, SLOT(expandNode(const QString &, const bool &, const qint32 &)));
  connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(centerGraphOnNode(QTreeWidgetItem *)));
}
