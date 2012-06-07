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



SvNavigator::SvNavigator( const qint32 & _user_role, const QString & _config_file, QWidget* parent)
: QMainWindow(parent) ,
  configFile(_config_file) ,
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
  msgPanel( new MsgPanel() ),
  serverUrl ("tcp://localhost:1983") //TODO Set the url dynamically from config file
{
	setWindowTitle(configFile + " - " + QString(ngrt4n::APP_NAME.c_str()) + " | Dashboard") ;
	loadMenus();
	topRightPanel->addTab(graphView, "Dashboard") ;
	topRightPanel->addTab(webBrowser, "Native Web UI") ;
	bottomRightPanel->addTab(msgPanel, "Event Console") ;
	mainSplitter->addWidget( navigationTree ) ;
	mainSplitter->addWidget( rightSplitter ) ;
	rightSplitter->addWidget( topRightPanel ) ;
	rightSplitter->addWidget( bottomRightPanel ) ;
	rightSplitter->setOrientation( Qt::Vertical ) ;
	setCentralWidget( mainSplitter ) ;

	// Activate Qt's event handler on widget
	addEvents();
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

	if ( tree_nodes.length() || graph_node )
	{
		if( graph_node )
		{
			graph_item_id = graph_node->data(NODE_ID_DATA_INDEX).toString() ;
			selectedNodeId =  graph_item_id.left( graph_item_id.indexOf(":") ) ;
		}
		else
		{
			selectedNodeId = tree_nodes[0]->text(TREE_NODE_ID_COLUMN) ;
		}

		nodeContextMenu->exec( global_pos ) ;
	}
}

void SvNavigator::timerEvent(QTimerEvent *)
{
	monitor() ;
}

void SvNavigator::load(void)
{
	Parser config_parser ;

	openedFile = configFile ;
	config_parser.parseSvConfig(configFile, *snavStruct) ;

	navigationTree->clear() ;
	navigationTree->addTopLevelItem(snavStruct->tree_item_list[snavStruct->root_id]) ;
	graphView->load(config_parser.getDotGraphFile(), snavStruct->node_list) ;

	webUIUrl = settings->value(Preferences::NAGIOS_URL_KEY ).toString() ;
	webBrowser->setUrl( webUIUrl ) ;

	resize() ;

	monitor();

	show() ;

	graphView->scaleToFitViewPort( ) ;

	updateInterval = settings->value(Preferences::UPDATE_INTERVAL_KEY ).toInt() * 1000 ;
	if ( updateInterval <= 0 ) updateInterval = MonitorBroker::DEFAULT_UPDATE_INTERVAL * 1000 ;
	timerId = startTimer( updateInterval ) ;
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

	killTimer( timerId ) ;

	updateInterval = settings->value(Preferences::UPDATE_INTERVAL_KEY ).toInt() * 1000 ;

	if ( updateInterval <= 0 ) updateInterval = MonitorBroker::DEFAULT_UPDATE_INTERVAL * 1000 ;

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

	//Initialize the communication channel with the server
	zmq::context_t comContext(1);
	comChannel = new zmq::socket_t (comContext, ZMQ_REQ);
	comChannel->connect(serverUrl.c_str());

	qint32 all_checks_count = 0 ;
	snavStruct->check_status_count.clear() ;
	for(check_id_it = snavStruct->check_list.begin();
			check_id_it != snavStruct->check_list.end(); check_id_it++) {

		node_it = snavStruct->node_list.find( (*check_id_it).trimmed() ) ;
		if( node_it == snavStruct->node_list.end()) continue ;

		if( node_it->child_nodes == "" ) {
			node_it->status = MonitorBroker::NAGIOS_UNKNOWN;
			unknown_count += 1 ;
			continue;
		}

		child_nodes_list = node_it->child_nodes.split( Parser::CHILD_NODES_SEP );
		for(node_id_it = child_nodes_list.begin(); node_id_it != child_nodes_list.end(); node_id_it++) 	{

			MonitorBroker::NagiosCheckT check ;
			string sid = "$1$$2buz9IzkVLweFQeOPcuS61:"+(*node_id_it).trimmed().toStdString() ;

			// Prepare and send request
			zmq::message_t request( MonitorBroker::MAX_MSG );
			memset(request.data(), 0, MonitorBroker::MAX_MSG) ;
			memcpy(request.data(), sid.c_str(), sid.size());
			comChannel->send(request);

			//  Get reply.
			zmq::message_t reply ;
			comChannel->recv(&reply);
			int msize = reply.size() ;
			char* result = (char*)malloc(msize *  sizeof(char)) ;
			memcpy(result, reply.data(), msize) ;

			QRegExp sepRgx; QStringList sInfoVec ; sepRgx.setPattern("#");
			sInfoVec = QString(result).split(sepRgx) ; free(result) ;

			if( sInfoVec.length() != 5) {
				cerr << "ERROR :: " << sInfoVec[1].toStdString() << endl ;
				unknown_count += 1 ;
				continue ;
			}

			check.status = sInfoVec[0].toInt() ;
			check.host = sInfoVec[1].toStdString() ;
			check.last_state_change = sInfoVec[2].toStdString() ;
			node_it->check.check_command = sInfoVec[3].toStdString() ;
			check.alarm_msg = sInfoVec[4].toStdString() ;

			switch( node_it->check.status ) {
			case MonitorBroker::NAGIOS_OK:
				ok_count += 1 ;
				break;

			case MonitorBroker::NAGIOS_WARNING:
				warning_count += 1 ;
				break;

			case MonitorBroker::NAGIOS_CRITICAL:
				critical_count += 1 ;
				break;

			default:
				unknown_count += 1 ;
				break;
			}

			if ( node_it->status != check.status ) {
				node_it->status = check.status ;
				node_it->check = check ;
				updateAlarmMsg(node_it) ;
				QString toolTip = getNodeToolTip(*node_it) ;
				updateNavTreeItemStatus(node_it, toolTip) ;
				graphView->updateNode(node_it, toolTip) ;
				msgPanel->addMsg(node_it) ;

				emit hasToBeUpdate( node_it->parent ) ;
			}

//			if( result ) free(result) ;
//			if( reply.data() ) free(reply.data()) ;
		}
	}

	all_checks_count = snavStruct->check_list.size() ;
	snavStruct->check_status_count[MonitorBroker::NAGIOS_OK] = ok_count ;
	snavStruct->check_status_count[MonitorBroker::NAGIOS_WARNING] = warning_count ;
	snavStruct->check_status_count[MonitorBroker::NAGIOS_CRITICAL] = critical_count ;
	snavStruct->check_status_count[MonitorBroker::NAGIOS_UNKNOWN] = unknown_count ;

	if( all_checks_count ) {
		Stats *  stats ;

		stats = new  Stats() ;

		stats->update(snavStruct->check_status_count, all_checks_count, statsPanelTooltip) ;
		graphView->updateStatsPanel( stats, statsPanelTooltip ) ;

		if( statsPanel ) delete statsPanel ;

		statsPanel = stats ;

		msgPanel->sortItems(MsgPanel::msgPanelColumnCount - 1, Qt::DescendingOrder) ;
		msgPanel->resizeFields( msgPanelSize ) ;
	}

	comChannel->close() ;
	return 0 ;
}

QString SvNavigator::getNodeToolTip(const NodeT & _node)
{

	QString toolTip = "Name: " + _node.name  +
			"\nDescription: " + const_cast<QString&>(_node.description).replace("\n", " ") +
			"\nStatus: " + Utils::statusToString(_node.status);

	if ( _node.type == NodeTypeT::ALARM_NODE ) {

		if( _node.status == MonitorBroker::NAGIOS_OK ) {
			toolTip += "\nMessage: " + const_cast<QString&>(_node.notification_msg).replace("\n", " ");
		}
		else {
			toolTip += "\nMessage: " + QString(_node.check.alarm_msg.c_str()).replace("\n", " ");
		}

		toolTip += "\nCheck Ouput: " +  QString(_node.check.alarm_msg.c_str()).replace("\n", " ");
		toolTip += "\nCheck Id: " + _node.child_nodes ;
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

	if( _node->status == MonitorBroker::NAGIOS_OK ) {
		msg = _node->notification_msg ;
	}
	else {
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

		if(_node->status == MonitorBroker::NAGIOS_WARNING ) {
			msg.replace(regexp, splited_check_command[2]) ;
		}

	}

	if( _node->status == MonitorBroker::NAGIOS_OK ) {
		_node->notification_msg = msg  ;
	}
	else {
		_node->alarm_msg = msg ;
	}
}

void SvNavigator::updateNodeStatus(QString _node_id)
{
	NodeListT::iterator node_it, child_node_it ;
	QStringList node_ids_list;
	QStringList::const_iterator it ;
	qint32 normal_count, warning_count, unknown_count, critical_count, sum_counts ;

	normal_count = warning_count = unknown_count = critical_count = 0 ;

	node_it = snavStruct->node_list.find( _node_id ) ;

	if (node_it != snavStruct->node_list.end() ) {
		node_ids_list = node_it->child_nodes.split( Parser::CHILD_NODES_SEP ) ;
		sum_counts = node_ids_list.size() ;

		for(it = node_ids_list.begin(); it != node_ids_list.end(); it++) {
			child_node_it = snavStruct->node_list.find( *it ) ;
			if ( child_node_it == snavStruct->node_list.end() )
				continue ;

			switch(child_node_it->status) {
			case MonitorBroker::NAGIOS_CRITICAL:
				node_it->status = MonitorBroker::NAGIOS_CRITICAL ;
				critical_count ++ ;

				break;

			case MonitorBroker::NAGIOS_WARNING:
				if(node_it->status != MonitorBroker::NAGIOS_CRITICAL) node_it->status = MonitorBroker::NAGIOS_WARNING;
				warning_count ++ ;

				break;

			case MonitorBroker::NAGIOS_UNKNOWN:
				if(node_it->status != MonitorBroker::NAGIOS_CRITICAL
						&& node_it->status != MonitorBroker::NAGIOS_WARNING) node_it->status = MonitorBroker::NAGIOS_UNKNOWN ;
				unknown_count ++ ;

				break ;

			case MonitorBroker::NAGIOS_OK:
				normal_count ++ ;

				break ;

			default:
				break ;
			}
		}

		if ( normal_count == sum_counts ) {
			node_it->status = MonitorBroker::NAGIOS_OK ;
		}
		else if ( node_it->status_calc_rule == WEIGHTED_CRITICITY_CALC_RULE ) {
			node_it->status = MonitorBroker::NAGIOS_WARNING ;
			if ( critical_count == sum_counts ) node_it->status = MonitorBroker::NAGIOS_CRITICAL ;
			else if ( unknown_count == sum_counts ) node_it->status = MonitorBroker::NAGIOS_UNKNOWN ;
		}

		QString toolTip = getNodeToolTip(*node_it) ;
		graphView->updateNode(node_it, toolTip) ;
		updateNavTreeItemStatus(node_it, toolTip) ;
		emit hasToBeUpdate(node_it->parent);
	}
}


void SvNavigator::updateNavTreeItemStatus(const NodeListT::iterator & _node, const QString & _tool_tip)
{
	QIcon icon;
	TreeNodeItemListT::iterator tnode_it ;
	switch(_node->status) {
	case MonitorBroker::NAGIOS_OK:
		icon.addFile(":/images/normal.png") ;
		break;

	case MonitorBroker::NAGIOS_WARNING:
		icon.addFile(":/images/warning.png") ;
		break;

	case MonitorBroker::NAGIOS_CRITICAL:
		icon.addFile(":/images/critical.png") ;
		break;

	default:
		icon.addFile(":/images/unknown.png") ;
		break;
	}

	tnode_it = snavStruct->tree_item_list.find(_node->id) ;
	if(tnode_it != snavStruct->tree_item_list.end() ) {
		(*tnode_it)->setIcon(0, icon) ;
		(*tnode_it)->setToolTip(0, _tool_tip) ;
	}
}

void SvNavigator::expandNode(const QString & _node_id, const bool & _expand, const qint32 & _level)
{
	QStringList child_nodes_list ;
	QStringList::iterator uds_it ;
	NodeT& node = snavStruct->node_list[_node_id] ;

	if( node.type == NodeTypeT::SERVICE_NODE && node.child_nodes != "") {
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

	if ( node_it->type == NodeTypeT::ALARM_NODE ) {
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
	if ( _tab_index != 0 )
	{
		subMenuList["ZoomIn"]->setEnabled( false ) ;
		subMenuList["ZoomOut"]->setEnabled( false ) ;
	}
	else
	{
		subMenuList["ZoomIn"]->setEnabled( true ) ;
		subMenuList["ZoomOut"]->setEnabled( true ) ;
	}
}

void SvNavigator::hideChart(void)
{
	if ( graphView->hideChart() )
	{
		subMenuList["HideChart"]->setIcon( QIcon(":images/check.png"));
	}
	else
	{
		subMenuList["HideChart"]->setIcon(QIcon("")) ;
	}
}

void SvNavigator::centerGraphOnNode( QTreeWidgetItem * _item )
{
	centerGraphOnNode( _item->text(TREE_NODE_ID_COLUMN) ) ;
}

void SvNavigator::resize(void)
{
	QSize screen_size, msg_panel_size, graph_port_view ;
	QList<qint32> frames_size ;
	const qreal GRAPH_HEIGHT_RATE = 0.50  ;

	screen_size = qApp->desktop()->screen(0)->size() ;
	graph_port_view =  QSize( screen_size.width() * 0.80, screen_size.height() * GRAPH_HEIGHT_RATE ) ;

	msgPanelSize = QSize( screen_size.width() * 0.80, screen_size.height() * (1.0 - GRAPH_HEIGHT_RATE) ) ;

	frames_size.push_back( screen_size.width() * 0.20 ) ;
	frames_size.push_back( msgPanelSize.width() ) ;
	mainSplitter->setSizes( frames_size ) ;

	frames_size[0] = ( screen_size.height() * GRAPH_HEIGHT_RATE ),
			frames_size[1] = ( msgPanelSize.height() ),
			rightSplitter->setSizes( frames_size );

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
	connect( this, SIGNAL( hasToBeUpdate( QString ) ), this, SLOT( updateNodeStatus( QString ) ));
	connect( subMenuList["Capture"], SIGNAL( triggered(bool) ), graphView, SLOT( capture() ));
	connect( subMenuList["ZoomIn"], SIGNAL( triggered(bool) ), graphView, SLOT( zoomIn() ) );
	connect( subMenuList["ZoomOut"], SIGNAL( triggered(bool) ), graphView, SLOT( zoomOut() ));
	connect( subMenuList["HideChart"], SIGNAL( triggered(bool) ), this, SLOT( hideChart() ));
	connect( subMenuList["Refresh"], SIGNAL( triggered(bool) ), this, SLOT( monitor() ));
	connect( subMenuList["ChangePassword"], SIGNAL( triggered(bool) ), this, SLOT(handleChangePasswordAction(void) ));
	connect( subMenuList["ChangeMonitoringSettings"], SIGNAL( triggered(bool) ), this, SLOT(handleChangeMonitoringSettingsAction(void) ));
	connect( subMenuList["ShowAbout"], SIGNAL( triggered(bool) ), this, SLOT( handleShowAbout() ));
	connect( subMenuList["ShowOnlineResources"], SIGNAL( triggered(bool) ), this, SLOT( handleShowOnlineResources() ));
	connect( subMenuList["Quit"], SIGNAL( triggered(bool) ), qApp, SLOT( quit() ));
	connect( contextMenuList["FilterNodeRelatedMessages"], SIGNAL( triggered(bool) ), this, SLOT( filterNodeRelatedMsg()) );
	connect( contextMenuList["CenterOnNode"], SIGNAL( triggered(bool) ), this, SLOT( centerGraphOnNode() ));
	connect( monPrefWindow, SIGNAL( urlChanged(QString) ), webBrowser, SLOT(setUrl( QString ) ));
	connect( topRightPanel, SIGNAL(currentChanged (int)), this, SLOT( tabChanged( int ) ));
	connect( graphView, SIGNAL( expandNode(QString, bool, qint32)), this, SLOT( expandNode(const QString &, const bool &, const qint32 &) ));
	connect( navigationTree, SIGNAL( itemDoubleClicked(QTreeWidgetItem *, int) ), this, SLOT( centerGraphOnNode(QTreeWidgetItem *) ));
}
