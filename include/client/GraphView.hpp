/*
 * GraphView.hpp
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

#ifndef GRAPHVIEW_HPP
#define GRAPHVIEW_HPP

#include "Base.hpp"
#include "Chart.hpp"
#include "SvNavigatorTree.hpp"

class GraphView : public QGraphicsView
{
  Q_OBJECT

public:
  static IconMapT nodeIcons();
  static const QString PLUS;
  static const QString MINUS;
  static const QString DEFAULT_ICON;

  GraphView(QWidget* = 0);
  virtual ~GraphView();

  bool load(const QString& _dotFile,
            const NodeListT& _bpnodes,
            const NodeListT& _cnodes);
  void setNodeVisible(const QString& _nodeId,
                      const QString& _parent,
                      const bool& _visible,
                      const qint32& _level);
  void updateNode(const NodeListT::iterator& _nodeIt, const QString& _toolTip);
  void updateNode(const NodeT& _nodeIt, const QString& _toolTip);
  void scaleToFitViewPort(void);
  void setStatsPanelPos(void);
  void updateStatsPanel(Chart * _stats_panel);

  inline void centerOnNode(const QString& id)
  { if (!id.isEmpty())centerOn(mgnodes[id].label); }
  inline QGraphicsItem* nodeAtGlobalPos(QPoint pos)
  { return mscene->itemAt(mapToScene(mapFromGlobal(pos))); }
  inline QGraphicsItem* nodeAt(QPoint pos)
  { return mscene->itemAt(mapToScene(pos)); }

public slots:
  void capture(void);
  void zoomIn();
  void zoomOut();
  bool hideChart(void);

signals:
  void mouseIsOverNode(QString);
  void expandNode(QString, bool, qint32);
  void rightClickOnItem(QGraphicsItem *, QPoint pos);

protected:
  void mouseReleaseEvent(QMouseEvent *);
  void mouseDoubleClickEvent(QMouseEvent *);
  void wheelEvent(QWheelEvent * _event) { (_event->delta() > 0)? zoomIn() : zoomOut();}
  void resizeEvent(QResizeEvent *) {setStatsPanelPos();}
  void showEvent(QShowEvent *) {setStatsPanelPos();}
  void scrollContentsBy(int dx, int dy) {QGraphicsView::scrollContentsBy (dx, dy); setStatsPanelPos();}

private:
  static const qreal XSCAL_FACTOR;
  static const qreal YSCAL_FACTOR;
  static const QString LABEL_NODE;
  static const QString  ICON_NODE;
  static const QString  EXPICON_NODE;

  QGraphicsScene* mscene;
  QGraphicsProxyWidget* mchart;
  QGraphicsRectItem* mchartArea;
  QPoint mchartPos;
  QString msvgGphFile;
  QString mgphCoordFile;
  GNodeListT mgnodes;
  GEdgeListT medges;
  IconMapT micons;
  qreal mviewScalFactor;
  qreal mchartScalFactor;
  bool misAjustedChartSize;

  void setEdgePath(const QString& _parentVertex, const QString& _childVertex, QPainterPath& path);
  void drawMap(const NodeListT &_bpnodes, const NodeListT& _cnodes);
  void drawNode(const NodeT&);
  void setNodePos(const QString& , const QPointF&);
  void ajustStatsPanelSize(void);
};

#endif /* GRAPHVIEW_HPP */
