/*
 * GraphView.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 13-05-2012												   #
#																		   #
# This file is part of NGRT4N (http://ngrt4n.com).						   #
#																		   #
# NGRT4N is free software: you can redistribute it and/or modify		   #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.									   #
#																		   #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of		   #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	           #
# GNU General Public License for more details.							   #
#																		   #
# You should have received a copy of the GNU General Public License		   #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.		   #
#--------------------------------------------------------------------------#
*/


#include "../include/GraphView.hpp"

const qreal GraphView::XScalingRatio = 75.0 ;
const qreal GraphView::YScalingRatio = 100.0 ;

GraphView::GraphView(QWidget* _parent)
: QGraphicsView(_parent),
  statsPanelItem ( 0 ),
  portViewScalingRatio (1),
  statsPanelScaleRatio (1),
  isAjustedStatsPanelSize (false)
{
	statsPanelItem = NULL ;
	graphScene = new QGraphicsScene() , setScene(graphScene);

	iconMap[DEFAULT_ICON] = ":/images/business-process.png";
	iconMap[OTH_CHECK_ICON] = ":/images/check.png";

	iconMap[SERVER_ICON] = ":/images/server.png";
	iconMap[FIREWALL_ICON] = ":/images/firewall.png";
	iconMap[ROUTER_ICON] = ":/images/router.png";
	iconMap[NETWORK_ICON] = ":/images/network.png";
	iconMap[SWITCH_ICON] = ":/images/switch.png";
	iconMap[FILER_ICON] = ":/images/filer.png";
	iconMap[HARDDISK_ICON] = ":/images/harddisk.png";
	iconMap[STORAGE_ICON] = ":/images/storage.png";

	iconMap[LINUX_ICON] = ":/images/linux.png" ;
	iconMap[WINDOWS_ICON] = ":/images/windows.png" ;
	iconMap[SOLARIS_ICON] = ":/images/solaris.png" ;
	iconMap[CLOUD_ICON] = ":/images/cloud.png" ;
	iconMap[HYPERVISOR_ICON] = ":/images/hypervisor.png" ;

	iconMap[APP_ICON] = ":/images/application.png" ;
	iconMap[WEB_ICON] = ":/images/web.png";
	iconMap[WEBSERVER_ICON] = ":/images/web-server.png";
	iconMap[DB_ICON] = ":/images/db.png";
	iconMap[DBSERVER_ICON] = ":/images/db-server.png" ;
	iconMap[PROCESS_ICON] = ":/images/process.png";
	iconMap[LOG_ICON] = ":/images/log.png";

	iconMap[PLUS] = ":/images/plus.png";
	iconMap[MINUS] = ":/images/minus.png";
}

GraphView::~GraphView()
{
	delete graphScene ;
	gnodesList.clear() ;
	edgesList.clear() ;
	iconMap.clear() ;
}

void GraphView::mouseReleaseEvent( QMouseEvent * _event)
{
	QGraphicsItem* item ;
	QStringList list ;
	QString n_id, sfx ;
	QPixmap exp_icon ;

	item = graphScene->itemAt( mapToScene( _event->pos() ) );
	if(item)
	{
		if (_event->button() == Qt::RightButton )
		{
			emit rightClickOnItem( item, _event->globalPos() ) ;

			return ;
		}

		list = item->data(NODE_ID_DATA_INDEX).toString().split(":");
		if(list.length() == 2)
		{
			n_id = list[0];
			sfx = ":" + list[1];
			if(sfx == NODE_EXP_ICON_ID_SFX)
			{
				if ( gnodesList[n_id].expand )
				{
					exp_icon.load(iconMap[MINUS], 0, Qt::AutoColor);
					gnodesList[n_id].expand = false;
				}
				else
				{
					exp_icon.load(iconMap[PLUS], 0, Qt::AutoColor);
					gnodesList[n_id].expand = true;
				}
				gnodesList[n_id].exp_icon->setPixmap(exp_icon);
				emit expandNode(n_id, gnodesList[n_id].expand, 1);
			}
		}
	}
}

void GraphView::mouseDoubleClickEvent( QMouseEvent * _event)
{
	QGraphicsItem* item;
	QPointF pos;

	pos = mapToScene( QPoint( _event->pos() ) );
	item = graphScene->itemAt( pos );

	if(item)
	{
		centerOn( pos ) ;
	}
}


void GraphView::wheelEvent( QWheelEvent * _event)
{
	if( _event->delta() > 0 )
	{
		zoomIn() ;
	}
	else
	{
		zoomOut() ;
	}
}


void GraphView::resizeEvent ( QResizeEvent * )
{
	setStatsPanelPos() ;
}

void GraphView::showEvent(QShowEvent * )
{
	setStatsPanelPos() ;
}

void GraphView::scrollContentsBy ( int dx, int dy )
{
	QGraphicsView::scrollContentsBy (dx, dy) ;
	setStatsPanelPos() ;
}

void GraphView::zoomIn()
{
	scale(1.1, 1.1) ;
	if(statsPanelItem)
	{
		setStatsPanelPos() ;
		statsPanelItem->scale(1 / 1.1, 1 / 1.1) ;
	}
}

void GraphView::zoomOut()
{
	scale(1 / 1.1, 1 / 1.1) ;

	if(statsPanelItem)
	{
		statsPanelItem->scale(1.1, 1.1) ;
		setStatsPanelPos() ;
	}
}

void GraphView::updateStatsPanel(Stats * _stats_panel, const QString & _tool_tip )
{
	bool visible ;
	if( statsPanelItem )
	{
		visible = statsPanelItem->isVisible() ;
		statsPanelItem->setWidget( _stats_panel ) ;
		statsPanelItem->setVisible( visible ) ;
	}
	else
	{
		statsPanelItem = graphScene->addWidget( _stats_panel ) ;
	}

	if( statsPanelItem )
	{
		statsPanelItem->setToolTip( _tool_tip ) ;
		setStatsPanelPos() ;
		if ( ! isAjustedStatsPanelSize ) ajustStatsPanelSize() ;
	}
}

void GraphView::setStatsPanelPos(void)
{
	qreal xp ;

	if( statsPanelItem )
	{
		xp = size().width() - statsPanelItem->size().width() *  statsPanelScaleRatio - 2 ;
		statsPanelItem->setPos( mapToScene(QPoint(xp, 0)) ) ;
	}
}


bool GraphView::hideChart(void)
{
	bool visible ;

	visible = statsPanelItem->isVisible() ;

	statsPanelItem->setVisible( ! visible ) ;

	return visible ;
}


bool GraphView::load(const QString & _dot_file, const NodeListT & _service_list)
{
	QStringList arguments;
	QProcess* dot_parser = new QProcess();
	QProcess p;

	coodinatesGraphFile = _dot_file + ".plain";
	arguments << "-Tplain"<< "-o" << coodinatesGraphFile << _dot_file;

	int exitCode = dot_parser->execute("dot", arguments);
	dot_parser->waitForFinished(60000);

	if ( ! exitCode )
	{
		drawGraph( _service_list ) ;
		graphScene->setSceneRect(graphScene->itemsBoundingRect()) ;
	}
	else
	{
		qDebug() << "The graph engine exited with the code: " << exitCode;
		exit( exitCode ) ;
	}

	delete dot_parser;
	return false;
}

void GraphView::drawGraph(const NodeListT & _node_list)
{
	QFile cood_graph_file;
	QTextStream cood_graph_file_s;
	QString line, image_dir;
	QRegExp sep_regex;
	QStringList splited_line;
	NodeListT::const_iterator node_it ;
	QPen pen;

	if (cood_graph_file.setFileName(coodinatesGraphFile),
			cood_graph_file.open(QFile::ReadOnly))
	{
		pen.setColor( UNKNOWN_COLOR );

		sep_regex.setPattern("[ ]+");
		cood_graph_file_s.setDevice(& cood_graph_file );

		while (line = cood_graph_file_s.readLine(0), ! line.isNull())
		{
			splited_line = line.split (sep_regex);

			if (splited_line[0] == "node")
			{
				QString n_id = splited_line[1].trimmed() ;
				qreal x_corner = splited_line[2].trimmed().toFloat() ;
				qreal y_corner = -1 * splited_line[3].trimmed().toFloat() ;

				node_it = _node_list.find( n_id ) ;

				if( node_it == _node_list.end() )
					continue ;

				gnodesList[n_id].type = node_it->type ;
				gnodesList[n_id].expand = true;

				QPointF n_label_origin = QPointF(x_corner * XScalingRatio, y_corner * YScalingRatio);

				drawNode( *node_it) ;
				setNodePos(n_id, n_label_origin);
			}
			else if (splited_line[0] == "edge")
			{
				QString edge_name; 	QPainterPath path;
				setEdgePath(splited_line[1], splited_line[2], path);
				edge_name =  splited_line[1] + ":" + splited_line[2];

				edgesList[edge_name].edge = new QGraphicsPathItem(path),
						edgesList[edge_name].edge->setPen(pen),
						graphScene->addItem(edgesList[edge_name].edge),
						edgesList[edge_name].edge->setZValue(-20);
			}
			else if(splited_line[0] == "stop")
			{
				break;
			}
		}
	}
}

void GraphView::drawNode(const NodeT & _node )
{
	QString html_n_label ;
	QPixmap icon, exp_icon ;

	icon.load(iconMap[_node.icon], 0, Qt::AutoColor);  // TODO
	exp_icon.load(iconMap[PLUS], 0, Qt::AutoColor);
	html_n_label = "<span style=\"background: 'lightgray'\">&nbsp;" + _node.name + "&nbsp;</span>";

	gnodesList[_node.id].label = new QGraphicsTextItem(),
			gnodesList[_node.id].label->setHtml(html_n_label),
			gnodesList[_node.id].label->setData(NODE_ID_DATA_INDEX, _node.id + NODE_LABEL_ID_SFX),
			graphScene->addItem(gnodesList[_node.id].label),
			gnodesList[_node.id].label->setZValue(-5);

	gnodesList[_node.id].icon = new QGraphicsPixmapItem(icon),
			gnodesList[_node.id].icon->setData(NODE_ID_DATA_INDEX, _node.id + NODE_ICON_ID_SFX),
			graphScene->addItem(gnodesList[_node.id].icon),
			gnodesList[_node.id].icon->setZValue(-10);

	gnodesList[_node.id].exp_icon = new QGraphicsPixmapItem(exp_icon),
			gnodesList[_node.id].exp_icon->setData(NODE_ID_DATA_INDEX, _node.id + NODE_EXP_ICON_ID_SFX),
			graphScene->addItem(gnodesList[_node.id].exp_icon),
			gnodesList[_node.id].exp_icon->setZValue(0) ;

	if(gnodesList[_node.id].type == ALARM_NODE) 	gnodesList[_node.id].exp_icon->setVisible(false);

	setNodeToolTip( _node ) ;
}

void GraphView::updateNodeColor(const NodeListT::iterator & _node)
{
	QString n_label, html_n_label ;
	GEdgeListT::iterator edge_it ;
	QColor color ;

	switch (_node->status)
	{
	case NAGIOS_OK:
		color = OK_COLOR ;
		break;

	case NAGIOS_WARNING:
		color = WARNING_COLOR ;
		break;

	case NAGIOS_CRITICAL:
		color = CRITICAL_COLOR ;
		break;

	default:
		color = UNKNOWN_COLOR ;
		break;
	}

	html_n_label = "<span style=\"background: '" + color.name() + "'\">&nbsp;" +_node->name + "&nbsp;</span>";

	gnodesList[_node->id].label->setHtml(html_n_label);

	edge_it = edgesList.find( _node->parent + ":" + _node->id ) ;

	if( edge_it != edgesList.end() ) edge_it->edge->setPen( color );
}

void GraphView::updateNode(const NodeListT::iterator & _node_it, const QString & _tool_tip)
{
	QString n_label, html_n_label ;
	GEdgeListT::iterator edge_it ;
	QColor color ;

	switch (_node_it->status)
	{
	case NAGIOS_OK:
		color = OK_COLOR ;
		break;

	case NAGIOS_WARNING:
		color = WARNING_COLOR ;
		break;

	case NAGIOS_CRITICAL:
		color = CRITICAL_COLOR ;
		break;

	default:
		color = UNKNOWN_COLOR ;
		break;
	}
	html_n_label = "<span style=\"background: '" + color.name() + "'\">&nbsp;" +_node_it->name + "&nbsp;</span>";

	GNodeListT::iterator gnode_it =  gnodesList.find(_node_it->id);
	if (gnode_it != gnodesList.end() )
	{
		gnode_it->label->setHtml(html_n_label);
		gnode_it->icon->setToolTip(_tool_tip);
		gnode_it->label->setToolTip(_tool_tip);

		edge_it = edgesList.find( _node_it->parent + ":" + _node_it->id ) ;
		if( edge_it != edgesList.end() )
			edge_it->edge->setPen( color );
	}
}

void GraphView::setEdgePath(const QString& _parent_vertex, const QString& _child_vertex, QPainterPath& path)
{
	QPointF parent_anchor_point, child_anchor_point;
	QSizeF p_size, c_size;

	_GNode & p_gnode = gnodesList[_parent_vertex] ;
	_GNode & c_gnode = gnodesList[_child_vertex] ;

	p_size = p_gnode.exp_icon->boundingRect().size();
	c_size = c_gnode.icon->boundingRect().size();
	parent_anchor_point = p_gnode.exp_icon->pos() + QPointF(0.5 * p_size.width(), p_size.height());
	child_anchor_point = c_gnode.icon->pos() + QPointF(0.5 * c_size.width(), 0);

	path.moveTo(parent_anchor_point),
			path.lineTo(child_anchor_point);

	if( ! p_gnode.exp_icon->isVisible() )
	{
		p_gnode.exp_icon->setVisible(true);
	}
}

void GraphView::setNodePos(const QString & _node_id, const QPointF & _pos)
{
	QSizeF l_size, i_size, ei_size;
	qreal xi, xl, yi, yl, xei, yei;

	l_size = gnodesList[_node_id].label->boundingRect().size();
	i_size = gnodesList[_node_id].icon->boundingRect().size();
	ei_size = gnodesList[_node_id].exp_icon->boundingRect().size();

	xi = _pos.x() + 0.5 * (l_size.width() - i_size.width());
	yi = _pos.y();
	xei = _pos.x() + 0.5 * (l_size.width() - ei_size.width());
	xl = _pos.x();
	yl = _pos.y() + 0.75 * i_size.height();
	yei = yl + 0.75 * l_size.height();

	gnodesList[_node_id].label->setPos(xl, yl);
	gnodesList[_node_id].icon->setPos(xi, yi);
	gnodesList[_node_id].exp_icon->setPos(xei, yei);
}


void GraphView::setNodeToolTip(const NodeT & _node)
{
	QString msg;

	msg = "Name: " + _node.name  +
			"\nDescription: " + static_cast<QString>(_node.description).replace("\n", " ") +
			"\nStatus: " + Utils::statusToString(_node.status);

	if ( _node.type == ALARM_NODE )
	{

		if( _node.status == NAGIOS_OK )
		{
			msg += "\nMessage: " + static_cast<QString>(_node.notification_msg).replace("\n", " ");
		}
		else
		{
			msg += "\nMessage: " + static_cast<QString>(_node.alarm_msg).replace("\n", " ");
		}

		msg += "\nCheck Ouput: " +  static_cast<QString>(_node.check.alarm_msg).replace("\n", " ");
		msg += "\nCheck Id: " + _node.child_nodes ;
	}

	gnodesList[_node.id].icon->setToolTip(msg);
	gnodesList[_node.id].label->setToolTip(msg);
}


void GraphView::setNodeVisible(const QString & _node_id,
		const QString & _parent, const bool & _visible, const qint32 & _level)
{
	GNodeListT::iterator gnode;
	QPixmap exp_icon;
	QString e_id;

	e_id = _parent + ":" + _node_id;
	gnode = gnodesList.find(_node_id);

	if(gnode != gnodesList.end())
	{
		gnode->expand = _visible;
		gnode->label->setVisible(_visible);
		gnode->icon->setVisible(_visible);
		edgesList[e_id].edge->setVisible(_visible);

		if( gnode->type == SERVICE_NODE )
		{
			gnode->exp_icon->setVisible(_visible);
		}
		if( _visible )
		{
			exp_icon.load(iconMap[PLUS], 0, Qt::AutoColor);
			gnodesList[_node_id].exp_icon->setPixmap(exp_icon);
		}
		emit expandNode(_node_id, _visible, _level + 1);
	}
}

void GraphView::scaleToFitViewPort(void)
{
	QSizeF scene_size, view_size, stat_panel_size ;

	view_size = size() ;
	scene_size = graphScene->itemsBoundingRect().size() ;

	portViewScalingRatio = qMin(view_size.width() / scene_size.width(), view_size.height() / scene_size.height() )  ;
	if ( portViewScalingRatio < 1.0 )
	{
		if( statsPanelItem ) statsPanelItem->scale(1 / portViewScalingRatio, 1 / portViewScalingRatio) ;
		scale( portViewScalingRatio, portViewScalingRatio ) ;
	}
	else
	{
		portViewScalingRatio = 1 ;
	}
}

void GraphView::ajustStatsPanelSize(void)
{
	QSizeF stat_panel_size, view_size ;

	if( statsPanelItem )
	{
		view_size = size() ;
		stat_panel_size = statsPanelItem->size() ;

		statsPanelScaleRatio = qMin(view_size.width() / stat_panel_size.width(),
				view_size.height() / stat_panel_size.height()) / 4 ;

		if( statsPanelScaleRatio < 1 )
		{
			//TODO
			if( portViewScalingRatio < 1 ) 	statsPanelItem->scale(1 / portViewScalingRatio, 1 / portViewScalingRatio) ;
			statsPanelItem->scale(statsPanelScaleRatio, statsPanelScaleRatio) ;
		}

		isAjustedStatsPanelSize = true ;
		setStatsPanelPos() ;
	}
}

void GraphView::capture(void)
{
	QString file_name;
	QFileInfo file_info ;
	QPixmap pixmap( size() ) ;
	QPainter painter( &pixmap ) ;

	file_name= QFileDialog::getSaveFileName(this, "Select the image destination - " + APP_SHORT_NAME, ".", tr("PNG files (*.png)"));
	file_info.setFile(file_name) ;

	if(file_info.suffix() == "" )
	{
		file_name += ".png" ;
	}
	setBackgroundBrush( Qt::white ) ;
	painter.setRenderHint(QPainter::Antialiasing) ;
	render( &painter ) ;
	painter.end() ;
	pixmap.save( file_name ) ;
}
