/*
 * GraphView.hpp
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

#ifndef GRAPHVIEW_H_
#define GRAPHVIEW_H_

#include "Base.hpp"
#include "Stats.hpp"
#include "SvNavigatorTree.hpp"


class GraphView : public QGraphicsView
{
	Q_OBJECT

public:
	GraphView(QWidget* = 0 ) ;
	virtual ~GraphView() ;

	bool load( const QString&, const NodeListT& ) ;
	void updateNode(const NodeListT::iterator &, const QString & _tool_tip) ;
	void updateNodeColor(const NodeListT::iterator & ) ;
	void setNodeToolTip( const NodeT &  ) ;
	void setNodeVisible( const QString &, const QString &, const bool &, const qint32 &) ;
	void scaleToFitViewPort(void) ;
	void updateStatsPanel(Stats * _stats_panel, const QString &  ) ;
	void setStatsPanelPos(void) ;

	inline void centerOnNode( const QString & _node_id ) {
		if (! _node_id.isEmpty() ) centerOn( gnodesList[_node_id].label ) ;
	}

	inline QGraphicsItem* nodeAtGlobalPos(QPoint pos){
		return graphScene->itemAt( mapToScene( mapFromGlobal(pos) ) ) ;
	}

	inline QGraphicsItem* nodeAt(QPoint pos){
		return graphScene->itemAt( mapToScene( pos ) ) ;
	}


public slots:
void capture(void) ;
void zoomIn();
void zoomOut();
bool hideChart(void) ;

signals:
void mouseIsOverNode( QString ) ;
void expandNode( QString, bool, qint32 ) ;
void rightClickOnItem( QGraphicsItem *, QPoint pos ) ;


protected:
void mouseReleaseEvent( QMouseEvent * ) ;
void mouseDoubleClickEvent( QMouseEvent * ) ;
void wheelEvent( QWheelEvent * ) ;
void resizeEvent ( QResizeEvent * ) ;
void showEvent(QShowEvent * ) ;
void scrollContentsBy ( int dx, int dy ) ;


private:

QGraphicsScene* graphScene ;
QGraphicsProxyWidget* statsPanelItem ;
QPoint statsPanelPos ;

QString svgGraphFile ;
QString coodinatesGraphFile ;

GNodeListT gnodesList ;
GEdgeListT edgesList ;

IconMapT iconMap ;

static const qreal XScalingRatio ;
static const qreal YScalingRatio ;
qreal portViewScalingRatio ;
qreal statsPanelScaleRatio ;
bool isAjustedStatsPanelSize ;

void drawGraph(const NodeListT &) ;
void drawNode(const NodeT & ) ;
void setEdgePath(const QString &, const QString &, QPainterPath & ) ;
void setNodePos(const QString & , const QPointF & ) ;
void ajustStatsPanelSize(void) ;
};

#endif /* GRAPHVIEW_H_ */
