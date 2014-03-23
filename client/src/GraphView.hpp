/*
# GraphView.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-03-2014                                                  #
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

#ifndef GRAPHVIEW_HPP
#define GRAPHVIEW_HPP

#include "Base.hpp"
#include "Chart.hpp"
#include "SvNavigatorTree.hpp"

class GraphView : public QGraphicsView
{
  Q_OBJECT

public:
  GraphView(CoreDataT* _cdata, QWidget* = 0);
  virtual ~GraphView();

  void drawMap(void);
  void setNodeVisible(const QString& _nodeId,
                      const QString& _parent,
                      const bool& _visible,
                      const qint32& _level);
  void updateNode(const NodeListT::iterator& _nodeIt, const QString& _toolTip);
  void updateNode(const NodeT& _nodeIt, const QString& _toolTip);
  void scaleToFitViewPort(void);
  void setChartPos(void);
  void updateStatsPanel(Chart * _stats_panel);
  void centerOnNode(const QString& id);
  QGraphicsItem* nodeAtGlobalPos(QPoint pos) { return m_scene->itemAt(mapToScene(mapFromGlobal(pos))); }
  QGraphicsItem* nodeAt(QPoint pos) { return m_scene->itemAt(mapToScene(pos)); }

public Q_SLOTS:
  void capture(void);
  void zoomIn();
  void zoomOut();
  bool hideChart(void);
  void handleScrollBarMoved(void);

Q_SIGNALS:
  void mouseIsOverNode(QString);
  void expandNode(QString, bool, qint32);
  void rightClickOnItem(QGraphicsItem *, QPoint pos);

protected:
  virtual void mouseReleaseEvent(QMouseEvent *);
  virtual void mouseDoubleClickEvent(QMouseEvent *);
  virtual void wheelEvent(QWheelEvent * _event) { (_event->delta() > 0)? zoomIn() : zoomOut();}
  virtual void resizeEvent(QResizeEvent *) { setChartPos(); }
  virtual void showEvent(QShowEvent *) { setChartPos(); }
  virtual void scrollBy(int dx, int dy);
  virtual void mouseMoveEvent(QMouseEvent * event);

private:
  static const QString LABEL_NODE;
  static const QString  ICON_NODE;
  static const QString  EXPICON_NODE;

  CoreDataT* m_cdata;
  QGraphicsScene* m_scene;
  QGraphicsProxyWidget* m_chart;
  QGraphicsRectItem* m_chartArea;
  QString m_mcoordFile;
  GNodeListT m_mnodes;
  GEdgeListT m_medges;
  IconMapT m_icons;
  qreal m_mapScalFactor;
  qreal m_chartScalFactor;
  bool m_isAjustedChartSize;
  QPoint m_lastTrackingPos;
  bool m_trackingOn;

  void drawNode(const NodeT&);
  void setNodePos(const QString& _nodeId, const QPointF& _pos);
  void drawEdge(const QString& _headNodeId, const QString& _tailNodeId);
  void setEdgePath(const QString& _parentVertex, const QString& _childVertex, QPainterPath& path);
  void ajustStatsPanelSize(void);
  void addEvents(void);
};

#endif /* GRAPHVIEW_HPP */
