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
#include "client/JsonHelper.hpp"
#include <sstream>
#include <QScriptValueIterator>


const QString DefaultTip = "Service: %1 \
        \nDescription: %2 \
        \nStatus: %3 \
        \nStatus Calc. Rule: %4 \
        \nStatus Prop. Rule: %5" ;

        const QString AlarmSpecificTip = "Host: %6 \
        \nCheck/Trigger: %7 \
        \nMessage: %8 \
        \nCheck Output: %9";
        const QString ServiceOfficeMsg = "ERROR: failed to connect to %1";
const QString JsonError = "{\"return_code\": \"-1\", \"message\": \""+ServiceOfficeMsg+"\"}";

ComboBoxItemsT SvNavigator::propRules() {
    ComboBoxItemsT map;
    map.insert(StatusPropRules::label(StatusPropRules::Unchanged),StatusPropRules::toString(StatusPropRules::Unchanged));
    map.insert(StatusPropRules::label(StatusPropRules::Decreased),StatusPropRules::toString(StatusPropRules::Decreased));
    map.insert(StatusPropRules::label(StatusPropRules::Increased),StatusPropRules::toString(StatusPropRules::Increased));
    return map;
}

ComboBoxItemsT SvNavigator::calcRules() {
    ComboBoxItemsT map;
    map.insert(StatusCalcRules::label(StatusCalcRules::HighCriticity),StatusCalcRules::toString(StatusCalcRules::HighCriticity));
    map.insert(StatusCalcRules::label(StatusCalcRules::WeightedCriticity),StatusCalcRules::toString(StatusCalcRules::WeightedCriticity));
    return map;
}

SvNavigator::SvNavigator(const qint32 & _userRole,
                         const QString & _config,
                         const qint8 & _monitorType,
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
      graphView (new GraphView()),
      navigationTree (new SvNavigatorTree()),
      monPrefWindow (new Preferences(_userRole, Preferences::ChangeMonitoringSettings)),
      changePasswdWindow (new Preferences(_userRole, Preferences::ChangePassword)),
      msgPanel(new MsgPanel()),
      nodeContextMenu (new QMenu()),
      zabbixHelper(new ZabbixHelper()),
      monitorType(_monitorType)
{
    setWindowTitle(configFile + " | " + appName.toUpper() + " Operations Console");
    loadMenus();

    topRightPanel->addTab(graphView, "Dashboard");
    topRightPanel->addTab(browser, "Native Web UI");
    bottomRightPanel->addTab(msgPanel, "Event Console");
    mainSplitter->addWidget(navigationTree);
    mainSplitter->addWidget(rightSplitter);
    rightSplitter->addWidget(topRightPanel);
    rightSplitter->addWidget(bottomRightPanel);
    rightSplitter->setOrientation(Qt::Vertical);
    setCentralWidget(mainSplitter);

    updateMonitoringSettings();
    addEvents();
}


void SvNavigator::addEvents(void)
{
    connect(this, SIGNAL(sortEventConsole()), msgPanel, SLOT(sortEventConsole()));
    connect(this, SIGNAL(hasToBeUpdate(QString)), this, SLOT(updateNodeStatus(QString)));
    connect(subMenuList["Capture"], SIGNAL(triggered(bool)), graphView, SLOT(capture()));
    connect(subMenuList["ZoomIn"], SIGNAL(triggered(bool)), graphView, SLOT(zoomIn()));
    connect(subMenuList["ZoomOut"], SIGNAL(triggered(bool)), graphView, SLOT(zoomOut()));
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
    connect(graphView, SIGNAL(expandNode(QString, bool, qint32)), this, SLOT(expandNode(const QString &, const bool &, const qint32 &)));
    connect(navigationTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(centerGraphOnNode(QTreeWidgetItem *)));
    connect(zabbixHelper, SIGNAL(finished(QNetworkReply*)), this, SLOT(processZabbixReply(QNetworkReply*)));
    connect(zabbixHelper, SIGNAL(propagateError(QNetworkReply::NetworkError)), this, SLOT(processZabbixError(QNetworkReply::NetworkError)));
}

SvNavigator::~SvNavigator()
{
    delete msgPanel;
    delete statsPanel;
    delete navigationTree;
    delete browser;
    delete graphView;
    delete coreData;
    delete topRightPanel;
    delete bottomRightPanel;
    delete rightSplitter;
    delete mainSplitter;
    delete monPrefWindow;
    delete changePasswdWindow;
    delete zabbixHelper;
    if(filteredMsgPanel) delete filteredMsgPanel;
    unloadMenus();
}


void SvNavigator::alert(const QString & msg) {
    QMessageBox::warning(0, + "Warning | " + appName, msg, QMessageBox::Yes);
}

void SvNavigator::closeEvent(QCloseEvent * event)
{
    if(filteredMsgPanel) {
        filteredMsgPanel->close();
    }

    QMainWindow::closeEvent(event);
}


void SvNavigator::contextMenuEvent(QContextMenuEvent * event)
{
    QPoint global_pos;
    QList<QTreeWidgetItem*> tree_nodes;
    QGraphicsItem* graph_node;
    QString graph_item_id;

    global_pos = event->globalPos();
    tree_nodes = navigationTree->selectedItems();
    graph_node = graphView->nodeAtGlobalPos(global_pos);

    if (tree_nodes.length() || graph_node) {
        if(graph_node) {
            graph_item_id = graph_node->data(0).toString();
            selectedNodeId =  graph_item_id.left(graph_item_id.indexOf(":"));
        }  else {
            selectedNodeId = tree_nodes[0]->data(0, QTreeWidgetItem::UserType).toString();
        }

        nodeContextMenu->exec(global_pos);
    }
}

void SvNavigator::startMonitor()
{
    switch(monitorType){
    case MonitorBroker::ZABBIX : openZabbixSession();
        break ;
    case MonitorBroker::NAGIOS :
    default : runNagiosMonitor();
        break;
    }
    updateInterval = settings->value(Preferences::UPDATE_INTERVAL_KEY).toInt() * 1000;
    updateInterval = 1000 * ((updateInterval > 0)? updateInterval : MonitorBroker::DEFAULT_UPDATE_INTERVAL);
    timerId = startTimer(updateInterval);
}

void SvNavigator::timerEvent(QTimerEvent *)
{
    startMonitor();
}

void SvNavigator::load(const QString & _file)
{
    if( ! _file.isEmpty()) {
        configFile = _file ;
    }
    openedFile = configFile;
    Parser parser;
    parser.parseSvConfig(configFile, *coreData);
    navigationTree->clear();
    navigationTree->addTopLevelItem(coreData->tree_items[SvNavigatorTree::rootID]);
    graphView->load(parser.getDotGraphFile(), coreData->nodes);
    browser->setUrl(monitorBaseUrl);
    resize(); show();
    graphView->scaleToFitViewPort();
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
}

void SvNavigator::handleShowOnlineResources(void)
{
    QDesktopServices app_launcher;
    app_launcher.openUrl(QUrl("http://ngrt4n.com/docs/"));
}

void SvNavigator::handleShowAbout(void)
{
    Preferences about(userRole, Preferences::ShowAbout);
    about.exec();
}

int SvNavigator::runNagiosMonitor(void)
{
    zmq::context_t comContext(1);
    comChannel = ZmqHelper::initCliChannel(comContext, serverUrl);

    if(! comChannel) {
        QString msg = ServiceOfficeMsg.arg("the Daemon Module on "+QString::fromStdString(serverUrl));
        alert(msg);
    }
    qint32 ok_count = 0;
    qint32 warning_count = 0;
    qint32 critical_count = 0;
    qint32 unknown_count = 0;
    coreData->check_status_count.clear();
    msgPanel->setSortingEnabled(false);

    foreach(const QString & checkId, coreData->checks) {
        NodeListT::iterator node_it = coreData->nodes.find(checkId.trimmed());
        if(node_it == coreData->nodes.end()) continue;

        if(node_it->child_nodes == "") {
            node_it->status = MonitorBroker::UNKNOWN;
            unknown_count += 1;
            continue;
        }

        QStringList childNodes = node_it->child_nodes.split(Parser::CHILD_NODES_SEP);
        foreach(const QString & nodeId, childNodes) {
            MonitorBroker::NagiosCheckT check;

            string msg = serverAuthChain + ":"+nodeId.trimmed().toStdString(); //TODO
            if(comChannel) {
                ZmqHelper::sendFromSocket(*comChannel, msg);
                msg = ZmqHelper::recvFromSocket(*comChannel);
            } else {
                msg = JsonError.arg(QString::fromStdString((serverUrl))).toStdString();
            }

            JsonHelper jsHelper(msg);
            if(jsHelper.getProperty("return_code").toInt32() == 0 && comChannel) {
                check.status = jsHelper.getProperty("status").toInt32();
                check.host = jsHelper.getProperty("host").toString().toStdString();
                check.last_state_change = jsHelper.getProperty("lastchange").toString().toStdString();
                node_it->check.check_command = jsHelper.getProperty("command").toString().toStdString() ;
                check.alarm_msg = jsHelper.getProperty("message").toString().toStdString() ;
            } else {
                check.status = MonitorBroker::UNKNOWN;
                check.host = "Unknown";
                check.last_state_change = "0";
                node_it->check.check_command = "Unknown"  ;
                check.alarm_msg = jsHelper.getProperty("message").toString().toStdString() ;
            }

            switch(check.status) {
            case MonitorBroker::OK:
                ok_count += 1;
                break;

            case MonitorBroker::WARNING:
                warning_count += 1;
                break;

            case MonitorBroker::CRITICAL:
                critical_count += 1;
                break;

            default:
                unknown_count += 1;
                break;
            }

            if (node_it->status != check.status) {

                node_it->check = check;
                node_it->status = check.status;
                node_it->prop_status = check.status;
                updateAlarmMsg(node_it);
                QString toolTip = getNodeToolTip(*node_it);

                updateNavTreeItemStatus(node_it, toolTip);
                graphView->updateNode(node_it, toolTip);
                msgPanel->addMsg(node_it);
                emit hasToBeUpdate(node_it->parent);
            }
        }
    }

    coreData->check_status_count[MonitorBroker::OK] = ok_count;
    coreData->check_status_count[MonitorBroker::WARNING] = warning_count;
    coreData->check_status_count[MonitorBroker::CRITICAL] = critical_count;
    coreData->check_status_count[MonitorBroker::UNKNOWN] = unknown_count;
    updateStats();

    ZmqHelper::endCliChannel(comChannel);
    return 0;
}

int SvNavigator::runZabbixMonitor(void)
{
    qint32 ok_count = 0;
    qint32 warning_count = 0;
    qint32 critical_count = 0;
    qint32 unknown_count = 0;

    foreach(const QString & host, coreData->host_checks.keys()) {
        retrieveZabbixTriggers(host);
    }

    coreData->check_status_count[MonitorBroker::OK] = ok_count;
    coreData->check_status_count[MonitorBroker::WARNING] = warning_count;
    coreData->check_status_count[MonitorBroker::CRITICAL] = critical_count;
    coreData->check_status_count[MonitorBroker::UNKNOWN] = unknown_count;
    updateStats();
    return 0;
}

QString SvNavigator::getNodeToolTip(const NodeT & _node)
{
    QString toolTip = DefaultTip.arg(_node.name)
            .arg(const_cast<QString&>(_node.description).replace("\n", " "))
            .arg(Utils::statusToString(_node.status))
            .arg(StatusCalcRules::label(_node.status_crule))
            .arg(StatusPropRules::label(_node.status_prule));
    if (_node.type == NodeType::ALARM_NODE) {
        QString msg = "" ;
        if(_node.status == MonitorBroker::OK) {
            msg = const_cast<QString&>(_node.notification_msg).replace("\n", " ");
        } else {
            msg = QString(_node.check.alarm_msg.c_str()).replace("\n", " ");
        }
        toolTip += AlarmSpecificTip.arg(QString(_node.check.host.c_str()).replace("\n", " "))
                .arg(_node.child_nodes)
                .arg(msg)
                .arg(QString(_node.check.alarm_msg.c_str()).replace("\n", " ")) ;
    }

    return toolTip;
}

void SvNavigator::updateStats() {

    qint32 all_checks_count = coreData->checks.size();
    if(all_checks_count == 0) return ;
    Stats *stats = new Stats();
    QString info = stats->update(coreData->check_status_count, all_checks_count);
    stats->setToolTip(info);
    graphView->updateStatsPanel(stats);
    if(statsPanel) {delete statsPanel;}
    statsPanel = stats;
    msgPanel->sortItems(MsgPanel::msgPanelColumnCount - 1, Qt::DescendingOrder);
    msgPanel->resizeFields(msgPanelSize);
}

void SvNavigator::updateAlarmMsg(NodeListT::iterator &  _node)
{
    QRegExp regexp;
    QStringList splited_check_id;
    QStringList splited_check_command;
    QString  msg;
    qint32 len;

    splited_check_id = QString(_node->check.id.c_str()).split("/");
    len =  splited_check_id.length();

    if(_node->status == MonitorBroker::OK) {
        msg = _node->notification_msg;
    } else {
        msg = _node->alarm_msg;
    }

    if(len) {
        regexp.setPattern(MsgPanel::HOSTNAME_META_MSG_PATERN);
        msg.replace(regexp, splited_check_id[0]);

        if(len == 2) {
            regexp.setPattern(MsgPanel::SERVICE_META_MSG_PATERN);
            msg.replace(regexp, splited_check_id[1]);
        }
    }

    splited_check_command = QString(_node->check.check_command.c_str()).split("!");
    if(splited_check_command.length() >= 3) {
        regexp.setPattern(MsgPanel::THERESHOLD_META_MSG_PATERN);
        msg.replace(regexp, splited_check_command[1]);

        if(_node->status == MonitorBroker::WARNING) {
            msg.replace(regexp, splited_check_command[2]);
        }

    }

    if(_node->status == MonitorBroker::OK) {
        _node->notification_msg = msg ;
    } else {
        _node->alarm_msg = msg;
    }
}

void SvNavigator::updateNodeStatus(QString _node_id)
{
    qint32 normal_count = 0;
    qint32 warning_count = 0;
    qint32 unknown_count = 0;
    qint32 critical_count = 0;

    NodeListT::iterator node = coreData->nodes.find(_node_id);
    if (node == coreData->nodes.end()) return ;

    Status status;
    QStringList node_ids = node->child_nodes.split(Parser::CHILD_NODES_SEP);
    for(QStringList::const_iterator it = node_ids.begin(); it != node_ids.end(); it++) {

        NodeListT::iterator child = coreData->nodes.find(*it);
        if (child == coreData->nodes.end()) continue;

        Status cst(static_cast<MonitorBroker::StatusT>(child->prop_status));

        if(node->status_crule == StatusCalcRules::WeightedCriticity) {
            status = status / cst;
        } else {
            status = status *  cst;
        }

        switch(child->status) {

        case MonitorBroker::CRITICAL:
            critical_count ++;
            break;

        case MonitorBroker::WARNING:
            warning_count ++;
            break;

        case MonitorBroker::UNKNOWN:
            unknown_count ++;
            break;

        case MonitorBroker::OK:
            normal_count ++;
            break;

        default:
            break;
        }
    }

    node->status = status.getValue();

    switch(node->status_prule) {
    case StatusPropRules::Increased: node->prop_status = (status++).getValue();
        break;
    case StatusPropRules::Decreased: node->prop_status = (status--).getValue();
        break;
    default: node->prop_status = node->status;
        break;

    }
    QString toolTip = getNodeToolTip(*node);
    graphView->updateNode(node, toolTip);
    updateNavTreeItemStatus(node, toolTip);
    emit hasToBeUpdate(node->parent);
}


void SvNavigator::updateNavTreeItemStatus(const NodeListT::iterator & _node, const QString & _tip)
{
    QIcon icon;
    switch(_node->status) {
    case MonitorBroker::OK:
        icon.addFile(":/images/normal.png");
        break;

    case MonitorBroker::WARNING:
        icon.addFile(":/images/warning.png");
        break;

    case MonitorBroker::CRITICAL:
        icon.addFile(":/images/critical.png");
        break;

    default:
        icon.addFile(":/images/unknown.png");
        break;
    }

    TreeNodeItemListT::iterator tnode_it = coreData->tree_items.find(_node->id);
    if(tnode_it != coreData->tree_items.end()) {
        (*tnode_it)->setIcon(0, icon);
        (*tnode_it)->setToolTip(0, _tip);
    }
}

void SvNavigator::updateMonitoringSettings(){
    monitorBaseUrl = settings->value(Preferences::URL_KEY).toString();
    serverAuthChain = settings->value(Preferences::SERVER_PASS_KEY).toString().toStdString();
    serverAddr = settings->value(Preferences::SERVER_ADDR_KEY).toString();
    serverPort = settings->value(Preferences::SERVER_PORT_KEY).toString();
    serverUrl = QString("tcp://%1:%2").arg(serverAddr).arg(serverPort).toStdString();
    updateInterval = settings->value(Preferences::UPDATE_INTERVAL_KEY).toInt() * 1000;
    if (updateInterval <= 0) updateInterval = MonitorBroker::DEFAULT_UPDATE_INTERVAL * 1000;
}

void SvNavigator::expandNode(const QString & _node_id, const bool & _expand, const qint32 & _level)
{
    QStringList::iterator uds_it;
    NodeT& node = coreData->nodes[_node_id];

    if(node.type == NodeType::SERVICE_NODE && node.child_nodes != "") {
        QStringList  childNodes = node.child_nodes.split(Parser::CHILD_NODES_SEP);
        for (uds_it = childNodes.begin(); uds_it != childNodes.end(); uds_it++) {
            graphView->setNodeVisible(*uds_it, _node_id, _expand, _level);
        }
    }
}

void SvNavigator::centerGraphOnNode(const QString & _node_id)
{
    if(_node_id != "") selectedNodeId =  _node_id;
    graphView->centerOnNode(selectedNodeId);
}

void SvNavigator::filterNodeRelatedMsg(void)
{
    QString window_title;
    NodeListT::iterator node_it;

    if(filteredMsgPanel) delete filteredMsgPanel;
    filteredMsgPanel = new MsgPanel();
    node_it = coreData->nodes.find(selectedNodeId);

    if(node_it != coreData->nodes.end()) {
        filterNodeRelatedMsg(selectedNodeId);
        window_title = "Filtered messages from the component '"
                + coreData->nodes[selectedNodeId].name
                + "' - " + appName.toUpper();
        filteredMsgPanel->resizeFields(msgPanelSize, true);
        filteredMsgPanel->setWindowTitle(window_title);
    }

    filteredMsgPanel->show();
}

void SvNavigator::filterNodeRelatedMsg(const QString & _nodeId)
{
    NodeListT::iterator nodeIt = coreData->nodes.find(_nodeId);

    if(nodeIt == coreData->nodes.end() || nodeIt->child_nodes == "") {
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
    if (graphView->hideChart()) {
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
    QList<qint32> frames_size;
    const qreal GRAPH_HEIGHT_RATE = 0.50 ;

    QSize screen_size = qApp->desktop()->screen(0)->size();

    msgPanelSize = QSize(screen_size.width() * 0.80, screen_size.height() * (1.0 - GRAPH_HEIGHT_RATE));

    frames_size.push_back(screen_size.width() * 0.20);
    frames_size.push_back(msgPanelSize.width());
    mainSplitter->setSizes(frames_size);
    frames_size[0] = (screen_size.height() * GRAPH_HEIGHT_RATE),
            frames_size[1] = (msgPanelSize.height()),
            rightSplitter->setSizes(frames_size);

    mainSplitter->resize(screen_size.width(), screen_size.height() * 0.85);
    QMainWindow::resize(screen_size.width(),  screen_size.height());
}


void SvNavigator::loadMenus(void)
{
    QIcon refreshIcon(":images/built-in/refresh.png");
    QIcon cameraIcon(":images/built-in/camera.png");
    QIcon zoominIcon(":images/built-in/zoomin.png");
    QIcon zoomoutIcon(":images/built-in/zoomout.png");

    QMenuBar* menuBar = new QMenuBar();
    menuList["MENU1"] = menuBar->addMenu("&File"),
            subMenuList["Refresh"] = menuList["MENU1"]->addAction(refreshIcon, "&Refresh Screen"),
            subMenuList["Capture"] = menuList["MENU1"]->addAction(cameraIcon, "&Save Map as Image"),
            menuList["MENU1"]->addSeparator(),
            subMenuList["Quit"] = menuList["MENU1"]->addAction("&Quit");

    menuList["MENU2"] = menuBar->addMenu("&Map"),
            subMenuList["ZoomIn"] = menuList["MENU2"]->addAction(zoominIcon, "Zoom &In"),
            subMenuList["ZoomOut"] = menuList["MENU2"]->addAction(zoomoutIcon, "Zoom &Out"),
            subMenuList["HideChart"] = menuList["MENU2"]->addAction("Hide &Chart");

    menuList["MENU3"] = menuBar->addMenu("&Preferences"),
            subMenuList["ChangePassword"] = menuList["MENU3"]->addAction("Change &Password"),
            subMenuList["ChangeMonitoringSettings"] = menuList["MENU3"]->addAction("&Monitoring Settings");

    menuList["MENU4"] = menuBar->addMenu("&Help"),
            subMenuList["ShowOnlineResources"] = menuList["MENU4"]->addAction("Online &Resources"),
            menuList["MENU4"]->addSeparator(),
            subMenuList["ShowAbout"] = menuList["MENU4"]->addAction("&About " + appName.toUpper());

    subMenuList["Capture"]->setShortcut(QKeySequence::Save);
    subMenuList["Refresh"]->setShortcut(QKeySequence::Refresh);
    subMenuList["ZoomIn"]->setShortcut(QKeySequence::ZoomIn);
    subMenuList["ZoomOut"]->setShortcut(QKeySequence::ZoomOut);
    subMenuList["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
    subMenuList["Quit"]->setShortcut(QKeySequence::Quit);

    contextMenuList["FilterNodeRelatedMessages"] = nodeContextMenu->addAction("&Filter related messages");
    contextMenuList["CenterOnNode"] = nodeContextMenu->addAction("Center Graph &On");
    contextMenuList["Cancel"] = nodeContextMenu->addAction("&Cancel");

    QToolBar* toolBar = addToolBar(QString(ngrt4n::APP_NAME.c_str()));
    toolBar->addAction(subMenuList["Refresh"]);
    toolBar->addAction(subMenuList["ZoomIn"]);
    toolBar->addAction(subMenuList["ZoomOut"]);
    toolBar->addAction(subMenuList["Capture"]);
    toolBar->setIconSize(QSize(16,16));
    setMenuBar(menuBar);
}


void SvNavigator::openZabbixSession()
{
    zabbixHelper->setBaseUrl(monitorBaseUrl);
    QStringList params = QString::fromStdString("admin:zabbix").split(":");
    zabbixHelper->get(ZabbixHelper::LOGIN, params);
}

void SvNavigator::retrieveZabbixTriggers(const QString & host)
{
    QStringList params;
    params.push_back(zabbixAuthToken);
    params.push_back(host);
    zabbixHelper->get(ZabbixHelper::TRIGGER, params);
}

void SvNavigator::processZabbixReply(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
        return;

    string data = static_cast<QString>(reply->readAll()).toStdString();
    JsonHelper jsHelper(data);
    QScriptValue content = jsHelper.getProperty("result");
    QString dataId = jsHelper.getProperty("id").toString();
    if(dataId == "0") {
        zabbixAuthToken = content.toString();
        runZabbixMonitor();
    } else {
        QScriptValueIterator trigger(content);
        while (trigger.hasNext()) {

            trigger.next();
            QScriptValue triggerData = trigger.value();

            MonitorBroker::CheckT check ;
            check.check_command = triggerData.property("description").toString().toStdString() ;
            check.status = triggerData.property("value").toInt32() ;

            QScriptValueIterator host(triggerData.property("hosts"));
            if(host.hasNext()) {
                host.next();
                QScriptValue hostData = host.value();
                check.host = hostData.property("host").toString().toStdString() ;
            }

            QScriptValueIterator item(triggerData.property("items"));
            if(item.hasNext()) {
                item.next();
                QScriptValue itemData = item.value();
                check.last_state_change = itemData.property("lastclock").toString().toStdString() ;
            }
            cout << check.host << " status: " << check.status<< " lastchange:" << check.last_state_change << endl;
        }
    }
}

void SvNavigator::processZabbixError(QNetworkReply::NetworkError code){

    QString msg = ServiceOfficeMsg.arg("ZABBIX API on "+zabbixHelper->getApiUri()+".\n"
                                       +"Error code: "+QString::number(code));
    alert(msg);
}
