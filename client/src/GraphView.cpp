/*
 * GraphView.cpp
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

#include "GraphView.hpp"
#include "StatsLegend.hpp"
#include "DashboardBase.hpp"
#include "utilsCore.hpp"
#include <memory>
#include <QGraphicsProxyWidget>
#include <QScrollBar>
#include <QFileDialog>

const QString GraphView::LABEL_NODE = ":LABEL";
const QString GraphView::ICON_NODE = ":ICON";
const QString GraphView::EXPICON_NODE = ":EXPICON";

GraphView::GraphView(CoreDataT* _cdata, QWidget* _parent)
  : QGraphicsView(_parent),
    m_cdata(_cdata),
    m_scene(new QGraphicsScene()),
    m_icons(ngrt4n::nodeIcons()),
    m_mapScalFactor(1),
    m_chartScalFactor(1),
    m_trackingOn(false)
{
  setScene(m_scene);
  addEvents();
}

GraphView::~GraphView()
{
  delete m_scene;
  m_mnodes.clear();
  m_medges.clear();
  m_icons.clear();
}

void GraphView::mouseReleaseEvent(QMouseEvent * _event)
{
  QGraphicsItem* item = nodeAt(_event->pos());
  if (item) {
    if (_event->button() == Qt::RightButton) {
      Q_EMIT rightClickOnItem(item, _event->globalPos());
      return;
    }
    QStringList list  = item->data(0).toString().split(":");
    if (list.length() == 2) {
      QString nodeId = list[0];
      QString sfx = ":" + list[1];
      if (sfx == EXPICON_NODE) {
        QPixmap exp_icon;
        if (m_mnodes[nodeId].expand) {
          exp_icon.load(m_icons[ngrt4n::MINUS], 0, Qt::AutoColor);
          m_mnodes[nodeId].expand = false;
        } else {
          exp_icon.load(m_icons[ngrt4n::PLUS], 0, Qt::AutoColor);
          m_mnodes[nodeId].expand = true;
        }
        m_mnodes[nodeId].exp_icon->setPixmap(exp_icon);
        Q_EMIT expandNode(nodeId, m_mnodes[nodeId].expand, 1);
      }
    }
  }
}

void GraphView::mouseDoubleClickEvent(QMouseEvent * _event)
{
  QPointF pos = mapToScene(QPoint(_event->pos()));
  QGraphicsItem* item = m_scene->itemAt(pos, QTransform());
  if (item) {
    centerOn(pos);
  }
}

void GraphView::scrollBy(int dx, int dy)
{
  horizontalScrollBar()->setValue(horizontalScrollBar()->value() + dx);
  verticalScrollBar()->setValue(verticalScrollBar()->value() + dy);
}


void GraphView::mouseMoveEvent(QMouseEvent * event)
{
  if (event->buttons() == Qt::LeftButton) {
    if (! m_trackingOn) {
      m_lastTrackingPos = event->pos();
      m_trackingOn = true;
    } else {
      QPoint pos = event->pos();
      QPoint dt = pos - m_lastTrackingPos;
      scrollBy(dt.x(), dt.y());
      m_lastTrackingPos = pos;
    }
  } else {
    m_trackingOn = false;
  }
}

void GraphView::zoomIn()
{
  QGraphicsView::scale(ngrt4n::SCALIN_FACTOR, ngrt4n::SCALIN_FACTOR);
}

void GraphView::zoomOut()
{
  QGraphicsView::scale(ngrt4n::SCALOUT_FACTOR, ngrt4n::SCALOUT_FACTOR);
}

void GraphView::centerOnNode(const QString& id)
{
  if (!id.isEmpty()) {
    centerOn(m_mnodes[id].label);
  }
}

void GraphView::drawMap(void)
{
  // Draw bp nodes
  for (NodeListT::Iterator node=m_cdata->bpnodes.begin(), end = m_cdata->bpnodes.end();
       node != end; ++node) {
    m_mnodes[node->id].type = node->type;
    m_mnodes[node->id].expand = true;
    drawNode(*node);
  }

  // Draw check nodes
  for (NodeListT::Iterator node=m_cdata->cnodes.begin(), end = m_cdata->cnodes.end();
       node != end; ++node) {
    m_mnodes[node->id].type = node->type;
    m_mnodes[node->id].expand = true;
    drawNode(*node);
  }

  // Draw edges
  for (StringListT::Iterator edge = m_cdata->edges.begin(), end = m_cdata->edges.end();
       edge != end; ++edge) {
    drawEdge(edge.key(), edge.value());
  }

  m_scene->setSceneRect(m_scene->itemsBoundingRect());
}

void GraphView::drawNode(const NodeT& _node)
{
  QPixmap icon, expIcon;
  icon.load(m_icons[_node.icon], 0, Qt::AutoColor);
  expIcon.load(m_icons[ngrt4n::PLUS], 0, Qt::AutoColor);
  //FIXME: take care with background color
  QString label = "<span style=\"background: '#F8F8FF'\">&nbsp;"%_node.name%"&nbsp;</span>";
  QString nodeData = _node.id%LABEL_NODE;

  m_mnodes[_node.id].label = new QGraphicsTextItem(),
      m_mnodes[_node.id].label->setHtml(label),
      m_mnodes[_node.id].label->setData(0, nodeData),
      m_scene->addItem(m_mnodes[_node.id].label),
      m_mnodes[_node.id].label->setZValue(-5);
  nodeData = _node.id%ICON_NODE;
  m_mnodes[_node.id].icon = new QGraphicsPixmapItem(icon),
      m_mnodes[_node.id].icon->setData(0, nodeData),
      m_scene->addItem(m_mnodes[_node.id].icon),
      m_mnodes[_node.id].icon->setZValue(-10);
  nodeData = _node.id%EXPICON_NODE;
  m_mnodes[_node.id].exp_icon = new QGraphicsPixmapItem(expIcon),
      m_mnodes[_node.id].exp_icon->setData(0, nodeData),
      m_scene->addItem(m_mnodes[_node.id].exp_icon),
      m_mnodes[_node.id].exp_icon->setZValue(0);
  if (m_mnodes[_node.id].type == NodeType::AlarmNode)
    m_mnodes[_node.id].exp_icon->setVisible(false);

  QString msg = ngrt4n::generateToolTip(_node);
  m_mnodes[_node.id].icon->setToolTip(msg);
  m_mnodes[_node.id].label->setToolTip(msg);

  setNodePos(_node.id, QPointF(_node.pos_x, _node.pos_y));
}

void GraphView::setNodePos(const QString& _nodeId, const QPointF& _pos)
{
  QSizeF l_size, i_size, ei_size;
  qreal xi, xl, yi, yl, xei, yei;
  l_size = m_mnodes[_nodeId].label->boundingRect().size();
  i_size = m_mnodes[_nodeId].icon->boundingRect().size();
  ei_size = m_mnodes[_nodeId].exp_icon->boundingRect().size();
  xi = _pos.x() + 0.5 * (l_size.width() - i_size.width());
  yi = _pos.y();
  xei = _pos.x() + 0.5 * (l_size.width() - ei_size.width());
  xl = _pos.x();
  yl = _pos.y() + 0.75 * i_size.height();
  yei = yl + 0.75 * l_size.height();
  m_mnodes[_nodeId].label->setPos(xl, yl);
  m_mnodes[_nodeId].icon->setPos(xi, yi);
  m_mnodes[_nodeId].exp_icon->setPos(xei, yei);
}


void GraphView::setNodeVisible(const QString& _nodeId,
                               const QString& _parent,
                               const bool& _visible, const qint32& _level)
{
  GNodeListT::iterator gnode = m_mnodes.find(_nodeId);
  if (gnode != m_mnodes.end()) {
    QString edgeId = _parent + ":" + _nodeId;
    m_medges[edgeId].edge->setVisible(_visible);
    gnode->expand = _visible;
    gnode->label->setVisible(_visible);
    gnode->icon->setVisible(_visible);
    if (gnode->type == NodeType::ServiceNode)
      gnode->exp_icon->setVisible(_visible);
    if (_visible) {
      QPixmap expandIcon(m_icons[ngrt4n::PLUS], 0, Qt::AutoColor);
      m_mnodes[_nodeId].exp_icon->setPixmap(expandIcon);
    }
    Q_EMIT expandNode(_nodeId, _visible, _level + 1);
    repaint();
  }
}


void GraphView::drawEdge(const QString& _headNodeId, const QString& _tailNodeId)
{
  QString eid =  QString("%1:%2").arg(_headNodeId, _tailNodeId);
  QPainterPath path;
  setEdgePath(_headNodeId, _tailNodeId, path);
  QPen pen(ngrt4n::COLOR_UNKNOWN);
  m_medges[eid].edge = new QGraphicsPathItem(path),
      m_medges[eid].edge->setPen(pen),
      m_scene->addItem(m_medges[eid].edge),
      m_medges[eid].edge->setZValue(-20);
}

void GraphView::setEdgePath(const QString& _parentVertex,
                            const QString& _childVertex,
                            QPainterPath& path)
{
  GNodeT& p_gnode = m_mnodes[_parentVertex];
  GNodeT& c_gnode = m_mnodes[_childVertex];
  QSizeF p_size = p_gnode.exp_icon->boundingRect().size();
  QSizeF c_size = c_gnode.icon->boundingRect().size();
  QPointF parentAnchor = p_gnode.exp_icon->pos() + QPointF(0.5 * p_size.width(), p_size.height());
  QPointF childAnchor = c_gnode.icon->pos() + QPointF(0.5 * c_size.width(), 0);
  path.moveTo(parentAnchor), path.lineTo(childAnchor);
  if (! p_gnode.exp_icon->isVisible()) p_gnode.exp_icon->setVisible(true);
}

void GraphView::updateNode(const NodeListT::iterator& _node, const QString& _toolTip)
{
  updateNode(*_node, _toolTip);
}

void GraphView::updateNode(const NodeT& _node, const QString& _toolTip)
{
  QString label = "<span style=\"background: '"%ngrt4n::severityQColor(_node.sev).name()
      %"'\">&nbsp;" %_node.name%"&nbsp;</span>";
  GNodeListT::iterator gnodeIt =  m_mnodes.find(_node.id);
  if (gnodeIt != m_mnodes.end()) {
    gnodeIt->label->setHtml(label);
    gnodeIt->icon->setToolTip(_toolTip);
    gnodeIt->label->setToolTip(_toolTip);
    GEdgeListT::iterator edge = m_medges.find(_node.parent + ":" + _node.id);
    if (edge != m_medges.end())
      edge->edge->setPen(ngrt4n::severityQColor(_node.sev_prop));
  }
}


void GraphView::scaleToFitViewPort(void)
{
  QSizeF viewSize = size();
  QSizeF sceneSize = m_scene->itemsBoundingRect().size();
  m_mapScalFactor = qMin(viewSize.width()/sceneSize.width(), viewSize.height()/sceneSize.height()) ;
  if (m_mapScalFactor < 1.0) {
    scale(m_mapScalFactor, m_mapScalFactor);
  } else {
    m_mapScalFactor = 1;
  }
}


void GraphView::capture(void)
{
  QPixmap pixmap(size());
  QPainter painter(&pixmap);
  QString fileName= QFileDialog::getSaveFileName(this,
                                                 tr("Select the image destination - %1").arg(APP_NAME),
                                                 ".",
                                                 tr("PNG files (*.png);; All files (*)"));
  QFileInfo fileInfo(fileName);
  if (fileInfo.suffix().isEmpty()) fileName.append(".png");

  setBackgroundBrush(Qt::white);
  painter.setRenderHint(QPainter::Antialiasing);
  render(&painter);
  painter.end();
  pixmap.save(fileName);
}

void GraphView::addEvents(void)
{
}
