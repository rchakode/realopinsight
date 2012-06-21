/*
 * SvNavigator.cpp
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


#include "SvNavigator.hpp"
#include "core/MonitorBroker.hpp"
#include <crypt.h>
#include "core/ns.hpp"
#include <sstream>

ComboBoxItemsT SvNavigator::propRules() {
	ComboBoxItemsT map;
	map.insert(StatusPropRules::label(StatusPropRules::Unchanged),StatusPropRules::toString(StatusPropRules::Unchanged));
	map.insert(StatusPropRules::label(StatusPropRules::Decreased),StatusPropRules::toString(StatusPropRules::Decreased));
	map.insert(StatusPropRules::label(StatusPropRules::Increased),StatusPropRules::toString(StatusPropRules::Increased)) ;
	return map;
}

ComboBoxItemsT SvNavigator::calcRules() {
	ComboBoxItemsT map;
	map.insert(StatusCalcRules::label(StatusCalcRules::HighCriticity),StatusCalcRules::toString(StatusCalcRules::HighCriticity));
	map.insert(StatusCalcRules::label(StatusCalcRules::WeightedCriticity),StatusCalcRules::toString(StatusCalcRules::WeightedCriticity));
	return map;
}

SvNavigator::SvNavigator( const qint32 & _user_role, const QString & _config_file, QWidget* parent)
: QMainWindow(parent) ,
  configFile(_config_file),
  userRole (_user_role) , //TODO change implementation
  settings ( new Settings()) ,
  snavStruct ( new Struct()) ,
  statsPanel ( new Stats( ) ) ,
  filteredMsgPanel (NULL),
  mainSplitter ( new QSplitter( this ) ) ,
  rightSplitter ( new QSplitter() ) ,
  topRightPanel ( new QTabWidget()) ,
  bottomRightPanel ( new QTabWidget() ) ,
  webBrowser ( new WebKit() ) ,
  graphView ( new GraphView() ) ,
  navigationTree ( new SvNavigatorTree() ) ,
  monPrefWindow (new Preferences(_user_role, Preferences::ChangeMonitoringSettings)) ,
  changePasswdWindow (new Preferences(_user_role, Preferences::ChangePassword )) ,
  msgPanel( new MsgPanel() )
{
	setWindowTitle(configFile + " - " + QString(ngrt4n::APP_NAME.c_str()) + " | Dashboard") ;
	loadMenus();

	topRightPanel->addTab(graphView, "Dashboard") ;
	topRightPanel->addTab(webBrowser, "Native Web UI") ;
	bottomRightPanel->addTab(msgPanel, "Event Console") ;
	mainSplitter->addWidget(navigationTree) ;
	mainSplitter->addWidget(rightSplitter) ;
	rightSplitter->addWidget(topRightPanel) ;
	rightSplitter->addWidget(bottomRightPanel) ;
	rightSplitter->setOrientation(Qt::Vertical) ;
	setCentralWidget(mainSplitter) ;

	// Activate Qt's event handler on widget
	addEvents();

	// Get application settings
	webUIUrl = settings->value(Preferences::URL_KEY ).toString() ;
	serverAuthChain = settings->value(Preferences::SERVER_PASS_KEY).toString().toStdString() ;
	serverUrl = "tcp://"
			+ settings->value(Preferences::SERVER_ADDR_KEY).toString().toStdString()
			+ ":"
			+ settings->value(Preferences::SERVER_PORT_KEY).toString().toStdString();
}

SvNavigator::~SvNavigator()
{
	delete msgPanel ;
	delete statsPanel ;
	delete navigationTree ;
	delete webBrowser ;
	delete graphView ;
	delete snavStruct ;
	delete topRightPanel ;
	delete bottomRightPanel ;
	delete rightSplitter ;
	delete mainSplitter ;
	delete monPrefWindow ;
	delete changePasswdWindow ;
	delete comChannel ;
	if( filteredMsgPanel ) delete filteredMsgPanel ;
	unloadMenus();
}

void SvNavigator::closeEvent(QCloseEvent * event)
{
	if( filteredMsgPanel ) {
		filteredMsgPanel->close() ;
	}

	QMainWindow::closeEvent( event ) ;
}


void SvNavigator::contextMenuEvent(QContextMenuEvent * event)
{
	QPoint global_pos ;
	QList<QTreeWidgetItem*> tree_nodes ;
	QGraphicsItem* graph_node ;
	QString graph_item_id ;

	global_pos = event->globalPos() ;
	tree_nodes = navigationTree->selectedItems() ;
	graph_node = graphView->nodeAtGlobalPos( global_pos ) ;

	if ( tree_nodes.length() || graph_node ) {
		if( graph_node ) {
			graph_item_id = graph_node->data(0).toString() ;
			selectedNodeId =  graph_item_id.left( graph_item_id.indexOf(":") ) ;
		}
		else {
			selectedNodeId = tree_nodes[0]->data(0, QTreeWidgetItem::UserType).toString() ;
		}

		nodeContextMenu->exec( global_pos ) ;
	}
}

void SvNavigator::startMonitor()
{
	load() ;
	monitor();
	updateInterval = settings->value(Preferences::UPDATE_INTERVAL_KEY ).toInt() * 1000 ;
	if ( updateInterval <= 0 ) updateInterval = MonitorBroker::DEFAULT_UPDATE_INTERVAL * 1000 ;
	timerId = startTimer(updateInterval);
}

void SvNavigator::timerEvent(QTimerEvent *)
{
	monitor() ;
}

void SvNavigator::load(void)
{
	openedFile = configFile ;
	Parser config_parser ;
	config_parser.parseSvConfig(configFile, *snavStruct) ;
	navigationTree->clear() ;
	navigationTree->addTopLevelItem(snavStruct->tree_item_list[SvNavigatorTree::rootID]) ;
	graphView->load(config_parser.getDotGraphFile(), snavStruct->node_list) ;
	webBrowser->setUrl( webUIUrl ) ;
	resize() ;
	show() ;
	graphView->scaleToFitViewPort( ) ;
}

void SvNavigator::unloadMenus(void)
{
	subMenuList.clear() ;
	menuList.clear() ;
	delete nodeContextMenu ;
}

void SvNavigator::handleChangePasswordAction(void)
{
	changePasswdWindow->exec() ;
}

void SvNavigator::handleChangeMonitoringSettingsAction(void)
{
	monPrefWindow->exec() ;

	serverAuthChain = settings->value(Preferences::SERVER_PASS_KEY).toString().toStdString() ;
	updateInterval = settings->value(Preferences::UPDATE_INTERVAL_KEY).toInt() * 1000 ;
	if ( updateInterval <= 0 ) updateInterval = MonitorBroker::DEFAULT_UPDATE_INTERVAL * 1000 ;

	killTimer( timerId ) ;
	timerId = startTimer( updateInterval ) ;
}

void SvNavigator::handleShowOnlineResources(void)
{
	QDesktopServices app_launcher ;
	app_launcher.openUrl(QUrl("http://www.ngrt4n.com/docs/")) ;
}

void SvNavigator::handleShowAbout(void)
{
	Preferences about(userRole, Preferences::ShowAbout) ;
	about.exec() ;
}

int SvNavigator::monitor(void)
{
	NodeListT::iterator node_it ;
	MonitorBroker::NagiosChecksT nagios_checks;
	Parser config_parser;
	QStringList::const_iterator check_id_it, node_id_it ;
	QStringList child_nodes_list ;
	qint32 ok_count = 0 ;
	qint32 warning_count = 0;
	qint32 critical_count = 0 ;
	qint32 unknown_count = 0 ;

	//Initialize the communication channel with the broker module
	zmq::context_t comContext(1);
	comChannel = new zmq::socket_t (comContext, ZMQ_REQ);
	comChannel->connect(serverUrl.c_str());

	snavStruct->check_status_count.clear() ;
	for(check_id_it = snavStruct->check_list.begin(); check_id_it != snavStruct->check_list.end(); check_id_it++) {

		node_it = snavStruct->node_list.find( (*check_id_it).trimmed() ) ;
		if( node_it == snavStruct->node_list.end()) continue ;

		if( node_it->child_nodes == "" ) {
			node_it->status = MonitorBroker::UNKNOWN;
			unknown_count += 1 ;
			continue;
		}

		child_nodes_list = node_it->child_nodes.split( Parser::CHILD_NODES_SEP );
		for(node_id_it = child_nodes_list.begin(); node_id_it != child_nodes_list.end(); node_id_it++) 	{

			MonitorBroker::NagiosCheckT check ;
			string sid = serverAuthChain + ":"+(*node_id_it).trimmed().toStdString() ; //TODO

			zmq::message_t request(MonitorBroker::MAX_MSG);
			memset(request.data(), 0, MonitorBroker::MAX_MSG) ;
			memcpy(request.data(), sid.c_str(), sid.size());
			comChannel->send(request) ;

			zmq::message_t reply ;
			comChannel->recv(&reply) ;
			int msize = reply.size() ;
			char* result = (char*)malloc(msize *  sizeof(char)) ;
			memcpy(result, reply.data(), msize) ;

			QRegExp sepRgx; QStringList sInfoVec ; sepRgx.setPattern("#");
			sInfoVec = QString(result).split(sepRgx) ; free(result) ;

			switch(sInfoVec.length())
			{
			case 5 :
				check.status = sInfoVec[0].toInt() ;
				check.host = sInfoVec[1].toStdString() ;
				check.last_state_change = sInfoVec[2].toStdString() ;
				node_it->check.check_command = sInfoVec[3].toStdString() ;
				check.alarm_msg = sInfoVec[4].toStdString() ;
				break ;

			default :
				check.status = MonitorBroker::UNKNOWN ;
				check.alarm_msg = "ERROR :: " + sInfoVec[1].toStdString();
				check.host = "Unknown" ;
				check.last_state_change = "Unknown" ;
				node_it->check.check_command = "Unknown" ;
				break ;
			}

			switch( check.status ) {
			case MonitorBroker::OK:
				ok_count += 1 ;
				break;

			case MonitorBroker::WARNING:
				warning_count += 1 ;
				break;

			case MonitorBroker::CRITICAL:
				critical_count += 1 ;
				break;

			default:
				unknown_count += 1 ;
				break;
			}

			if ( node_it->status != check.status ) {
				QString toolTip = "" ;

				node_it->check = check ;
				node_it->status = check.status ;
				updateAlarmMsg(node_it) ;
				toolTip = getNodeToolTip(*node_it) ;

				updateNavTreeItemStatus(node_it, toolTip) ;
				graphView->updateNode(node_it, toolTip) ;
				msgPanel->addMsg(node_it) ;

				node_it->status = node_it->prop_status =  check.status ;
				emit hasToBeUpdate( node_it->parent ) ;
			}
		}
	}

	qint32 all_checks_count = snavStruct->check_list.size() ;
	snavStruct->check_status_count[MonitorBroker::OK] = ok_count ;
	snavStruct->check_status_count[MonitorBroker::WARNING] = warning_count ;
	snavStruct->check_status_count[MonitorBroker::CRITICAL] = critical_count ;
	snavStruct->check_status_count[MonitorBroker::UNKNOWN] = unknown_count ;

	if( all_checks_count>0) {

		Stats *stats = new Stats() ;
		QString info = stats->update(snavStruct->check_status_count, all_checks_count) ;
		stats->setToolTip(info) ;
		graphView->updateStatsPanel(stats) ;
		if(statsPanel) delete statsPanel ;

		statsPanel = stats ;
		msgPanel->sortItems(MsgPanel::msgPanelColumnCount - 1, Qt::DescendingOrder) ;
		msgPanel->resizeFields(msgPanelSize) ;
	}

	comChannel->close() ;
	return 0 ;
}

QString SvNavigator::getNodeToolTip(const NodeT & _node)
{
	QString toolTip="Service Name : "+_node.name + "\n"
			+"Description : " + const_cast<QString&>(_node.description).replace("\n", " ") + "\n"
			+"Status : "+Utils::statusToString(_node.status) + "\n"
			+"  Calc. rule : " + StatusCalcRules::label(_node.status_crule) + "\n"
			+"  Prop. rule : " + StatusPropRules::label(_node.status_prule);

	if ( _node.type == NodeType::ALARM_NODE ) {
		toolTip+= + "\n";
		toolTip+="Host : "+ QString(_node.check.host.c_str()).replace("\n", " ") + "\n";
		toolTip+="Service ID : "+_node.child_nodes + "\n";
		if( _node.status == MonitorBroker::OK ) {
			toolTip+="Message : "+const_cast<QString&>(_node.notification_msg).replace("\n", " ") + "\n";
		} else {
			toolTip+="Message : "+QString(_node.check.alarm_msg.c_str()).replace("\n", " ") + "\n";
		}
		toolTip+="Check Output : "+ QString(_node.check.alarm_msg.c_str()).replace("\n", " ");
	}

	return toolTip ;
}

void SvNavigator::updateAlarmMsg(NodeListT::iterator &  _node)
{
	QRegExp regexp ;
	QStringList splited_check_id ;
	QStringList splited_check_command ;
	QString  msg ;
	qint32 len ;

	splited_check_id = QString(_node->check.id.c_str()).split("/") ;
	len =  splited_check_id.length() ;

	if( _node->status == MonitorBroker::OK ) {
		msg = _node->notification_msg ;
	} else {
		msg = _node->alarm_msg ;
	}

	if( len ) {
		regexp.setPattern( MsgPanel::HOSTNAME_META_MSG_PATERN ) ;
		msg.replace(regexp, splited_check_id[0]) ;

		if( len == 2 ) {
			regexp.setPattern( MsgPanel::SERVICE_META_MSG_PATERN ) ;
			msg.replace(regexp, splited_check_id[1]) ;
		}
	}

	splited_check_command = QString(_node->check.check_command.c_str()).split("!") ;
	if( splited_check_command.length() >= 3) {
		regexp.setPattern( MsgPanel::THERESHOLD_META_MSG_PATERN ) ;
		msg.replace(regexp, splited_check_command[1]) ;

		if(_node->status == MonitorBroker::WARNING ) {
			msg.replace(regexp, splited_check_command[2]) ;
		}

	}

	if( _node->status == MonitorBroker::OK ) {
		_node->notification_msg = msg  ;
	}
	else {
		_node->alarm_msg = msg ;
	}
}

void SvNavigator::updateNodeStatus(QString _node_id)
{
	qint32 normal_count = 0 ;
	qint32 warning_count = 0 ;
	qint32 unknown_count = 0 ;
	qint32 critical_count = 0 ;

	NodeListT::iterator node = snavStruct->node_list.find( _node_id ) ;
	if (node == snavStruct->node_list.end() ) return  ;

	Status status;
	QStringList node_ids = node->child_nodes.split( Parser::CHILD_NODES_SEP ) ;
	for(QStringList::const_iterator it = node_ids.begin(); it != node_ids.end(); it++) {

		NodeListT::iterator child = snavStruct->node_list.find( *it ) ;
		if ( child == snavStruct->node_list.end() ) continue ;

		Status cst(static_cast<MonitorBroker::StatusT>(child->prop_status)) ;

		if(node->status_crule == StatusCalcRules::WeightedCriticity) {
			status = status / cst ;
		} else {
			status = status *  cst ;
		}

		switch(child->status) {

		case MonitorBroker::CRITICAL:
			critical_count ++ ;
			break;

		case MonitorBroker::WARNING:
			warning_count ++ ;
			break;

		case MonitorBroker::UNKNOWN:
			unknown_count ++ ;
			break ;

		case MonitorBroker::OK:
			normal_count ++ ;
			break ;

		default:
			break ;
		}
	}

	node->status = status.getValue() ;

	switch(node->status_prule) {
	case StatusPropRules::Increased: node->prop_status = (status++).getValue() ;
	break ;
	case StatusPropRules::Decreased: node->prop_status = (status--).getValue() ;
	break ;
	default : node->prop_status = node->status ;
	break ;

	}
	QString toolTip = getNodeToolTip(*node) ;
	graphView->updateNode(node, toolTip) ;
	updateNavTreeItemStatus(node, toolTip) ;
	emit hasToBeUpdate(node->parent);
}


void SvNavigator::updateNavTreeItemStatus(const NodeListT::iterator & _node, const QString & _tip)
{
	QIcon icon;
	switch(_node->status) {
	case MonitorBroker::OK:
		icon.addFile(":/images/normal.png") ;
		break;

	case MonitorBroker::WARNING:
		icon.addFile(":/images/warning.png") ;
		break;

	case MonitorBroker::CRITICAL:
		icon.addFile(":/images/critical.png") ;
		break;

	default:
		icon.addFile(":/images/unknown.png") ;
		break;
	}

	TreeNodeItemListT::iterator tnode_it = snavStruct->tree_item_list.find(_node->id) ;
	if(tnode_it != snavStruct->tree_item_list.end() ) {
		(*tnode_it)->setIcon(0, icon) ;
		(*tnode_it)->setToolTip(0, _tip) ;
	}
}

void SvNavigator::expandNode(const QString & _node_id, const bool & _expand, const qint32 & _level)
{
	QStringList child_nodes_list ;
	QStringList::iterator uds_it ;
	NodeT& node = snavStruct->node_list[_node_id] ;

	if( node.type == NodeType::SERVICE_NODE && node.child_nodes != "") {
		child_nodes_list = node.child_nodes.split( Parser::CHILD_NODES_SEP ) ;
		for (uds_it = child_nodes_list.begin(); uds_it != child_nodes_list.end(); uds_it++) {
			graphView->setNodeVisible(* uds_it, _node_id, _expand, _level) ;
		}
	}
}

void SvNavigator::centerGraphOnNode( const QString & _node_id )
{
	if( _node_id != "") selectedNodeId =  _node_id ;
	graphView->centerOnNode( selectedNodeId ) ;
}

void SvNavigator::filterNodeRelatedMsg(void)
{
	QString window_title ;
	NodeListT::iterator node_it ;

	if( filteredMsgPanel ) delete filteredMsgPanel ;
	filteredMsgPanel = new MsgPanel() ;
	node_it = snavStruct->node_list.find(selectedNodeId) ;

	if( node_it != snavStruct->node_list.end()) {
		filterNodeRelatedMsg( selectedNodeId ) ;
		window_title = "Filtered messages from the component '"
				+ snavStruct->node_list[selectedNodeId].name
				+ "' - " + QString(ngrt4n::APP_NAME.c_str()) ;
		filteredMsgPanel->resizeFields( msgPanelSize, true );
		filteredMsgPanel->setWindowTitle( window_title ) ;
	}

	filteredMsgPanel->show() ;
}

void SvNavigator::filterNodeRelatedMsg( const QString & _node_id )
{
	NodeListT::iterator node_it = snavStruct->node_list.find(_node_id) ;

	if(node_it == snavStruct->node_list.end()
			|| node_it->child_nodes == "" )
		return ;

	if ( node_it->type == NodeType::ALARM_NODE ) {
		filteredMsgPanel->addMsg(node_it) ;
	}
	else {
		QStringList u_servs = node_it->child_nodes.split( Parser::CHILD_NODES_SEP ) ;

		for(QStringList::iterator it = u_servs.begin() ; it != u_servs.end() ; it ++) {
			filterNodeRelatedMsg( *it ) ;
		}
	}
}


void SvNavigator::acknowledge(void)
{
	//TODO
}

void SvNavigator::tabChanged(int _tab_index)
{
	if ( _tab_index != 0 ) {
		subMenuList["ZoomIn"]->setEnabled( false ) ;
		subMenuList["ZoomOut"]->setEnabled( false ) ;
		return ;
	}

	subMenuList["ZoomIn"]->setEnabled( true ) ;
	subMenuList["ZoomOut"]->setEnabled( true ) ;
}

void SvNavigator::hideChart(void)
{
	if ( graphView->hideChart() ) {
		subMenuList["HideChart"]->setIcon(QIcon(":images/check.png"));
		return ;
	}
	subMenuList["HideChart"]->setIcon(QIcon("")) ;
}

void SvNavigator::centerGraphOnNode( QTreeWidgetItem * _item )
{
	centerGraphOnNode(_item->data(0, QTreeWidgetItem::UserType).toString() ) ;
}

void SvNavigator::resize(void)
{
	QList<qint32> frames_size ;
	const qreal GRAPH_HEIGHT_RATE = 0.50  ;

	QSize screen_size = qApp->desktop()->screen(0)->size() ;

	msgPanelSize = QSize(screen_size.width() * 0.80, screen_size.height() * (1.0 - GRAPH_HEIGHT_RATE)) ;

	frames_size.push_back(screen_size.width() * 0.20) ;
	frames_size.push_back(msgPanelSize.width()) ;
	mainSplitter->setSizes(frames_size) ;
	frames_size[0] = ( screen_size.height() * GRAPH_HEIGHT_RATE ),
			frames_size[1] = (msgPanelSize.height()),
			rightSplitter->setSizes(frames_size);

	mainSplitter->resize(screen_size.width(), screen_size.height() * 0.85);
	QMainWindow::resize(screen_size.width(),  screen_size.height());
}


void SvNavigator::loadMenus(void)
{
	QIcon camera_icon, zoomin_icon, zoomout_icon, refresh_icon ;
	refresh_icon.addFile(":images/refresh.png");
	camera_icon.addFile(":images/camera.png");
	zoomin_icon.addFile(":images/zoomin.png");
	zoomout_icon.addFile(":images/zoomout.png");

	QMenuBar* menuBar = new QMenuBar();
	menuList["MENU1"] = menuBar->addMenu("&File"),
			subMenuList["Refresh"] = menuList["MENU1"]->addAction(refresh_icon, "&Refresh Screen") ,
			subMenuList["Capture"] = menuList["MENU1"]->addAction(camera_icon, "&Save Map as Image") ,
			menuList["MENU1"]->addSeparator(),
			subMenuList["Quit"] = menuList["MENU1"]->addAction("&Quit");

	menuList["MENU2"] = menuBar->addMenu("&Map"),
			subMenuList["ZoomIn"] = menuList["MENU2"]->addAction(zoomin_icon, "Zoom &In"),
			subMenuList["ZoomOut"] = menuList["MENU2"]->addAction(zoomout_icon, "Zoom &Out"),
			subMenuList["HideChart"] = menuList["MENU2"]->addAction("Hide &Chart") ;

	menuList["MENU3"] = menuBar->addMenu("&Preferences"),
			subMenuList["ChangePassword"] = menuList["MENU3"]->addAction("Change &Password"),
			subMenuList["ChangeMonitoringSettings"] = menuList["MENU3"]->addAction("&Monitoring Settings");

	menuList["MENU4"] = menuBar->addMenu("&Help"),
			subMenuList["ShowOnlineResources"] = menuList["MENU4"]->addAction("Online &Resources"),
			menuList["MENU4"]->addSeparator(),
			subMenuList["ShowAbout"] = menuList["MENU4"]->addAction("&About " + QString(ngrt4n::APP_NAME.c_str()));

	subMenuList["Capture"]->setShortcut(QKeySequence::Save) ;
	subMenuList["Refresh"]->setShortcut(QKeySequence::Refresh) ;
	subMenuList["ZoomIn"]->setShortcut(QKeySequence::ZoomIn) ;
	subMenuList["ZoomOut"]->setShortcut(QKeySequence::ZoomOut) ;
	subMenuList["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents) ;
	subMenuList["Quit"]->setShortcut(QKeySequence::Quit) ;

	nodeContextMenu = new QMenu() ;
	contextMenuList["FilterNodeRelatedMessages"] = nodeContextMenu->addAction("&Filter related messages") ;
	contextMenuList["CenterOnNode"] = nodeContextMenu->addAction("Center Graph &On") ;
	contextMenuList["Cancel"] = nodeContextMenu->addAction("&Cancel") ;

	QToolBar* toolBar = addToolBar(QString(ngrt4n::APP_NAME.c_str())) ;
	toolBar->addAction(subMenuList["Refresh"]) ;
	toolBar->addAction(subMenuList["ZoomIn"]) ;
	toolBar->addAction(subMenuList["ZoomOut"]) ;
	toolBar->addAction(subMenuList["Capture"]) ;
	toolBar->setIconSize(QSize(16,16)) ;
	setMenuBar( menuBar ) ;
}


void SvNavigator::addEvents(void)
{
	connect( this, SIGNAL(sortEventConsole()), msgPanel, SLOT(sortEventConsole()) );
	connect( this, SIGNAL(hasToBeUpdate(QString)), this, SLOT( updateNodeStatus(QString) ));
	connect( subMenuList["Capture"], SIGNAL(triggered(bool)), graphView, SLOT( capture() ));
	connect( subMenuList["ZoomIn"], SIGNAL(triggered(bool)), graphView, SLOT( zoomIn() ) );
	connect( subMenuList["ZoomOut"], SIGNAL(triggered(bool)), graphView, SLOT( zoomOut() ));
	connect( subMenuList["HideChart"], SIGNAL(triggered(bool)), this, SLOT( hideChart() ));
	connect( subMenuList["Refresh"], SIGNAL(triggered(bool)), this, SLOT(monitor()));
	connect( subMenuList["ChangePassword"], SIGNAL( triggered(bool) ), this, SLOT(handleChangePasswordAction(void) ));
	connect( subMenuList["ChangeMonitoringSettings"], SIGNAL(triggered(bool)), this, SLOT(handleChangeMonitoringSettingsAction(void) ));
	connect( subMenuList["ShowAbout"], SIGNAL(triggered(bool)), this, SLOT(handleShowAbout()));
	connect( subMenuList["ShowOnlineResources"], SIGNAL(triggered(bool)), this, SLOT( handleShowOnlineResources() ));
	connect( subMenuList["Quit"], SIGNAL(triggered(bool)), qApp, SLOT( quit() ));
	connect( contextMenuList["FilterNodeRelatedMessages"], SIGNAL( triggered(bool) ), this, SLOT( filterNodeRelatedMsg()) );
	connect( contextMenuList["CenterOnNode"], SIGNAL( triggered(bool) ), this, SLOT( centerGraphOnNode() ));
	connect( monPrefWindow, SIGNAL( urlChanged(QString) ), webBrowser, SLOT(setUrl( QString ) ));
	connect( topRightPanel, SIGNAL(currentChanged (int)), this, SLOT( tabChanged( int ) ));
	connect( graphView, SIGNAL( expandNode(QString, bool, qint32)), this, SLOT( expandNode(const QString &, const bool &, const qint32 &) ));
	connect( navigationTree, SIGNAL( itemDoubleClicked(QTreeWidgetItem *, int) ), this, SLOT( centerGraphOnNode(QTreeWidgetItem *) ));
}
