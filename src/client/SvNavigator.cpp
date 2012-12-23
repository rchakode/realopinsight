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
#include "Utils.hpp"
#include "client/JsHelper.hpp"
#include <sstream>
#include <QStatusBar>
#include <QObject>
#include <zmq.h>
#include<iostream>


const QString DEFAULT_TIP_PATTERN(QObject::tr("Service: %1\nDescription: %2\nStatus: %3\n   Calc. Rule: %4\n   Prop. Rule: %5"));
const QString ALARM_SPECIFIC_TIP_PATTERN(QObject::tr("\nTarget Host: %6\nCheck/Trigger ID: %7\nCheck Output: %8\nMore info: %9"));
const QString SERVICE_OFFLINE_MSG(QObject::tr("ERROR: Failed to connect to %1"));
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

SvNavigator::SvNavigator(const qint32 & _userRole,
                         const QString & _config,
                         QWidget* parent)
    : QMainWindow(parent),
      coreData (new Struct()),
      configFile(_config),
      userRole (_userRole),
      settings (new Settings()),
      statsPanel (new Stats()),
      filteredMsgPanel (NULL),
      mainSplitter (new QSplitter(this)),
      rightSplitter (new QSplitter()),
      topRightPanel (new QTabWidget()),
      bottomRightPanel (new QTabWidget()),
      browser (new WebKit()),
      map (new GraphView()),
      tree (new SvNavigatorTree()),
      monPrefWindow (new Preferences(_userRole, Preferences::ChangeMonitoringSettings)),
      changePasswdWindow (new Preferences(_userRole, Preferences::ChangePassword)),
      msgPanel(new MsgPanel()),
      nodeContextMenu (new QMenu()),
      zxHelper(new ZbxHelper()),
      zbxAuthToken(""),
      hLeft(0),
      znsHelper(new ZnsHelper()),
      isLoggedOnZenoss(false)
{
    setWindowTitle(tr("%1 Operations Console").arg(appName));
    loadMenus();
    topRightPanel->addTab(map, tr("Dashboard"));
    topRightPanel->addTab(browser, tr("Native Web UI"));
    bottomRightPanel->addTab(msgPanel, tr("Event Console"));
    mainSplitter->addWidget(tree);
    mainSplitter->addWidget(rightSplitter);
    rightSplitter->addWidget(topRightPanel);
    rightSplitter->addWidget(bottomRightPanel);
    rightSplitter->setOrientation(Qt::Vertical);
    setCentralWidget(mainSplitter);
    updateMonitoringSettings();
    addEvents();
}

SvNavigator::~SvNavigator()
{
    delete msgPanel;
    delete statsPanel;
    delete tree;
    delete browser;
    delete map;
    delete coreData;
    delete topRightPanel;
    delete bottomRightPanel;
    delete rightSplitter;
    delete mainSplitter;
    delete monPrefWindow;
    delete changePasswdWindow;
    delete zxHelper;
    delete znsHelper;
    if(filteredMsgPanel) delete filteredMsgPanel;
    unloadMenus();
}

void SvNavigator::closeEvent(QCloseEvent * event)
{
    if(filteredMsgPanel)
        filteredMsgPanel->close();
    QMainWindow::closeEvent(event);
}


void SvNavigator::contextMenuEvent(QContextMenuEvent * event)
{
    QPoint pos = event->globalPos();
    QList<QTreeWidgetItem*> treeNodes = tree->selectedItems();
    QGraphicsItem* graphNode = map->nodeAtGlobalPos(pos);

    if (treeNodes.length() || graphNode) {
        if(graphNode) {
            QString itemId = graphNode->data(0).toString();
            selectedNodeId =  itemId.left(itemId.indexOf(":"));
        }  else {
            selectedNodeId = treeNodes[0]->data(0, QTreeWidgetItem::UserType).toString();
        }

        nodeContextMenu->exec(pos);
    }
}

void SvNavigator::startMonitor()
{
    updateStatusBar(tr("Connecting to the server..."));
    updateSucceed = true ;
    setEnabled(false);
    switch(coreData->monitor){
    case MonitorBroker::ZENOSS:
        isLoggedOnZenoss ? requestZenossEvents() : openZenossSession() ;
        break;

    case MonitorBroker::ZABBIX:
        zbxAuthToken.isEmpty()? openZabbixSession() : requestZabbixTriggers();
        break;
    case MonitorBroker::NAGIOS:
    default :
        runNagiosMonitor();
        break;
    }
    setEnabled(true);
    updateInterval = settings->value(Preferences::UPDATE_INTERVAL_KEY).toInt();
    updateInterval = 1000 * ((updateInterval > 0)? updateInterval :
                                                   MonitorBroker::DEFAULT_UPDATE_INTERVAL);
    timerId = startTimer(updateInterval);
}

void SvNavigator::timerEvent(QTimerEvent *)
{
    startMonitor();
}

void  SvNavigator::updateStatusBar(const QString & msg) {
    statusBar()->showMessage(msg);
}


void SvNavigator::load(const QString & _file)
{
    if( ! _file.isEmpty()) {
        configFile = utils::getAbsolutePath(_file);
    }
    openedFile = configFile;
    Parser parser;
    parser.parseSvConfig(configFile, *coreData);
    tree->clear();
    tree->addTopLevelItem(coreData->tree_items[SvNavigatorTree::rootID]);
    map->load(parser.getDotGraphFile(), coreData->bpnodes, coreData->cnodes);
    browser->setUrl(monitorBaseUrl);
    resize();
    show();
    map->scaleToFitViewPort();
    setWindowTitle(tr("%1 Operations Console - %2").arg(appName).arg(configFile));
}

void SvNavigator::unloadMenus(void)
{
    subMenuList.clear();
    menuList.clear();
    delete nodeContextMenu;
}

void SvNavigator::handleChangePasswordAction(void)
{
    changePasswdWindow->exec();
}

void SvNavigator::handleChangeMonitoringSettingsAction(void)
{
    monPrefWindow->exec();
    updateMonitoringSettings();
    killTimer(timerId);
    timerId = startTimer(updateInterval);
    zbxAuthToken.clear();
    startMonitor();
}

void SvNavigator::handleShowOnlineResources(void)
{
    QDesktopServices appLauncher;
    appLauncher.openUrl(QUrl("http://RealOpInsight.com/"));
}

void SvNavigator::handleShowAbout(void)
{
    Preferences about(userRole, Preferences::ShowAbout);
    about.exec();
}

int SvNavigator::runNagiosMonitor(void)
{
    msocket = new Socket(ZMQ_REQ);
    msocket->connect(serverUrl.toStdString());
    msocket->makeHandShake();

    if(! msocket->isConnected2Server()) {
        QString msg = SERVICE_OFFLINE_MSG.arg(serverUrl);
        utils::alert(msg);
        updateStatusBar(msg);
        updateSucceed = false ;
    } else {
        if(msocket->getServerSerial() < 110) {
            utils::alert(tr("ERROR: The server %1 is not supported")
                         .arg(msocket->getServerSerial()));
            updateSucceed = false;
        }
        updateStatusBar(tr("Updating..."));
    }
    resetStatData();
    msgPanel->setSortingEnabled(false);

    foreach(const QString & checkId, coreData->cnodes.keys()) {

        NodeListT::iterator cnode = coreData->cnodes.find(checkId);
        if(cnode == coreData->cnodes.end()) {
            continue;
        }

        if(cnode->child_nodes == "") {
            cnode->criticity = MonitorBroker::CRITICITY_UNKNOWN;
            coreData->check_status_count[cnode->criticity]++;
            continue;
        }

        QStringList childNodes = cnode->child_nodes.split(Parser::CHILD_NODES_SEP);
        foreach(const QString & nodeId, childNodes) {
            QString msg = serverAuthChain%":"%nodeId;
            if(updateSucceed) {
                msocket->send(msg.toStdString());
                msg = QString::fromStdString(msocket->recv());
            } else {
                msg = DEFAULT_ERROR_MSG.arg(serverUrl);
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
            coreData->check_status_count[cnode->criticity]++;
        }
    }
    updateStats();
    msocket->disconnect();
    return 0;
}

void SvNavigator::resetStatData(void)
{
    coreData->check_status_count[MonitorBroker::CRITICITY_NORMAL] = 0;
    coreData->check_status_count[MonitorBroker::CRITICITY_MINOR] = 0;
    coreData->check_status_count[MonitorBroker::CRITICITY_MAJOR] = 0;
    coreData->check_status_count[MonitorBroker::CRITICITY_HIGH] = 0;
    coreData->check_status_count[MonitorBroker::CRITICITY_UNKNOWN] = 0;
    hLeft = coreData->hosts.size();
    iter = 0;
}


QString SvNavigator::getNodeToolTip(const NodeT & _node)
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


void SvNavigator::updateCNode(NodeListT::iterator & _node) {

    _node->criticity =
            _node->prop_status =
            utils::getCriticity(coreData->monitor, _node->check.status);
    updateAlarmMsg(_node);
    QString toolTip = getNodeToolTip(*_node);
    updateNavTreeItemStatus(_node, toolTip);
    map->updateNode(_node, toolTip);
    msgPanel->addMsg(_node);

    emit hasToBeUpdate(_node->parent);
}

void SvNavigator::updateStats() {

    qint64 nbChecks = coreData->cnodes.size();
    if(nbChecks == 0) return;
    Stats *stats = new Stats;
    QString info = stats->update(coreData->check_status_count, nbChecks);
    stats->setToolTip(info);
    map->updateStatsPanel(stats);
    if(statsPanel) delete statsPanel;
    statsPanel = stats;
    msgPanel->sortItems(MsgPanel::msgPanelColumnCount - 1, Qt::DescendingOrder);
    msgPanel->resizeFields(msgPanelSize);
    if(updateSucceed) updateStatusBar("Update completed");
}

void SvNavigator::updateAlarmMsg(NodeListT::iterator &  _node)
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

        if(_node->criticity == MonitorBroker::NAGIOS_WARNING)
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
    NodeListT::iterator node = coreData->bpnodes.find(_nodeId);
    if (node == coreData->bpnodes.end()) {
        node = coreData->cnodes.find(_nodeId);
        if (node == coreData->cnodes.end()) {
            return ;
        }
    }

    Criticity criticity;
    QStringList nodeIds = node->child_nodes.split(Parser::CHILD_NODES_SEP);
    for(QStringList::const_iterator it = nodeIds.begin(); it != nodeIds.end(); it++) {

        NodeListT::iterator child = coreData->cnodes.find(*it);
        if (child == coreData->cnodes.end()) {
            child = coreData->bpnodes.find(*it);
            if (child == coreData->bpnodes.end()) {
                continue;
            }
        }

        Criticity cst(static_cast<MonitorBroker::CriticityT>(child->prop_status));
        if(node->status_crule == StatusCalcRules::WeightedCriticity) {
            criticity = criticity / cst;
        } else {
            criticity = criticity *  cst;
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
    map->updateNode(node, toolTip);
    updateNavTreeItemStatus(node, toolTip);
    emit hasToBeUpdate(node->parent);
}


void SvNavigator::updateNavTreeItemStatus(const NodeListT::iterator & _node, const QString & _tip)
{

    TreeNodeItemListT::iterator tnode_it = coreData->tree_items.find(_node->id);
    if(tnode_it != coreData->tree_items.end()) {
        (*tnode_it)->setIcon(0, utils::getTreeIcon(_node->criticity));
        (*tnode_it)->setToolTip(0, _tip);
    }
}

void SvNavigator::updateMonitoringSettings(){
    monitorBaseUrl = settings->value(Preferences::URL_KEY).toString();
    serverAuthChain = settings->value(Preferences::SERVER_PASS_KEY).toString();
    serverAddr = settings->value(Preferences::SERVER_ADDR_KEY).toString();
    serverPort = settings->value(Preferences::SERVER_PORT_KEY).toString();
    serverUrl = QString("tcp://%1:%2").arg(serverAddr).arg(serverPort);
    updateInterval = settings->value(Preferences::UPDATE_INTERVAL_KEY).toInt() * 1000;
    if (updateInterval <= 0) updateInterval = MonitorBroker::DEFAULT_UPDATE_INTERVAL * 1000;
}

void SvNavigator::expandNode(const QString & _nodeId,
                             const bool & _expand,
                             const qint32 & _level)
{
    NodeListT::iterator node = coreData->bpnodes.find(_nodeId);
    if(node == coreData->bpnodes.end()) {
        return ;
    }

    if(node->child_nodes != "") {
        QStringList  childNodes = node->child_nodes.split(Parser::CHILD_NODES_SEP);
        for (QStringList::iterator udsIt = childNodes.begin(); udsIt != childNodes.end(); udsIt++) {
            map->setNodeVisible(*udsIt, _nodeId, _expand, _level);
        }
    }
}

void SvNavigator::centerGraphOnNode(const QString & _node_id)
{
    if(_node_id != "") selectedNodeId =  _node_id;
    map->centerOnNode(selectedNodeId);
}

void SvNavigator::filterNodeRelatedMsg(void)
{
    bool process = true;
    if(filteredMsgPanel) {
        delete filteredMsgPanel;
    }
    filteredMsgPanel = new MsgPanel();
    NodeListT::iterator node = coreData->bpnodes.find(selectedNodeId);

    if(node == coreData->bpnodes.end()) {
        node = coreData->cnodes.find(selectedNodeId);
        if(node == coreData->bpnodes.end()) {
            process = false;
        }
    }

    if(process) {
        filterNodeRelatedMsg(selectedNodeId);
        QString title = tr("Messages related to '%2' - %1")
                .arg(appName)
                .arg(node->name);
        filteredMsgPanel->resizeFields(msgPanelSize, true);
        filteredMsgPanel->setWindowTitle(title);
    }

    filteredMsgPanel->show();
}

void SvNavigator::filterNodeRelatedMsg(const QString & _nodeId)
{
    bool quick = false;
    NodeListT::iterator nodeIt = coreData->bpnodes.find(_nodeId);
    if(nodeIt == coreData->bpnodes.end() || nodeIt->child_nodes == "") {
        if(nodeIt == coreData->bpnodes.end()) {
            nodeIt = coreData->cnodes.find(_nodeId);
            if(nodeIt == coreData->cnodes.end()) {
                quick = true;
            }
        } else {
            quick = true;
        }
    }
    if(quick) {
        return;
    }

    if (nodeIt->type == NodeType::ALARM_NODE) {
        filteredMsgPanel->addMsg(nodeIt);
    } else {
        QStringList childIds = nodeIt->child_nodes.split(Parser::CHILD_NODES_SEP);
        foreach(QString checkId, childIds) {
            filterNodeRelatedMsg(checkId);
        }
    }
}


void SvNavigator::acknowledge(void)
{
    //TODO
}

void SvNavigator::tabChanged(int _tab_index)
{
    if (_tab_index != 0) {
        subMenuList["ZoomIn"]->setEnabled(false);
        subMenuList["ZoomOut"]->setEnabled(false);
        return;
    }

    subMenuList["ZoomIn"]->setEnabled(true);
    subMenuList["ZoomOut"]->setEnabled(true);
}

void SvNavigator::hideChart(void)
{
    if (map->hideChart()) {
        subMenuList["HideChart"]->setIcon(QIcon(":images/check.png"));
        return;
    }
    subMenuList["HideChart"]->setIcon(QIcon(""));
}

void SvNavigator::centerGraphOnNode(QTreeWidgetItem * _item)
{
    centerGraphOnNode(_item->data(0, QTreeWidgetItem::UserType).toString());
}

void SvNavigator::resize(void)
{
    const qreal GRAPH_HEIGHT_RATE = 0.50;
    QSize screenSize = qApp->desktop()->screen(0)->size();
    msgPanelSize = QSize(screenSize.width() * 0.80, screenSize.height() * (1.0 - GRAPH_HEIGHT_RATE));

    QList<qint32> framesSize;
    framesSize.push_back(screenSize.width() * 0.20);
    framesSize.push_back(msgPanelSize.width());
    mainSplitter->setSizes(framesSize);

    framesSize[0] = (screenSize.height() * GRAPH_HEIGHT_RATE);
    framesSize[1] = (msgPanelSize.height());
    rightSplitter->setSizes(framesSize);

    mainSplitter->resize(screenSize.width(), screenSize.height() * 0.85);
    QMainWindow::resize(screenSize.width(),  screenSize.height());
}


void SvNavigator::loadMenus(void)
{
    QIcon refreshIcon(":images/built-in/refresh.png");
    QIcon cameraIcon(":images/built-in/camera.png");
    QIcon zoominIcon(":images/built-in/zoomin.png");
    QIcon zoomoutIcon(":images/built-in/zoomout.png");

    QMenuBar* menuBar = new QMenuBar();
    menuList["MENU1"] = menuBar->addMenu("&File"),
            subMenuList["Refresh"] = menuList["MENU1"]->addAction(refreshIcon, tr("&Refresh Screen")),
            subMenuList["Capture"] = menuList["MENU1"]->addAction(cameraIcon, tr("&Save Map as Image")),
            menuList["MENU1"]->addSeparator(),
            subMenuList["Quit"] = menuList["MENU1"]->addAction(tr("&Quit"));

    menuList["MENU2"] = menuBar->addMenu("&Map"),
            subMenuList["ZoomIn"] = menuList["MENU2"]->addAction(zoominIcon, tr("Zoom &In")),
            subMenuList["ZoomOut"] = menuList["MENU2"]->addAction(zoomoutIcon, tr("Zoom &Out")),
            subMenuList["HideChart"] = menuList["MENU2"]->addAction(tr("Hide &Chart"));

    menuList["MENU3"] = menuBar->addMenu("&Preferences"),
            subMenuList["ChangePassword"] = menuList["MENU3"]->addAction(tr("Change &Password")),
            subMenuList["ChangeMonitoringSettings"] = menuList["MENU3"]->addAction(tr("&Monitoring Settings"));

    menuList["MENU4"] = menuBar->addMenu("&Help"),
            subMenuList["ShowOnlineResources"] = menuList["MENU4"]->addAction(tr("Online &Resources")),
            menuList["MENU4"]->addSeparator(),
            subMenuList["ShowAbout"] = menuList["MENU4"]->addAction(tr("&About %1").arg(appName));

    subMenuList["Capture"]->setShortcut(QKeySequence::Save);
    subMenuList["Refresh"]->setShortcut(QKeySequence::Refresh);
    subMenuList["ZoomIn"]->setShortcut(QKeySequence::ZoomIn);
    subMenuList["ZoomOut"]->setShortcut(QKeySequence::ZoomOut);
    subMenuList["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
    subMenuList["Quit"]->setShortcut(QKeySequence::Quit);

    contextMenuList["FilterNodeRelatedMessages"] = nodeContextMenu->addAction(tr("&Filter related messages"));
    contextMenuList["CenterOnNode"] = nodeContextMenu->addAction(tr("Center Graph &On"));
    contextMenuList["Cancel"] = nodeContextMenu->addAction(tr("&Cancel"));

    QToolBar* toolBar = addToolBar(QString(ngrt4n::APP_NAME.c_str()));
    toolBar->addAction(subMenuList["Refresh"]);
    toolBar->addAction(subMenuList["ZoomIn"]);
    toolBar->addAction(subMenuList["ZoomOut"]);
    toolBar->addAction(subMenuList["Capture"]);
    toolBar->setIconSize(QSize(16,16));
    setMenuBar(menuBar);
}


void SvNavigator::openZabbixSession(void)
{
    zxHelper->setBaseUrl(monitorBaseUrl);
    QStringList params = serverAuthChain.split(":");
    params.push_back(QString::number(ZbxHelper::LOGIN));
    zxHelper->get(ZbxHelper::LOGIN, params);
}

void SvNavigator::closeZabbixSession(void)
{
    QStringList params;
    params.push_back(zbxAuthToken);
    params.push_back(QString::number(ZbxHelper::LOGOUT));
    zxHelper->get(ZbxHelper::LOGOUT, params);
}

void SvNavigator::retrieveZabbixHostTriggers(const QString & host)
{
    QStringList params;
    params.push_back(zbxAuthToken);
    params.push_back(host);
    params.push_back(QString::number(ZbxHelper::TRIGGER));
    zxHelper->get(ZbxHelper::TRIGGER, params);
}

void SvNavigator::processZabbixReply(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    string data = static_cast<QString>(reply->readAll()).toStdString();
    JsonHelper jsHelper(data);
    qint32 dataId = jsHelper.getProperty("id").toInt32();

    switch(dataId) {
    case ZbxHelper::LOGIN :
        zbxAuthToken = jsHelper.getProperty("result").toString();
        resetStatData();
        requestZabbixTriggers();
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
                Criticity st(utils::getCriticity(coreData->monitor, sev));
                check.status = st.getValue();
            } else {
                check.alarm_msg = triggerName.toStdString(); //TODO
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
            coreData->checks_[key] = check;
        }
        if(hLeft--, hLeft == 0){

            foreach(const QString & checkId, coreData->cnodes.keys()) {

                NodeListT::iterator node = coreData->bpnodes.find(checkId);
                if(node == coreData->bpnodes.end()) {
                    continue;
                }
                CheckListT::Iterator check = coreData->checks_.find(node->child_nodes);
                if(check == coreData->checks_.end()) {
                    continue;
                }

                node->check = *check;
                updateCNode(node);
                coreData->check_status_count[check->status]++;
            }
            updateSucceed = true;
            updateStats();
        }
        break;
    }
    default :
        utils::alert(tr("ERROR: Weird response received from the server"));
        exit(1);
        break;
    }
}

void SvNavigator::requestZabbixTriggers(void) {
    updateStatusBar(tr("Updating..."));
    foreach(const QString & host, coreData->hosts.keys()) {
        retrieveZabbixHostTriggers(host);
    }
}

void SvNavigator::processPostError(QNetworkReply::NetworkError code){

    QString apiUrl = "";
    if(coreData->monitor == MonitorBroker::ZABBIX) {
        apiUrl = zxHelper->getApiUri();
    } else if(coreData->monitor == MonitorBroker::ZENOSS){
        apiUrl =  znsHelper->getRequestUrl();
    }
    QString msg = SERVICE_OFFLINE_MSG.arg(apiUrl%tr(" (error code %1)").arg(code));
    utils::alert(msg);
    updateStatusBar(msg);
    foreach(const QString & c, coreData->cnodes.keys()) {
        NodeListT::iterator node = coreData->bpnodes.find(c);
        if(node == coreData->bpnodes.end()) continue;
        node->check.status = MonitorBroker::CRITICITY_UNKNOWN;
        node->check.host = "Unknown";
        node->check.last_state_change = "0";
        node->check.check_command = "Unknown" ;
        node->check.alarm_msg = msg.toStdString();
        updateCNode(node);
        coreData->check_status_count[node->check.status]++;
    }
    updateSucceed = false ;
    updateStats();
}


void SvNavigator::openZenossSession(void)
{
    QUrl urlParams;
    QStringList authInfo = serverAuthChain.split(":");
    znsHelper->setBaseUrl(monitorBaseUrl);  //Must be set before setting the came_from parameter
    urlParams.addQueryItem("__ac_name", authInfo[0]);
    urlParams.addQueryItem("__ac_password", authInfo[1]);
    urlParams.addQueryItem("submitted", "true");
    urlParams.addQueryItem("came_from", znsHelper->getApiContextUrl());
    znsHelper->performPostRequest(ZnsHelper::LOGIN_REQUEST, urlParams.encodedQuery());
}

void SvNavigator::requestZenossEvents(void) {
    updateStatusBar(tr("Updating..."));
    foreach(const QString & host, coreData->hosts.keys()) {
        //TODO
    }
}

void SvNavigator::processZenossReply(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString data = reply->readAll();
    if ((code >= 200 && code < 300) ||  //Success
            (code >= 300 && code < 400)) { // Redirection
        QVariant cookiesContainer = reply->header(QNetworkRequest::SetCookieHeader);
        QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(cookiesContainer);
        qDebug() << data;
        if(znsHelper->cookieJar()->setCookiesFromUrl(cookies, znsHelper->getApiBaseUrl())){
            if(data.endsWith("submitted=true")) {
                isLoggedOnZenoss = true;
                QString jsonReq = "{\"action\": \"EventsRouter\", \
                        \"method\": \"query\", \
                        \"data\": [{ \
                        \"limit\": 100, \
                        \"sort\": \"severity\", \
                        \"dir\": \"DESC\", \
                        \"start\": 0, \
                        \"params\": { \
                        \"device\": \"tchieudjie\", \
                        \"eventState\": [0,1], \
                        \"severity\": [5,4,3,2,1,0]} }], \
                        \"type\": \"rpc\", \
                        \"tid\": 1}";
                QUrl eventsRouterUrl = QUrl(znsHelper->getApiContextUrl().toAscii()+"/evconsole_router");
                znsHelper->setRequestUrl(eventsRouterUrl);
                znsHelper->performPostRequest(ZnsHelper::EVENT_REQUEST, jsonReq.toAscii());
            }
        }
    } else {
        utils::alert("Can't connect to the Zenoss API: " + data);
    }
}


void SvNavigator::addEvents(void)
{
    connect(this, SIGNAL(sortEventConsole()), msgPanel, SLOT(sortEventConsole()));
    connect(this, SIGNAL(hasToBeUpdate(QString)), this, SLOT(updateBpNode(QString)));
    connect(subMenuList["Capture"], SIGNAL(triggered(bool)), map, SLOT(capture()));
    connect(subMenuList["ZoomIn"], SIGNAL(triggered(bool)), map, SLOT(zoomIn()));
    connect(subMenuList["ZoomOut"], SIGNAL(triggered(bool)), map, SLOT(zoomOut()));
    connect(subMenuList["HideChart"], SIGNAL(triggered(bool)), this, SLOT(hideChart()));
    connect(subMenuList["Refresh"], SIGNAL(triggered(bool)), this, SLOT(startMonitor()));
    connect(subMenuList["ChangePassword"], SIGNAL(triggered(bool)), this, SLOT(handleChangePasswordAction(void)));
    connect(subMenuList["ChangeMonitoringSettings"], SIGNAL(triggered(bool)), this, SLOT(handleChangeMonitoringSettingsAction(void)));
    connect(subMenuList["ShowAbout"], SIGNAL(triggered(bool)), this, SLOT(handleShowAbout()));
    connect(subMenuList["ShowOnlineResources"], SIGNAL(triggered(bool)), this, SLOT(handleShowOnlineResources()));
    connect(subMenuList["Quit"], SIGNAL(triggered(bool)), qApp, SLOT(quit()));
    connect(contextMenuList["FilterNodeRelatedMessages"], SIGNAL(triggered(bool)), this, SLOT(filterNodeRelatedMsg()));
    connect(contextMenuList["CenterOnNode"], SIGNAL(triggered(bool)), this, SLOT(centerGraphOnNode()));
    connect(monPrefWindow, SIGNAL(urlChanged(QString)), browser, SLOT(setUrl(QString)));
    connect(topRightPanel, SIGNAL(currentChanged (int)), this, SLOT(tabChanged(int)));
    connect(map, SIGNAL(expandNode(QString, bool, qint32)), this, SLOT(expandNode(const QString &, const bool &, const qint32 &)));
    connect(tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(centerGraphOnNode(QTreeWidgetItem *)));
    connect(zxHelper, SIGNAL(finished(QNetworkReply*)), this, SLOT(processZabbixReply(QNetworkReply*)));
    connect(zxHelper, SIGNAL(propagateError(QNetworkReply::NetworkError)), this, SLOT(processPostError(QNetworkReply::NetworkError)));
    connect(znsHelper, SIGNAL(finished(QNetworkReply*)), this, SLOT(processZenossReply(QNetworkReply*)));
    connect(znsHelper, SIGNAL(propagateError(QNetworkReply::NetworkError)), this, SLOT(processPostError(QNetworkReply::NetworkError)));
}
