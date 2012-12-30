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
#include <sstream>
#include <QStatusBar>
#include <QObject>
#include <zmq.h>
#include<iostream>


const QString DEFAULT_TIP_PATTERN(QObject::tr("Service: %1\nDescription: %2\nStatus: %3\n   Calc. Rule: %4\n   Prop. Rule: %5"));
const QString ALARM_SPECIFIC_TIP_PATTERN(QObject::tr("\nTarget Host: %6\nCheck/Trigger ID: %7\nCheck Output: %8\nMore info: %9"));
const QString SERVICE_OFFLINE_MSG(QObject::tr("Failed to connect to %1"));
const QString DEFAULT_ERROR_MSG("{\"return_code\": \"-1\", \"message\": \""%SERVICE_OFFLINE_MSG%"\"}");
const QString ID_PATTERN("%1/%2");

ComboBoxItemsT SvNavigator::propRules() {
  ComboBoxItemsT map;
  map.insert(StatusPropRules::label(StatusPropRules::Unchanged),
             StatusPropRules::toString(StatusPropRules::Unchanged));
  map.insert(StatusPropRules::label(StatusPropRules::Decreased),
             StatusPropRules::toString(StatusPropRules::Decreased));
  map.insert(StatusPropRules::label(StatusPropRules::Increased),
             StatusPropRules::toString(StatusPropRules::Increased));
  return map;
}

ComboBoxItemsT SvNavigator::calcRules() {
  ComboBoxItemsT map;
  map.insert(StatusCalcRules::label(StatusCalcRules::HighCriticity),
             StatusCalcRules::toString(StatusCalcRules::HighCriticity));
  map.insert(StatusCalcRules::label(StatusCalcRules::WeightedCriticity),
             StatusCalcRules::toString(StatusCalcRules::WeightedCriticity));
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
    mstatsPanel (new Stats()),
    mfilteredMsgPanel (NULL),
    mmainSplitter (new QSplitter(this)),
    mrightSplitter (new QSplitter()),
    mtopRightPanel (new QTabWidget()),
    mbottomRightPanel (new QTabWidget()),
    mbrowser (new WebKit()),
    mmap (new GraphView()),
    mtree (new SvNavigatorTree()),
    mprefWindow (new Preferences(_userRole, Preferences::ChangeMonitoringSettings)),
    mchangePasswdWindow (new Preferences(_userRole, Preferences::ChangePassword)),
    mmsgPanel(new MsgPanel()),
    mnodeContextMenu (new QMenu()),
    mzbxHelper(new ZbxHelper()),
    mzbxAuthToken(""),
    mhostLeft(0),
    mznsHelper(new ZnsHelper()),
    misLogged(false)
{
  setWindowTitle(tr("%1 Operations Console").arg(appName));
  loadMenus();
  mtopRightPanel->addTab(mmap, tr("Dashboard"));
  mtopRightPanel->addTab(mbrowser, tr("Native Web UI"));
  mbottomRightPanel->addTab(mmsgPanel, tr("Event Console"));
  mmainSplitter->addWidget(mtree);
  mmainSplitter->addWidget(mrightSplitter);
  mrightSplitter->addWidget(mtopRightPanel);
  mrightSplitter->addWidget(mbottomRightPanel);
  mrightSplitter->setOrientation(Qt::Vertical);
  setCentralWidget(mmainSplitter);
  updateMonitoringSettings();
  addEvents();
}

SvNavigator::~SvNavigator()
{
  delete mmsgPanel;
  delete mstatsPanel;
  delete mtree;
  delete mbrowser;
  delete mmap;
  delete mcoreData;
  delete mtopRightPanel;
  delete mbottomRightPanel;
  delete mrightSplitter;
  delete mmainSplitter;
  delete mprefWindow;
  delete mchangePasswdWindow;
  delete mzbxHelper;
  delete mznsHelper;
  if(mfilteredMsgPanel) delete mfilteredMsgPanel;
  unloadMenus();
}

void SvNavigator::closeEvent(QCloseEvent * event)
{
  if(mfilteredMsgPanel)
    mfilteredMsgPanel->close();
  QMainWindow::closeEvent(event);
}


void SvNavigator::contextMenuEvent(QContextMenuEvent * event)
{
  QPoint pos = event->globalPos();
  QList<QTreeWidgetItem*> treeNodes = mtree->selectedItems();
  QGraphicsItem* graphNode = mmap->nodeAtGlobalPos(pos);

  if (treeNodes.length() || graphNode) {
      if(graphNode) {
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
  updateStatusBar(tr("Connecting to the server..."));
  mupdateSucceed = true ;
  setEnabled(false);
  switch(mcoreData->monitor){
    case MonitorBroker::ZENOSS:
    case MonitorBroker::ZABBIX:
      !misLogged ? openRpcSession(): postRpcDataRequest();
      break;
    case MonitorBroker::NAGIOS:
    default:
      runNagiosMonitor();
      break;
    }
  setEnabled(true);
  mupdateInterval = msettings->value(Preferences::UPDATE_INTERVAL_KEY).toInt();
  mupdateInterval = 1000 * ((mupdateInterval > 0)? mupdateInterval :
                                                   MonitorBroker::DEFAULT_UPDATE_INTERVAL);
  mtimer = startTimer(mupdateInterval);
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
  if( ! _file.isEmpty()) {
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
  msubMenuList.clear();
  mmenuList.clear();
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
  msocket = new Socket(ZMQ_REQ);
  msocket->connect(mserverUrl.toStdString());
  msocket->makeHandShake();

  if(! msocket->isConnected2Server()) {
      QString msg = SERVICE_OFFLINE_MSG.arg(mserverUrl);
      utils::alert(msg);
      updateStatusBar(msg);
      mupdateSucceed = false ;
    } else {
      if(msocket->getServerSerial() < 110) {
          utils::alert(tr("The server %1 is not supported")
                       .arg(msocket->getServerSerial()));
          mupdateSucceed = false;
        }
      updateStatusBar(tr("Updating..."));
    }
  resetStatData();
  mmsgPanel->setSortingEnabled(false);

  foreach(const QString& checkId, mcoreData->cnodes.keys()) {

      NodeListT::iterator cnode = mcoreData->cnodes.find(checkId);
      if(cnode == mcoreData->cnodes.end())
        continue;
      if(cnode->child_nodes == "") {
          cnode->criticity = MonitorBroker::CRITICITY_UNKNOWN;
          mcoreData->check_status_count[cnode->criticity]++;
          continue;
        }

      QStringList checks = cnode->child_nodes.split(Parser::CHILD_SEP);
      foreach(const QString& cid, checks) {
          QString msg = mserverAuthChain%":"%cid;
          if(mupdateSucceed) {
              msocket->send(msg.toStdString());
              msg = QString::fromStdString(msocket->recv());
            } else {
              msg = DEFAULT_ERROR_MSG.arg(mserverUrl);
            }
          JsonHelper jsHelper(msg.toStdString());
          if(jsHelper.getProperty("return_code").toInt32() == 0
             && msocket->isConnected2Server()) {
              cnode->check.status = jsHelper.getProperty("status").toInt32();
              cnode->check.host = jsHelper.getProperty("host").toString().toStdString();
              cnode->check.last_state_change = jsHelper.getProperty("lastchange").toString().toStdString();
              cnode->check.check_command = jsHelper.getProperty("command").toString().toStdString();
              cnode->check.alarm_msg = jsHelper.getProperty("message").toString().toStdString();
            } else {
              cnode->check.status = MonitorBroker::NAGIOS_UNKNOWN;
              cnode->check.host = "Unknown";
              cnode->check.last_state_change = "0";
              cnode->check.check_command = "Unknown" ;
              cnode->check.alarm_msg = jsHelper.getProperty("message").toString().toStdString();
            }
          updateCNode(cnode);
          mcoreData->check_status_count[cnode->criticity]++;
        }
    }
  msocket->disconnect();
  updateStats();

  return 0;
}

void SvNavigator::resetStatData(void)
{
  mcoreData->check_status_count[MonitorBroker::CRITICITY_NORMAL] = 0;
  mcoreData->check_status_count[MonitorBroker::CRITICITY_MINOR] = 0;
  mcoreData->check_status_count[MonitorBroker::CRITICITY_MAJOR] = 0;
  mcoreData->check_status_count[MonitorBroker::CRITICITY_HIGH] = 0;
  mcoreData->check_status_count[MonitorBroker::CRITICITY_UNKNOWN] = 0;
  mhostLeft = mcoreData->hosts.size();
}


QString SvNavigator::getNodeToolTip(const NodeT& _node)
{
  QString toolTip = DEFAULT_TIP_PATTERN.arg(_node.name)
      .arg(const_cast<QString&>(_node.description).replace("\n", " "))
      .arg(utils::statusToString(_node.criticity))
      .arg(StatusCalcRules::label(_node.status_crule))
      .arg(StatusPropRules::label(_node.status_prule));

  if (_node.type == NodeType::ALARM_NODE) {
      QString msg = "";
      if(_node.criticity == MonitorBroker::CRITICITY_NORMAL) {
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


void SvNavigator::updateCNode(NodeListT::iterator& _node) {

  _node->criticity = _node->prop_status = utils::getCriticity(mcoreData->monitor, _node->check.status);
  updateAlarmMsg(_node);
  QString toolTip = getNodeToolTip(*_node);
  updateNavTreeItemStatus(_node, toolTip);
  mmap->updateNode(_node, toolTip);
  mmsgPanel->addMsg(_node);

  emit hasToBeUpdate(_node->parent);
}

void SvNavigator::updateStats() {

  qint64 nbChecks = mcoreData->cnodes.size();
  if(nbChecks == 0)
    return;

  Stats *stats = new Stats;
  QString info = stats->update(mcoreData->check_status_count, nbChecks);
  stats->setToolTip(info);
  mmap->updateStatsPanel(stats);

  if(mstatsPanel)
    delete mstatsPanel;
  mstatsPanel = stats;
  mmsgPanel->sortItems(MsgPanel::msgPanelColumnCount - 1, Qt::DescendingOrder);
  mmsgPanel->resizeFields(mmsgPanelSize);

  if(mupdateSucceed)
    updateStatusBar(tr("Update completed"));
}

void SvNavigator::updateAlarmMsg(NodeListT::iterator&  _node)
{
  QString msg = "";
  if(_node->criticity == MonitorBroker::CRITICITY_NORMAL) {
      msg = _node->notification_msg;
    } else {
      msg = _node->alarm_msg;
    }

  QRegExp regexp(MsgPanel::HOSTNAME_META_MSG_PATERN);
  QStringList checkIds = QString(_node->check.id.c_str()).split("/");
  qint32 nbChecks =  checkIds.length();
  if(nbChecks) {
      msg.replace(regexp, checkIds[0]);
      if(nbChecks == 2) {
          regexp.setPattern(MsgPanel::SERVICE_META_MSG_PATERN);
          msg.replace(regexp, checkIds[1]);
        }
    }
  // FIXME: generalize it to other monitor
  QStringList splitedCcommand = QString(_node->check.check_command.c_str()).split("!");
  if(splitedCcommand.length() >= 3) {
      regexp.setPattern(MsgPanel::THERESHOLD_META_MSG_PATERN);
      msg.replace(regexp, splitedCcommand[1]);

      if(_node->criticity == MonitorBroker::CRITICITY_MAJOR)
        msg.replace(regexp, splitedCcommand[2]);
    }

  if(_node->criticity == MonitorBroker::CRITICITY_NORMAL) {
      _node->notification_msg = msg;
    } else {
      _node->alarm_msg = msg;
    }
}

void SvNavigator::updateBpNode(QString _nodeId)
{
  NodeListT::iterator node;
  if(!utils::findNode(mcoreData, _nodeId, node))
    return;
  Criticity criticity;
  QStringList nodeIds = node->child_nodes.split(Parser::CHILD_SEP);
  for(QStringList::const_iterator it = nodeIds.begin(); it != nodeIds.end(); it++) {
      NodeListT::iterator child;
      if(! utils::findNode(mcoreData, *it, child))
        continue;
      Criticity cst(static_cast<MonitorBroker::CriticityT>(child->prop_status));
      if(node->status_crule == StatusCalcRules::WeightedCriticity) {
          criticity = criticity / cst;
        } else {
          criticity = criticity * cst;
        }
    }
  node->criticity = criticity.getValue();
  switch(node->status_prule) {
    case StatusPropRules::Increased: node->prop_status = (criticity++).getValue();
      break;
    case StatusPropRules::Decreased: node->prop_status = (criticity--).getValue();
      break;
    default: node->prop_status = node->criticity;
      break;

    }
  QString toolTip = getNodeToolTip(*node);
  mmap->updateNode(node, toolTip);
  updateNavTreeItemStatus(node, toolTip);
  emit hasToBeUpdate(node->parent);
}


void SvNavigator::updateNavTreeItemStatus(const NodeListT::iterator& _node, const QString& _tip)
{

  TreeNodeItemListT::iterator tnode_it = mcoreData->tree_items.find(_node->id);
  if(tnode_it != mcoreData->tree_items.end()) {
      (*tnode_it)->setIcon(0, utils::getTreeIcon(_node->criticity));
      (*tnode_it)->setToolTip(0, _tip);
    }
}

void SvNavigator::updateMonitoringSettings(){
  mmonitorBaseUrl = msettings->value(Preferences::URL_KEY).toString();
  mserverAuthChain = msettings->value(Preferences::SERVER_PASS_KEY).toString();
  mserverAddr = msettings->value(Preferences::SERVER_ADDR_KEY).toString();
  mserverPort = msettings->value(Preferences::SERVER_PORT_KEY).toString();
  mserverUrl = QString("tcp://%1:%2").arg(mserverAddr).arg(mserverPort);
  mupdateInterval = msettings->value(Preferences::UPDATE_INTERVAL_KEY).toInt() * 1000;
  if (mupdateInterval <= 0) mupdateInterval = MonitorBroker::DEFAULT_UPDATE_INTERVAL * 1000;
}

void SvNavigator::expandNode(const QString& _nodeId,
                             const bool& _expand,
                             const qint32& _level)
{
  NodeListT::iterator node = mcoreData->bpnodes.find(_nodeId);
  if(node == mcoreData->bpnodes.end()) {
      return ;
    }

  if(node->child_nodes != "") {
      QStringList  childNodes = node->child_nodes.split(Parser::CHILD_SEP);
      for (QStringList::iterator udsIt = childNodes.begin(); udsIt != childNodes.end(); udsIt++) {
          mmap->setNodeVisible(*udsIt, _nodeId, _expand, _level);
        }
    }
}

void SvNavigator::centerGraphOnNode(const QString& _node_id)
{
  if(_node_id != "") mselectedNode =  _node_id;
  mmap->centerOnNode(mselectedNode);
}

void SvNavigator::filterNodeRelatedMsg(void)
{
  if(mfilteredMsgPanel)
    delete mfilteredMsgPanel;

  mfilteredMsgPanel = new MsgPanel();
  NodeListT::iterator node;
  if(utils::findNode(mcoreData, mselectedNode, node)) {
      filterNodeRelatedMsg(mselectedNode);
      QString title = tr("Messages related to '%2' - %1")
          .arg(appName)
          .arg(node->name);
      mfilteredMsgPanel->resizeFields(mmsgPanelSize, true);
      mfilteredMsgPanel->setWindowTitle(title);
    }

  mfilteredMsgPanel->show();
}

void SvNavigator::filterNodeRelatedMsg(const QString& _nodeId)
{
  NodeListT::iterator node;
  if(utils::findNode(mcoreData, _nodeId, node) &&
     node->child_nodes != "") {  // Warning: take care in short-circuit evaluation!!!
      if (node->type == NodeType::ALARM_NODE) {
          mfilteredMsgPanel->addMsg(node);
        } else {
          QStringList childIds = node->child_nodes.split(Parser::CHILD_SEP);
          foreach(QString checkId, childIds) {
              filterNodeRelatedMsg(checkId);
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
  if (_index != 0) {
      msubMenuList["ZoomIn"]->setEnabled(false);
      msubMenuList["ZoomOut"]->setEnabled(false);
      return;
    }

  msubMenuList["ZoomIn"]->setEnabled(true);
  msubMenuList["ZoomOut"]->setEnabled(true);
}

void SvNavigator::hideChart(void)
{
  if (mmap->hideChart()) {
      msubMenuList["HideChart"]->setIcon(QIcon(":images/check.png"));
      return;
    }
  msubMenuList["HideChart"]->setIcon(QIcon(""));
}

void SvNavigator::centerGraphOnNode(QTreeWidgetItem * _item)
{
  centerGraphOnNode(_item->data(0, QTreeWidgetItem::UserType).toString());
}

void SvNavigator::resize(void)
{
  const qreal GRAPH_HEIGHT_RATE = 0.50;
  QSize screenSize = qApp->desktop()->screen(0)->size();
  mmsgPanelSize = QSize(screenSize.width() * 0.80, screenSize.height() * (1.0 - GRAPH_HEIGHT_RATE));

  QList<qint32> framesSize;
  framesSize.push_back(screenSize.width() * 0.20);
  framesSize.push_back(mmsgPanelSize.width());
  mmainSplitter->setSizes(framesSize);

  framesSize[0] = (screenSize.height() * GRAPH_HEIGHT_RATE);
  framesSize[1] = (mmsgPanelSize.height());
  mrightSplitter->setSizes(framesSize);

  mmainSplitter->resize(screenSize.width(), screenSize.height() * 0.85);
  QMainWindow::resize(screenSize.width(),  screenSize.height());
}


void SvNavigator::loadMenus(void)
{
  QIcon refreshIcon(":images/built-in/refresh.png");
  QIcon cameraIcon(":images/built-in/camera.png");
  QIcon zoominIcon(":images/built-in/zoomin.png");
  QIcon zoomoutIcon(":images/built-in/zoomout.png");

  QMenuBar* menuBar = new QMenuBar();
  mmenuList["MENU1"] = menuBar->addMenu("&File"),
      msubMenuList["Refresh"] = mmenuList["MENU1"]->addAction(refreshIcon, tr("&Refresh Screen")),
      msubMenuList["Capture"] = mmenuList["MENU1"]->addAction(cameraIcon, tr("&Save Map as Image")),
      mmenuList["MENU1"]->addSeparator(),
      msubMenuList["Quit"] = mmenuList["MENU1"]->addAction(tr("&Quit"));

  mmenuList["MENU2"] = menuBar->addMenu("&Map"),
      msubMenuList["ZoomIn"] = mmenuList["MENU2"]->addAction(zoominIcon, tr("Zoom &In")),
      msubMenuList["ZoomOut"] = mmenuList["MENU2"]->addAction(zoomoutIcon, tr("Zoom &Out")),
      msubMenuList["HideChart"] = mmenuList["MENU2"]->addAction(tr("Hide &Chart"));

  mmenuList["MENU3"] = menuBar->addMenu("&Preferences"),
      msubMenuList["ChangePassword"] = mmenuList["MENU3"]->addAction(tr("Change &Password")),
      msubMenuList["ChangeMonitoringSettings"] = mmenuList["MENU3"]->addAction(tr("&Monitoring Settings"));

  mmenuList["MENU4"] = menuBar->addMenu("&Help"),
      msubMenuList["ShowOnlineResources"] = mmenuList["MENU4"]->addAction(tr("Online &Resources")),
      mmenuList["MENU4"]->addSeparator(),
      msubMenuList["ShowAbout"] = mmenuList["MENU4"]->addAction(tr("&About %1").arg(appName));

  msubMenuList["Capture"]->setShortcut(QKeySequence::Save);
  msubMenuList["Refresh"]->setShortcut(QKeySequence::Refresh);
  msubMenuList["ZoomIn"]->setShortcut(QKeySequence::ZoomIn);
  msubMenuList["ZoomOut"]->setShortcut(QKeySequence::ZoomOut);
  msubMenuList["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
  msubMenuList["Quit"]->setShortcut(QKeySequence::Quit);

  mcontextMenuList["FilterNodeRelatedMessages"] = mnodeContextMenu->addAction(tr("&Filter related messages"));
  mcontextMenuList["CenterOnNode"] = mnodeContextMenu->addAction(tr("Center Graph &On"));
  mcontextMenuList["Cancel"] = mnodeContextMenu->addAction(tr("&Cancel"));

  QToolBar* toolBar = addToolBar(QString(ngrt4n::APP_NAME.c_str()));
  toolBar->addAction(msubMenuList["Refresh"]);
  toolBar->addAction(msubMenuList["ZoomIn"]);
  toolBar->addAction(msubMenuList["ZoomOut"]);
  toolBar->addAction(msubMenuList["Capture"]);
  toolBar->setIconSize(QSize(16,16));
  setMenuBar(menuBar);
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

  if (_reply->error() != QNetworkReply::NoError) {
      return;
    }

  string data = static_cast<QString>(_reply->readAll()).toStdString();
  JsonHelper jsHelper(data);
  qint32 dataId = jsHelper.getProperty("id").toInt32();

  switch(dataId) {
    case ZbxHelper::LOGIN :
      mzbxAuthToken = jsHelper.getProperty("result").toString();
      if(! mzbxAuthToken.isEmpty())
        misLogged = true;
      resetStatData();
      postRpcDataRequest();
      break;
    case ZbxHelper::TRIGGER: {
        QScriptValueIterator trigger(jsHelper.getProperty("result"));
        while (trigger.hasNext()) {
            trigger.next();
            if (trigger.flags()&QScriptValue::SkipInEnumeration) {
                continue;
              }
            QScriptValue triggerData = trigger.value();
            MonitorBroker::CheckT check;
            QString triggerName = triggerData.property("description").toString();
            check.check_command = triggerName.toStdString();
            check.status = triggerData.property("value").toInt32();
            if(check.status != MonitorBroker::NAGIOS_OK) {
                check.alarm_msg = triggerData.property("error").toString().toStdString();
                int sev = triggerData.property("priority").toInteger();
                Criticity criticity(utils::getCriticity(mcoreData->monitor, sev));
                check.status = criticity.getValue();
              } else {
                check.alarm_msg = triggerName.toStdString(); //TODO: user another parameter?
              }
            QString targetHost = "";
            QScriptValueIterator host(triggerData.property("hosts"));
            if(host.hasNext()) {
                host.next();
                QScriptValue hostData = host.value();
                targetHost = hostData.property("host").toString();
                check.host = targetHost.toStdString();
              }
            QScriptValueIterator item(triggerData.property("items"));
            if(item.hasNext()) {
                item.next();
                QScriptValue itemData = item.value();
                check.last_state_change = itemData.property("lastclock").toString().toStdString();
              }
            QString key = ID_PATTERN.arg(targetHost).arg(triggerName);
            check.id = key.toStdString();
            mcoreData->checks_[key] = check;
          }
        if(mhostLeft--, mhostLeft == 0){
            foreach(const QString& checkId, mcoreData->cnodes.keys()) {

                NodeListT::iterator node = mcoreData->cnodes.find(checkId);
                if(node == mcoreData->cnodes.end())
                  continue;

                CheckListT::Iterator check = mcoreData->checks_.find(node->child_nodes);
                if(check == mcoreData->checks_.end())
                  continue;

                node->check = *check;
                updateCNode(node);
                mcoreData->check_status_count[check->status]++;
              }
            mupdateSucceed = true;
            updateStats();
          }
        break;
      }
    default :
      utils::alert(tr("Weird response received from the server"));
      exit(1);
      break;
    }
}

void SvNavigator::processRpcError(QNetworkReply::NetworkError _code){

  QString apiUrl = "";
  if(mcoreData->monitor == MonitorBroker::ZABBIX) {
      apiUrl = mzbxHelper->getApiUri();
    } else if(mcoreData->monitor == MonitorBroker::ZENOSS){
      apiUrl =  mznsHelper->getRequestUrl();
    }
  QString msg = SERVICE_OFFLINE_MSG.arg(apiUrl%tr(" (error code %1)").arg(_code));
  utils::alert(msg);
  updateStatusBar(msg);
  foreach(const QString& c, mcoreData->cnodes.keys()) {
      NodeListT::iterator node = mcoreData->cnodes.find(c);
      if(node == mcoreData->cnodes.end())
        continue;

      node->check.status = MonitorBroker::CRITICITY_UNKNOWN;
      node->check.host = "Unknown";
      node->check.last_state_change = "0";
      node->check.check_command = "Unknown" ;
      node->check.alarm_msg = msg.toStdString();
      updateCNode(node);
      mcoreData->check_status_count[node->criticity]++;
    }
  mupdateSucceed = false ;
  updateStats();
}


void SvNavigator::openRpcSession(void)
{

  QStringList authParams = getAuthInfo();
  QUrl znsUrlParams;
  if(authParams.size() != 2) {
      utils::alert("Invalid authentication chain!\nMust be in the form login:password");
      //TODO: fill console with unknown status?
      return ;
    }

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
      mznsHelper->setRouter(ZnsHelper::RETRIEVE_DEV);
      foreach(const QString& host, mcoreData->hosts.keys()) {
          mznsHelper->postRequest(ZnsHelper::RETRIEVE_DEV,
                                  ZnsHelper::ReQPatterns[ZnsHelper::RETRIEVE_DEV]
                                  .arg(host)
                                  .arg(ZnsHelper::RETRIEVE_DEV)
                                  .toAscii());
        }
      break;
    default:
      break;
    }
}

void SvNavigator::processZnsReply(QNetworkReply* _reply)
{
  _reply->deleteLater();
  if (_reply->error() != QNetworkReply::NoError) {
      return;
    }
  int code = _reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  QString data = _reply->readAll();
  if ((code >= 200 && code < 300) ||  //Success
      (code >= 300 && code < 400)) { // Redirection
      QVariant cookiesContainer = _reply->header(QNetworkRequest::SetCookieHeader);
      QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(cookiesContainer);
      qDebug() << data;
      if(mznsHelper->cookieJar()->setCookiesFromUrl(cookies, mznsHelper->getApiBaseUrl())){
          if(data.endsWith("submitted=true")) {
              misLogged = true;
              postRpcDataRequest();
            }
        }
    } else {
      utils::alert("Can't connect to the Zenoss API: " + data);
    }
}

QStringList SvNavigator::getAuthInfo(void) {

  QStringList authInfo = QStringList();
  int pos = mserverAuthChain.indexOf(":");
  if(pos == -1){
      return authInfo;
    }
  authInfo.push_back(mserverAuthChain.left(pos));
  authInfo.push_back(mserverAuthChain.mid(pos+1, -1));
  return authInfo;
}


void SvNavigator::addEvents(void)
{
  connect(this, SIGNAL(sortEventConsole()), mmsgPanel, SLOT(sortEventConsole()));
  connect(this, SIGNAL(hasToBeUpdate(QString)), this, SLOT(updateBpNode(QString)));
  connect(msubMenuList["Capture"], SIGNAL(triggered(bool)), mmap, SLOT(capture()));
  connect(msubMenuList["ZoomIn"], SIGNAL(triggered(bool)), mmap, SLOT(zoomIn()));
  connect(msubMenuList["ZoomOut"], SIGNAL(triggered(bool)), mmap, SLOT(zoomOut()));
  connect(msubMenuList["HideChart"], SIGNAL(triggered(bool)), this, SLOT(hideChart()));
  connect(msubMenuList["Refresh"], SIGNAL(triggered(bool)), this, SLOT(startMonitor()));
  connect(msubMenuList["ChangePassword"], SIGNAL(triggered(bool)), this, SLOT(handleChangePasswordAction(void)));
  connect(msubMenuList["ChangeMonitoringSettings"], SIGNAL(triggered(bool)), this, SLOT(handleChangeMonitoringSettingsAction(void)));
  connect(msubMenuList["ShowAbout"], SIGNAL(triggered(bool)), this, SLOT(handleShowAbout()));
  connect(msubMenuList["ShowOnlineResources"], SIGNAL(triggered(bool)), this, SLOT(handleShowOnlineResources()));
  connect(msubMenuList["Quit"], SIGNAL(triggered(bool)), qApp, SLOT(quit()));
  connect(mcontextMenuList["FilterNodeRelatedMessages"], SIGNAL(triggered(bool)), this, SLOT(filterNodeRelatedMsg()));
  connect(mcontextMenuList["CenterOnNode"], SIGNAL(triggered(bool)), this, SLOT(centerGraphOnNode()));
  connect(mprefWindow, SIGNAL(urlChanged(QString)), mbrowser, SLOT(setUrl(QString)));
  connect(mtopRightPanel, SIGNAL(currentChanged (int)), this, SLOT(tabChanged(int)));
  connect(mmap, SIGNAL(expandNode(QString, bool, qint32)), this, SLOT(expandNode(const QString &, const bool &, const qint32 &)));
  connect(mtree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(centerGraphOnNode(QTreeWidgetItem *)));
  connect(mzbxHelper, SIGNAL(finished(QNetworkReply*)), this, SLOT(processZbxReply(QNetworkReply*)));
  connect(mzbxHelper, SIGNAL(propagateError(QNetworkReply::NetworkError)), this, SLOT(processRpcError(QNetworkReply::NetworkError)));
  connect(mznsHelper, SIGNAL(finished(QNetworkReply*)), this, SLOT(processZnsReply(QNetworkReply*)));
  connect(mznsHelper, SIGNAL(propagateError(QNetworkReply::NetworkError)), this, SLOT(processRpcError(QNetworkReply::NetworkError)));
}
