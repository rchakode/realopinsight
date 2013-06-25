/*
 * SvNavigator.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 24-05-2012                                                  #
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


namespace {
  const QString DEFAULT_TIP_PATTERN(QObject::tr("Service: %1\nDescription: %2\nSeverity: %3\n   Calc. Rule: %4\n   Prop. Rule: %5"));
  const QString ALARM_SPECIFIC_TIP_PATTERN(QObject::tr("\nTarget Host: %6\nData Point: %7\nRaw Output: %8\nOther Details: %9"));
  const QString SERVICE_OFFLINE_MSG(QObject::tr("Failed to connect to %1 (%2)"));
  const QString JSON_ERROR_MSG("{\"return_code\": \"-1\", \"message\": \""%SERVICE_OFFLINE_MSG%"\"}");
}

StringMapT SvNavigator::propRules() {
  StringMapT map;
  map.insert(PropRules::label(PropRules::Unchanged), PropRules::toString(PropRules::Unchanged));
  map.insert(PropRules::label(PropRules::Decreased), PropRules::toString(PropRules::Decreased));
  map.insert(PropRules::label(PropRules::Increased), PropRules::toString(PropRules::Increased));
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
    m_cdata (new CoreDataT()),
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
  delete m_cdata;
  delete m_viewPanel;
  delete m_rightSplitter;
  delete m_mainSplitter;
  delete m_preferences;
  delete m_changePasswdWindow;
  delete m_trayIcon;
  unloadMenus();
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

void SvNavigator::runMonitor()
{
  QMainWindow::setEnabled(false);
  prepareUpdate();
  if (m_cdata->monitor == MonitorBroker::Auto) {
    for (SourceListT::Iterator src=m_sources.begin(),end=m_sources.end();
         src!=end; ++src) {
      runMonitor(*src);
    }
  } else {
    SourceListT::Iterator src = m_sources.find(0);
    if (src != m_sources.end()) {
      runMonitor(*src);
    } else {
      utils::alert(tr("The default source is not yet"));
    }
  }
  QMainWindow::setEnabled(true);
}

void SvNavigator::runMonitor(SourceT& src)
{
  openRpcSession(src);
  switch(src.mon_type) {
    case MonitorBroker::Zenoss:
    case MonitorBroker::Zabbix:
      requestZbxZnsData(src);
      break;
    case MonitorBroker::Nagios:
    default:
      m_preferences->useLs()? runLivestatusUpdate(src) : runNagiosUpdate(src);
      break;
  }
}

void SvNavigator::timerEvent(QTimerEvent *)
{
  runMonitor();
}

void  SvNavigator::updateStatusBar(const QString& msg)
{
  statusBar()->showMessage(msg);
}

void SvNavigator::load(const QString& _file)
{
  if (!_file.isEmpty()) m_configFile = utils::getAbsolutePath(_file);
  m_activeFile = m_configFile;
  QMainWindow::setWindowTitle(tr("%1 Operations Console - %2").arg(APP_NAME, m_configFile));
  Parser parser;
  parser.parseSvConfig(m_configFile, *m_cdata);
  m_tree->clear();
  m_tree->addTopLevelItem(m_cdata->tree_items[SvNavigatorTree::RootId]);
  m_map->load(parser.getDotGraphFile(), m_cdata->bpnodes, m_cdata->cnodes);
  resizeDashboard();
  QMainWindow::show();
  m_map->scaleToFitViewPort();
  m_trayIcon->show();
  m_trayIcon->setToolTip(APP_NAME);
  resetSettings();
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
  m_preferences->clearUpdatedSources();
  m_preferences->exec();
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
    for (auto it = m_cdata->cnodes.begin(), end = m_cdata->cnodes.end();
         it != end; ++it) m_msgConsole->updateNodeMsg(it);
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
  SourceListT::Iterator src = m_sources.find(srcId);
  if (src != m_sources.end()) {
    runNagiosUpdate(*src);
  } else {
    updateDashboardOnError(*src, tr("Undefined source (%1)").arg(utils::sourceId(srcId)));
  }
}


void SvNavigator::runNagiosUpdate(const SourceT& src)
{
  CheckT invalidCheck;
  utils::setCheckOnError(MonitorBroker::Unknown, "", invalidCheck);

  /* Check if the handler is connected */
  if (src.d4n_handler->isConnected()) {
    if (src.d4n_handler->getServerSerial() < 110) {
      utils::alert(tr("The server serial %1 is not supported").arg(src.d4n_handler->getServerSerial()));
      return;
    }
  } else {
    invalidCheck.alarm_msg = src.d4n_handler->getErrorMsg();
    QString socketError(invalidCheck.alarm_msg.c_str());
    utils::alert(socketError);
    updateStatusBar(socketError);
    //FIXME: update dashboard according to the current source
    return;
  }

  /* Now start doing the job */
  updateStatusBar(tr("Updating..."));
  for (NodeListIteratorT cnode=m_cdata->cnodes.begin(),
       end=m_cdata->cnodes.end(); cnode!=end; ++cnode)
  {
    if (cnode->child_nodes.isEmpty()) {
      cnode->severity = MonitorBroker::Unknown;
      m_cdata->check_status_count[cnode->severity]+=1;
      continue;
    }

    QPair<QString, QString> info = utils::splitSourceHostInfo(cnode->child_nodes);
    if (info.first != src.id) {
      continue;
    }

    // Retrieve data
    QString msg = src.auth%":"%info.second;
    src.d4n_handler->send(msg.toStdString());
    JsonHelper jsHelper(src.d4n_handler->recv());

    // Treat data
    qint32 ret = jsHelper.getProperty("return_code").toInt32();
    cnode->check.status = (ret!=0)? MonitorBroker::NagiosUnknown : jsHelper.getProperty("status").toInt32();
    cnode->check.host = jsHelper.getProperty("host").toString().toStdString();
    cnode->check.last_state_change = utils::getCtime(jsHelper.getProperty("lastchange").toUInt32());
    cnode->check.check_command = jsHelper.getProperty("command").toString().toStdString();
    cnode->check.alarm_msg = jsHelper.getProperty("message").toString().toStdString();

    computeStatusInfo(cnode, src);
    updateDashboard(cnode);
    m_cdata->check_status_count[cnode->severity]+=1;
    cnode->monitored = true;
  }
}


void SvNavigator::runLivestatusUpdate(int srcId)
{
  SourceListT::Iterator src = m_sources.find(srcId);
  if (src != m_sources.end()) {
    runLivestatusUpdate(*src);
  } else {
    updateDashboardOnError(*src, tr("Undefined sources (%1)").arg(utils::sourceId(srcId)));
  }
}

void SvNavigator::runLivestatusUpdate(const SourceT& src)
{
  /* Check if the handler is connected */
  if (!src.ls_handler->isConnected()) {
    updateDashboardOnError(src, src.ls_handler->errorString());
    return;
  }

  CheckT invalidCheck;
  utils::setCheckOnError(MonitorBroker::Unknown, "", invalidCheck);
  QHashIterator<QString, QStringList> hostit(m_cdata->hosts);
  while (hostit.hasNext()) {
    hostit.next();
    QPair<QString, QString> info = utils::splitSourceHostInfo(hostit.key());
    if (src.ls_handler->loadHostData(info.second)) {
      foreach (const QString& value, hostit.value()) {
        QString key;
        if (value != "ping") {
          key = ID_PATTERN.arg(info.second).arg(value);
        }
        CheckListCstIterT chkit;
        if (src.ls_handler->findCheck(key, chkit)) {
          updateCNodes(*chkit, src);
        } else {
          invalidCheck.id = key.toStdString(); //FIXME: invalidCheck.id = key.toStdString();
          invalidCheck.alarm_msg = tr("Service not found (%1)").arg(key).toStdString();
          updateCNodes(invalidCheck, src);
        }
      }
    }
  }
}


void SvNavigator::prepareUpdate(void)
{
  m_cdata->check_status_count[MonitorBroker::Normal] = 0;
  m_cdata->check_status_count[MonitorBroker::Minor] = 0;
  m_cdata->check_status_count[MonitorBroker::Major] = 0;
  m_cdata->check_status_count[MonitorBroker::Critical] = 0;
  m_cdata->check_status_count[MonitorBroker::Unknown] = 0;
}


void SvNavigator::prepareUpdate(const SourceT& src)
{
  QString msg = QObject::tr("Connecting to %1...");
  switch(src.mon_type) {
    case MonitorBroker::Nagios:
      msg = msg.arg(QString("tcp://%1:%2").arg(src.ls_addr, QString::number(src.ls_port)));
      break;
    case MonitorBroker::Zabbix:
      msg = msg.arg(src.zbx_handler->getApiEndpoint());
      break;
    case MonitorBroker::Zenoss:
      msg = msg.arg(src.zns_handler->getApiBaseEndpoint()); //FIXME: msg.arg(mznsHelper->getApiContextUrl()) crashes
      break;
    default:
      msg = "Unknown source type";
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
  if (_node.type == NodeType::ALARM_NODE)
  {
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
  if (!m_showOnlyTroubles || (m_showOnlyTroubles && _node.severity != MonitorBroker::Normal))
  {
    m_msgConsole->updateNodeMsg(_node);
  }
  emit hasToBeUpdate(_node.parent);
}

void SvNavigator::updateCNodes(const CheckT& check, const SourceT& src)
{
  for (NodeListIteratorT cnode=m_cdata->cnodes.begin(); cnode!=m_cdata->cnodes.end(); ++cnode)
  {
    if (cnode->child_nodes.toLower()==utils::computeRealCheckId(src.id, QString::fromStdString(check.id)).toLower())
    {
      cnode->check = check;
      computeStatusInfo(cnode, src);
      ++(m_cdata->check_status_count[cnode->severity]);
      updateDashboard(cnode);
      cnode->monitored = true;
    }
  }
}

void SvNavigator::finalizeUpdate(const SourceT& src)
{
  if (!m_cdata->cnodes.isEmpty())
  {
    Chart *chart = new Chart;
    QString chartdDetails = chart->update(m_cdata->check_status_count, m_cdata->cnodes.size());
    m_map->updateStatsPanel(chart);
    if (m_chart) delete m_chart; m_chart = chart; m_chart->setToolTip(chartdDetails);
    m_msgConsole->sortByColumn(1, Qt::AscendingOrder);
    m_msgConsole->updateEntriesSize(m_msgConsoleSize); //FIXME: Take care of message wrapping
    resetInterval();
    m_timer = startTimer(m_interval);

    for (NodeListIteratorT cnode = m_cdata->cnodes.begin(),
         end = m_cdata->cnodes.end(); cnode != end; ++cnode)
    {
      if (!cnode->monitored) {
        utils::setCheckOnError(MonitorBroker::Unknown,
                               tr("Undefined service (%1)").arg(cnode->child_nodes),
                               cnode->check);
        computeStatusInfo(cnode, src);
        m_cdata->check_status_count[cnode->severity]+=1;
        updateDashboard(cnode);
        cnode->monitored = true;
      }
      cnode->monitored = false;
    }
  }
  //FIXME: Do this while avoiding searching at each update
  if (!m_cdata->bpnodes.isEmpty()) updateTrayInfo(m_cdata->bpnodes[SvNavigatorTree::RootId]);
}

void SvNavigator::computeStatusInfo(NodeListT::iterator&  _node, const SourceT& src)
{
  computeStatusInfo(*_node, src);
}

void SvNavigator::computeStatusInfo(NodeT& _node, const SourceT& src)
{
  QRegExp regexp;
  _node.severity = utils::computeCriticity(src.mon_type, _node.check.status);
  _node.prop_sev = utils::computePropCriticity(_node.severity, _node.sev_prule);
  _node.actual_msg = QString::fromStdString(_node.check.alarm_msg);

  if (_node.check.host == "-") {
    return;
  }

  if (m_cdata->monitor == MonitorBroker::Zabbix)
  {
    regexp.setPattern(MsgConsole::TAG_ZABBIX_HOSTNAME);
    _node.actual_msg.replace(regexp, _node.check.host.c_str());
    regexp.setPattern(MsgConsole::TAG_ZABBIX_HOSTNAME2);
    _node.actual_msg.replace(regexp, _node.check.host.c_str());
  }

  if (_node.severity == MonitorBroker::Normal)
  {
    if (_node.notification_msg.isEmpty()) {
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
  if (m_cdata->monitor == MonitorBroker::Nagios) {
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
  if (!utils::findNode(m_cdata, _nodeId, node)) return;

  QStringList nodeIds = node->child_nodes.split(Parser::CHILD_SEP);
  Criticity criticity;
  foreach (const QString& nodeId, nodeIds) {
    NodeListT::iterator child;
    if (!utils::findNode(m_cdata, nodeId, child)) continue;
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
    case PropRules::Unchanged:
    default:
      node->prop_sev = node->severity;
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
  auto tnode_it = m_cdata->tree_items.find(_node.id);
  if (tnode_it != m_cdata->tree_items.end()) {
    (*tnode_it)->setIcon(0, utils::computeCriticityIcon(_node.severity));
    (*tnode_it)->setToolTip(0, _tip);
  }
}

void SvNavigator::expandNode(const QString& _nodeId, const bool& _expand, const qint32& _level)
{
  auto node = m_cdata->bpnodes.find(_nodeId);
  if (node == m_cdata->bpnodes.end()) {
    return;
  }
  if (!node->child_nodes.isEmpty()) {
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
  if (utils::findNode(m_cdata, m_selectedNode, node)) {
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
  if (utils::findNode(m_cdata, _nodeId, node) &&
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


void SvNavigator::processZbxReply(QNetworkReply* _reply, SourceT& src)
{
  _reply->deleteLater();
  QNetworkReply::NetworkError errcode = _reply->error();
  if (errcode != QNetworkReply::NoError) {
    processRpcError(errcode, src);
    return;
  }
  QString data = _reply->readAll();
  JsonHelper jsHelper(data);
  QString errmsg = jsHelper.getProperty("error").property("data").toString();
  if (errmsg.isEmpty()) errmsg = jsHelper.getProperty("error").property("message").toString();
  if (!errmsg.isEmpty()) {
    updateDashboardOnError(src, errmsg);
    return;
  }
  qint32 tid = jsHelper.getProperty("id").toInt32();
  switch(tid) {
    case ZbxHelper::Login: {
      QString auth = jsHelper.getProperty("result").toString();
      if (!auth.isEmpty()) {
        src.zbx_handler->setAuth(auth);
        src.zbx_handler->setIsLogged(true);
      }
      break;
    }
    case ZbxHelper::ApiVersion: {
      src.zbx_handler->setTrid(jsHelper.getProperty("result").toString());
      break;
    }
    case ZbxHelper::Trigger:
    case ZbxHelper::TriggerV18: {
      QScriptValueIterator trigger(jsHelper.getProperty("result"));
      CheckT check;
      while (trigger.hasNext())
      {
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
        if (host.hasNext())
        {
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
        updateCNodes(check, src);
      }
      break;
    }
    default :
      updateDashboardOnError(src, tr("Weird response received from the server"));
      qDebug() << data;
      break;
  }
}

void SvNavigator::processZnsReply(QNetworkReply* _reply, SourceT& src)
{
  _reply->deleteLater();

  if (_reply->error() != QNetworkReply::NoError) {
    processRpcError(_reply->error(), src);
    return;
  }

  QVariant cookiesContainer = _reply->header(QNetworkRequest::SetCookieHeader);
  QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(cookiesContainer);
  QString data = _reply->readAll();
  if (data.endsWith("submitted=true")) {
    src.zns_handler->cookieJar()->setCookiesFromUrl(cookies, src.zns_handler->getApiBaseEndpoint());
    src.zns_handler->setIsLogged(true);
  } else {
    JsonHelper jsonHelper(data);
    qint32 tid = jsonHelper.getProperty("tid").toInt32();
    QScriptValue result = jsonHelper.getProperty("result");
    bool reqSucceed = result.property("success").toBool();
    if (!reqSucceed) {
      updateDashboardOnError(src, tr("Authentication failed: %1").arg(result.property("msg").toString()));
      return;
    }
    if (tid == ZnsHelper::Device) {
      QScriptValueIterator devices(result.property("devices"));
      while(devices.hasNext())
      {
        devices.next(); if (devices.flags()&QScriptValue::SkipInEnumeration) continue;

        QScriptValue ditem = devices.value();
        QString duid = ditem.property("uid").toString();
        QNetworkReply* reply = src.zns_handler->postRequest(ZnsHelper::Component,
                                                            ZnsHelper::ReqPatterns[ZnsHelper::Component]
                                                            .arg(duid, QString::number(ZnsHelper::Component))
                                                            .toAscii());
        processZnsReply(reply, src);

        QString dname = ditem.property("name").toString();
        if (m_cdata->hosts[dname].contains("ping", Qt::CaseInsensitive)) {
          reply = src.zns_handler->postRequest(ZnsHelper::Device,
                                               ZnsHelper::ReqPatterns[ZnsHelper::DeviceInfo]
                                               .arg(duid, QString::number(ZnsHelper::DeviceInfo))
                                               .toAscii());
          processZnsReply(reply, src);
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
          updateCNodes(check, src);
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
        updateCNodes(check, src);
      } else {
        updateDashboardOnError(src, tr("Weird response received from the server"));
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


void SvNavigator::openRpcSessions(void)
{
  for (SourceListT::Iterator src = m_sources.begin(), end = m_sources.end();
       src != end; ++src) {
    openRpcSession(*src);
  }
}

void SvNavigator::openRpcSession(int srcId)
{
  SourceListT::Iterator src = m_sources.find(srcId);
  if (src != m_sources.end()) {
    switch (src->mon_type) {
      case MonitorBroker::Zabbix:
        src->zbx_handler->setIsLogged(false);
        break;
      case MonitorBroker::Zenoss:
        src->zns_handler->setIsLogged(false);
        break;
      default:
        break;
    }
  }
  openRpcSession(*src);
}


void SvNavigator::openRpcSession(SourceT& src)
{
  QStringList authParams = getAuthInfo(src.auth);
  if (authParams.size() != 2 && src.mon_type != MonitorBroker::Nagios) {
    updateDashboardOnError(src, tr("Invalid authentication chain!\n"
                                   "Must follow the pattern login:password"));
    return;
  }
  QUrl znsUrlParams;
  switch(src.mon_type) {
    case MonitorBroker::Nagios:
      if (m_preferences->useLs()) {
        if (src.ls_handler->isConnected()) { //FIXME: do this???
          src.ls_handler->disconnectFromService();
        }
        src.ls_handler->connectToService();
      } else {
        if (src.d4n_handler->isConnected()) {
          src.d4n_handler->disconnecteFromService();
        }
        if(src.d4n_handler->connect()) {
          src.d4n_handler->makeHandShake();
        }
      }
      break;
    case MonitorBroker::Zabbix: {
      src.zbx_handler->setBaseUrl(src.mon_url);
      authParams.push_back(QString::number(ZbxHelper::Login));
      src.zbx_handler->setSslConfig(src.verify_ssl_peer);
      QNetworkReply* reply = src.zbx_handler->postRequest(ZbxHelper::Login, authParams);
      processZbxReply(reply, src);
      if (src.zbx_handler->getIsLogged()) {
        // The get API version
        QStringList params;
        params.push_back(QString::number(ZbxHelper::ApiVersion));
        src.zbx_handler->setSslConfig(src.verify_ssl_peer);
        reply = src.zbx_handler->postRequest(ZbxHelper::ApiVersion, params);
        processZbxReply(reply, src);
      }
      break;
    }
    case MonitorBroker::Zenoss: {
      src.zns_handler->setBaseUrl(src.mon_url);
      znsUrlParams.addQueryItem("__ac_name", authParams[0]);
      znsUrlParams.addQueryItem("__ac_password", authParams[1]);
      znsUrlParams.addQueryItem("submitted", "true");
      znsUrlParams.addQueryItem("came_from", src.zns_handler->getApiContextEndpoint());
      src.zns_handler->setSslConfig(src.verify_ssl_peer);
      QNetworkReply* reply = src.zns_handler->postRequest(ZnsHelper::Login, znsUrlParams.encodedQuery());
      processZnsReply(reply, src);
    }
      break;
    default:
      break;
  }
}


void SvNavigator::requestZbxZnsData(SourceT& src) {
  updateStatusBar(tr("Updating..."));
  switch(src.mon_type) {
    case MonitorBroker::Zabbix: {
      /* First check that we're logged */
      if (src.zbx_handler->getIsLogged()) {
        int trid = src.zbx_handler->getTrid();
        foreach (const QString& hitem, m_cdata->hosts.keys()) {
          QStringList params;
          params.push_back(utils::getHostFromSourceStr(hitem));
          params.push_back(QString::number(trid));
          QNetworkReply* reply = src.zbx_handler->postRequest(trid, params);
          processZbxReply(reply, src);
        }
      }
      break;
    }
    case MonitorBroker::Zenoss: {
      /* First check that we're logged */
      if (src.zns_handler->getIsLogged()) {
        src.zns_handler->setRouterEndpoint(ZnsHelper::Device);
        foreach (const QString& hitem, m_cdata->hosts.keys()) {
          QNetworkReply* reply = src.zns_handler->postRequest(ZnsHelper::Device,
                                                              ZnsHelper::ReqPatterns[ZnsHelper::Device]
                                                              .arg(utils::getHostFromSourceStr(hitem),
                                                                   QString::number(ZnsHelper::Device))
                                                              .toAscii());
          processZnsReply(reply, src);
        }
      }
      break;
    }
    default:
      break;
  }
}

void SvNavigator::processRpcError(QNetworkReply::NetworkError _code, const SourceT& src)
{
  QString apiUrl = "";
  if (src.mon_type == MonitorBroker::Zabbix) {
    apiUrl = src.zbx_handler->getApiEndpoint();
  } else if (src.mon_type == MonitorBroker::Zenoss) {
    apiUrl =  src.zns_handler->getRequestEndpoint();
  }
  QString errmsg;
  switch (_code) {
    case QNetworkReply::RemoteHostClosedError:
      errmsg = "The connection has been closed by the remote host";
      break;
    case QNetworkReply::HostNotFoundError:
      errmsg = "Host not found";
      break;
    case QNetworkReply::ConnectionRefusedError:
      errmsg = "Connection refused";
      break;
    case QNetworkReply::SslHandshakeFailedError:
      errmsg = tr("SSL Handshake failed");
      break;
    case QNetworkReply::TimeoutError:
      errmsg = tr("Timeout exceeded");
      break;
    default:
      errmsg = SERVICE_OFFLINE_MSG.arg(apiUrl, tr("error %1").arg(_code));
  }
  updateDashboardOnError(src, errmsg);
}

void SvNavigator::updateDashboardOnError(const SourceT& src, const QString& msg)
{
  if (!msg.isEmpty()) {
    utils::alert(msg);
    updateStatusBar(msg);
  }
  for (NodeListIteratorT cnode = m_cdata->cnodes.begin(); cnode != m_cdata->cnodes.end(); ++cnode) {

    CheckIdInfoT info = utils::splitSourceHostInfo(cnode->child_nodes);
    if ((info.first.isEmpty() && src.id == utils::sourceId(0)) ||
        (!info.first.isEmpty() && info.first == src.id)) {
      utils::setCheckOnError(MonitorBroker::Unknown, msg, cnode->check);
      computeStatusInfo(cnode, src);
      updateDashboard(cnode);
      m_cdata->check_status_count[MonitorBroker::Unknown]+=1;
      cnode->monitored = true;
    }
  }
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
  qint32 pbCount = m_cdata->cnodes.size() - m_cdata->check_status_count[MonitorBroker::Normal];
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
  m_sources.clear();
  SourceT src;
  for (auto id = m_cdata->sources.begin(),
       end = m_cdata->sources.end();
       id!=end; ++id) {
    QPair<bool, int> srcinfo = utils::checkSourceId(*id);
    if (srcinfo.first) {
      if (m_preferences->isSetSource(srcinfo.second) &&
          m_settings->loadSource(*id, src) ) {
        allocSourceHandler(src);
        m_sources.insert(srcinfo.second, src);
      } else {
        utils::alert(tr("This source is not set (%1)").arg(*id));
      }
    } else {
      utils::alert(tr("Could not handle this source (%1)").arg(*id));
    }

  }
  resetInterval();
  m_browser->setUrl(m_sources[0].mon_url); //FIXME: m_browser->setUrl(m_sources[0].mon_url);
}

void SvNavigator::resetInterval()
{
  m_interval = 1000 * m_settings->getUpdateInterval();
  killTimer(m_timer);
  m_timer = startTimer(m_interval);
}

void SvNavigator::allocSourceHandler(SourceT& src)
{
  if (src.mon_type == MonitorBroker::Auto) {
    src.mon_type = m_cdata->monitor;
  }

  switch (src.mon_type) {
    case MonitorBroker::Nagios:
      if (m_preferences->useLs()) {
        src.ls_handler = std::make_shared<LsHelper>(src.ls_addr, src.ls_port);
      } else {
        QString uri = QString("tcp://%1:%2").arg(src.ls_addr, QString::number(src.ls_port));
        src.d4n_handler = std::make_shared<ZmqSocket>(uri.toStdString(), ZMQ_REQ);
      }
      break;
    case MonitorBroker::Zabbix:
      src.zbx_handler = std::make_shared<ZbxHelper>();
      break;
    case MonitorBroker::Zenoss:
      src.zns_handler = std::make_shared<ZnsHelper>();
      break;
    default:
      utils::alert(tr("Unknown monitor type (%1").arg(src.mon_type));
      break;
  }
}


void SvNavigator::handleSourcesChanged(QList<qint8> ids)
{
  foreach (const qint8& id, ids) {
    SourceT newsrc;
    m_settings->loadSource(id, newsrc);
    SourceListT::Iterator olddata = m_sources.find(id);
    if (olddata != m_sources.end()) {
      switch (olddata->mon_type) {
        case MonitorBroker::Nagios:
          if (m_preferences->useLs()) {
            olddata->ls_handler.reset();
          } else {
            olddata->d4n_handler.reset();
          }
          break;
        case MonitorBroker::Zabbix:
          olddata->zbx_handler.reset();
          break;
        case MonitorBroker::Zenoss:
          olddata->zns_handler.reset();
          break;
        default:
          utils::alert(tr("Unknown monitor type (%1)").arg(olddata->mon_type));
          break;
      }
    }
    allocSourceHandler(newsrc);
    m_sources[id] = newsrc;
    if (id == 0) {
      m_browser->setUrl(newsrc.mon_url);
    }
  }
  runMonitor();
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

void SvNavigator::addEvents(void)
{
  connect(this, SIGNAL(hasToBeUpdate(QString)), this, SLOT(updateBpNode(QString)));
  connect(m_subMenus["Quit"], SIGNAL(triggered(bool)), qApp, SLOT(quit()));
  connect(m_subMenus["Capture"], SIGNAL(triggered(bool)), m_map, SLOT(capture()));
  connect(m_subMenus["ZoomIn"], SIGNAL(triggered(bool)), m_map, SLOT(zoomIn()));
  connect(m_subMenus["ZoomOut"], SIGNAL(triggered(bool)), m_map, SLOT(zoomOut()));
  connect(m_subMenus["HideChart"], SIGNAL(triggered(bool)), this, SLOT(hideChart()));
  connect(m_subMenus["Refresh"], SIGNAL(triggered(bool)), this, SLOT(runMonitor()));
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
  connect(m_preferences, SIGNAL(sourcesChanged(QList<qint8>)), this, SLOT(handleSourcesChanged(QList<qint8>)));
  connect(m_viewPanel, SIGNAL(currentChanged (int)), this, SLOT(tabChanged(int)));
  connect(m_map, SIGNAL(expandNode(QString, bool, qint32)), this, SLOT(expandNode(const QString &, const bool &, const qint32 &)));
  connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(centerGraphOnNode(QTreeWidgetItem *)));
}
