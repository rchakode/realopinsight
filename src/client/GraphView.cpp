/*
 * GraphView.cpp
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


#include "core/ns.hpp"
#include "GraphView.hpp"
#include "StatsLegend.hpp"
#include "SvNavigator.hpp"
#include "utilsClient.hpp"
#include <memory>

const qreal GraphView::XScalingRatio = 72.0;
const qreal GraphView::YScalingRatio = 100.0;
const QString GraphView::LABEL_NODE = ":LABEL";
const QString GraphView::ICON_NODE = ":ICON";
const QString GraphView::EXPICON_NODE = ":EXPICON";
const QString GraphView::PLUS = "plus";
const QString GraphView::MINUS = "minus";
const QString GraphView::DEFAULT_ICON = NodeType::toString(NodeType::SERVICE_NODE);

IconMapT GraphView::nodeIcons() {
  IconMapT icons;
  icons[GraphView::DEFAULT_ICON]= ":/images/business-process.png";
  icons["Other Check"] = ":/images/check.png";
  icons["Server"] = ":/images/server.png";
  icons["Firewall"] = ":/images/firewall.png";
  icons["Router"] = ":/images/router.png";
  icons["Network"] = ":/images/network.png";
  icons["Swicth"] = ":/images/switch.png";
  icons["Filer"] = ":/images/filer.png";
  icons["Hard disk"] = ":/images/harddisk.png";
  icons["Storage Area"] = ":/images/storage.png";
  icons["Linux"] = ":/images/linux.png";
  icons["Windows OS"] = ":/images/windows.png";
  icons["Solaris"] = ":/images/solaris.png";
  icons["Cloud"] = ":/images/cloud.png";
  icons["Hypervisor"] = ":/images/hypervisor.png";
  icons["Application"] = ":/images/application.png";
  icons["Web Accessibility"] = ":/images/web.png";
  icons["Web server"] = ":/images/web-server.png";
  icons["Database Engine"] = ":/images/db.png";
  icons["Database Server"] = ":/images/db-server.png";
  icons["Process"] = ":/images/process.png";
  icons["Logfile"] = ":/images/log.png";
  return icons;
}

GraphView::GraphView(QWidget* _parent)
  : QGraphicsView(_parent),
    statsPanelItem (0),
    portViewScalingRatio (1),
    statsPanelScaleRatio (1),
    isAjustedStatsPanelSize (false)
{
  statsPanelItem = NULL;
  graphScene = new QGraphicsScene() , setScene(graphScene);
  iconMap = nodeIcons();
  iconMap[PLUS] = ":/images/plus.png";
  iconMap[MINUS] = ":/images/minus.png";
}

GraphView::~GraphView()
{
  delete graphScene;
  gnodesList.clear();
  edgesList.clear();
  iconMap.clear();
}

void GraphView::mouseReleaseEvent(QMouseEvent * _event)
{
  QGraphicsItem* item = nodeAt(_event->pos());
  if (item) {

      if (_event->button() == Qt::RightButton) {
          emit rightClickOnItem(item, _event->globalPos());
          return;
        }
      QStringList list  = item->data(0).toString().split(":");
      if (list.length() == 2) {
          QString nodeId = list[0];
          QString sfx = ":" + list[1];
          if (sfx == EXPICON_NODE) {
              QPixmap exp_icon;
              if (gnodesList[nodeId].expand) {
                  exp_icon.load(iconMap[MINUS], 0, Qt::AutoColor);
                  gnodesList[nodeId].expand = false;
                } else {
                  exp_icon.load(iconMap[PLUS], 0, Qt::AutoColor);
                  gnodesList[nodeId].expand = true;
                }
              gnodesList[nodeId].exp_icon->setPixmap(exp_icon);
              emit expandNode(nodeId, gnodesList[nodeId].expand, 1);
            }
        }
    }
}

void GraphView::mouseDoubleClickEvent(QMouseEvent * _event)
{
  QPointF pos = mapToScene(QPoint(_event->pos()));
  QGraphicsItem* item = graphScene->itemAt(pos);
  if (item) {
      centerOn(pos);
    }
}


void GraphView::wheelEvent(QWheelEvent * _event)
{
  if (_event->delta() > 0) {
      zoomIn();
    } else {
      zoomOut();
    }
}


void GraphView::resizeEvent (QResizeEvent *)
{
  setStatsPanelPos();
}

void GraphView::showEvent(QShowEvent *)
{
  setStatsPanelPos();
}

void GraphView::scrollContentsBy (int dx, int dy)
{
  QGraphicsView::scrollContentsBy (dx, dy);
  setStatsPanelPos();
}

void GraphView::zoomIn()
{
  scale(1.1, 1.1);
  if (statsPanelItem) {
      setStatsPanelPos();
      statsPanelItem->scale(1/1.1, 1/1.1);
    }
}

void GraphView::zoomOut()
{
  scale(1/1.1, 1/1.1);
  if (statsPanelItem) {
      statsPanelItem->scale(1.1, 1.1);
      setStatsPanelPos();
    }
}

void GraphView::updateStatsPanel(Chart * _statsPanel)
{
  bool visible = true;
  if (statsPanelItem) {
      visible = statsPanelItem->isVisible();
      statsPanelItem->setWidget(_statsPanel);
      statsPanelItem->setVisible(visible);
    } else {
      statsPanelItem = graphScene->addWidget(_statsPanel);
      statsArea = new QGraphicsRectItem();
      statsArea->setBrush(Qt::transparent);
      statsArea->setPen(QColor(Qt::transparent));
      graphScene->addItem(statsArea);
    }

  if (statsPanelItem) { //Mandatory
      setStatsPanelPos();
      if (!isAjustedStatsPanelSize) ajustStatsPanelSize();
    }
}


void GraphView::ajustStatsPanelSize(void)
{
  if (statsPanelItem) {
      QSizeF viewSize = size();
      QSizeF statPanelSize = statsPanelItem->size();
      statsPanelScaleRatio = qMin(viewSize.width()/statPanelSize.width(), viewSize.height()/statPanelSize.height())/4;
      if (statsPanelScaleRatio < 1) {
          if (portViewScalingRatio < 1) 	statsPanelItem->scale(1/portViewScalingRatio, 1/portViewScalingRatio);
          statsPanelItem->scale(statsPanelScaleRatio, statsPanelScaleRatio);
        }
      isAjustedStatsPanelSize = true;
      setStatsPanelPos();
    }
}

void GraphView::setStatsPanelPos(void)
{

  if (statsPanelItem) {
      qreal xp = size().width() - statsPanelItem->size().width() *  statsPanelScaleRatio - 2;
      QPointF pos = mapToScene(QPoint(xp, 0));
      statsPanelItem->setPos(pos);
      Chart* w = dynamic_cast<Chart*>(statsPanelItem->widget());
      statsArea->setRect(w->x(), w->y(), Chart::DefaultWidth, Chart::DefaultHeight);
    }
}


bool GraphView::hideChart(void)
{
  bool visible = statsPanelItem->isVisible();
  statsPanelItem->setVisible(! visible);
  return visible;
}


bool GraphView::load(const QString& _dotFile,
                     const NodeListT& _bpnodes,
                     const NodeListT& _cnodes)
{

  auto dotParser = std::unique_ptr<QProcess>(new QProcess());
  coodinatesFile = _dotFile%".plain";
  QStringList arguments = QStringList() << "-Tplain"<< "-o" << coodinatesFile << _dotFile;
  int exitCode = dotParser->execute("dot", arguments);
  dotParser->waitForFinished(60000);
  if (! exitCode) {
      drawMap(_bpnodes, _cnodes);
      graphScene->setSceneRect(graphScene->itemsBoundingRect());
    } else {
      utils::alert(tr("The graph engine exited with the code %1").arg(exitCode));
      qDebug() << tr("The graph engine exited with the code %1").arg(exitCode);
      exit(exitCode);
    }

  dotParser.reset(nullptr);
  return false;
}

void GraphView::drawMap(const NodeListT& _bpnodes, const NodeListT& _cnodes)
{
  QString line;
  QRegExp regexSep;
  QStringList splitedLine;
  QPen pen;
  QFile coodFile(coodinatesFile);
  if (coodFile.open(QFile::ReadOnly)) {
      pen.setColor(StatsLegend::COLOR_UNKNOWN);
      regexSep.setPattern("[ ]+");
      QTextStream coodFileStream(& coodFile);
      while (line = coodFileStream.readLine(0), ! line.isNull()) {
          splitedLine = line.split (regexSep);
          if (splitedLine[0] == "node") {
              QString nid = splitedLine[1].trimmed();
              qreal x_corner = splitedLine[2].trimmed().toFloat();
              qreal y_corner = -1 * splitedLine[3].trimmed().toFloat();
              NodeListT::const_iterator nodeIt = _bpnodes.find(nid);
              if (nodeIt == _bpnodes.end()) {
                  nodeIt = _cnodes.find(nid);
                  if (nodeIt == _cnodes.end()) {
                      continue;
                    }
                }
              gnodesList[nid].type = nodeIt->type;
              gnodesList[nid].expand = true;
              QPointF labelOrigin = QPointF(x_corner * XScalingRatio, y_corner * YScalingRatio);
              drawNode(*nodeIt);
              setNodePos(nid, labelOrigin);
            } else if (splitedLine[0] == "edge") {
              QString edge_name; 	QPainterPath path;
              setEdgePath(splitedLine[1], splitedLine[2], path);
              edge_name =  splitedLine[1] + ":" + splitedLine[2];
              edgesList[edge_name].edge = new QGraphicsPathItem(path),
                  edgesList[edge_name].edge->setPen(pen),
                  graphScene->addItem(edgesList[edge_name].edge),
                  edgesList[edge_name].edge->setZValue(-20);
            } else if (splitedLine[0] == "stop") {
              break;
            }
        }
      coodFile.close();
    }
}

void GraphView::drawNode(const NodeT& _node)
{
  QPixmap icon, expIcon;
  icon.load(iconMap[_node.icon], 0, Qt::AutoColor);  // TODO
  expIcon.load(iconMap[PLUS], 0, Qt::AutoColor);
  QString label = "<span style=\"background: '#F8F8FF'\">&nbsp;"%_node.name%"&nbsp;</span>";
  QString nodeData = _node.id%LABEL_NODE;

  gnodesList[_node.id].label = new QGraphicsTextItem(),
      gnodesList[_node.id].label->setHtml(label),
      gnodesList[_node.id].label->setData(0, nodeData),
      graphScene->addItem(gnodesList[_node.id].label),
      gnodesList[_node.id].label->setZValue(-5);
  nodeData = _node.id%ICON_NODE;
  gnodesList[_node.id].icon = new QGraphicsPixmapItem(icon),
      gnodesList[_node.id].icon->setData(0, nodeData),
      graphScene->addItem(gnodesList[_node.id].icon),
      gnodesList[_node.id].icon->setZValue(-10);
  nodeData = _node.id%EXPICON_NODE;
  gnodesList[_node.id].exp_icon = new QGraphicsPixmapItem(expIcon),
      gnodesList[_node.id].exp_icon->setData(0, nodeData),
      graphScene->addItem(gnodesList[_node.id].exp_icon),
      gnodesList[_node.id].exp_icon->setZValue(0);
  if (gnodesList[_node.id].type == NodeType::ALARM_NODE)
    gnodesList[_node.id].exp_icon->setVisible(false);

  QString msg =  SvNavigator::getNodeToolTip(_node);
  gnodesList[_node.id].icon->setToolTip(msg);
  gnodesList[_node.id].label->setToolTip(msg);
}

void GraphView::updateNode(const NodeListT::iterator& _node, const QString& _toolTip)
{
  updateNode(*_node, _toolTip);
}

void GraphView::updateNode(const NodeT& _node, const QString& _toolTip)
{
  QString label = "<span style=\"background: '"%utils::computeColor(_node.criticity).name()
      %"'\">&nbsp;" %_node.name%"&nbsp;</span>";
  GNodeListT::iterator gnodeIt =  gnodesList.find(_node.id);
  if (gnodeIt != gnodesList.end()) {
      gnodeIt->label->setHtml(label);
      gnodeIt->icon->setToolTip(_toolTip);
      gnodeIt->label->setToolTip(_toolTip);
      GEdgeListT::iterator edge = edgesList.find(_node.parent + ":" + _node.id);
      if (edge != edgesList.end())
        edge->edge->setPen(utils::computeColor(_node.prop_criticity));
    }
}

void GraphView::setEdgePath(const QString& _parentVertex,
                            const QString& _childVertex,
                            QPainterPath& path)
{
  QPointF parentAnchor, childAnchor;
  QSizeF p_size, c_size;
  _GNode& p_gnode = gnodesList[_parentVertex];
  _GNode& c_gnode = gnodesList[_childVertex];
  p_size = p_gnode.exp_icon->boundingRect().size();
  c_size = c_gnode.icon->boundingRect().size();
  parentAnchor = p_gnode.exp_icon->pos() + QPointF(0.5 * p_size.width(), p_size.height());
  childAnchor = c_gnode.icon->pos() + QPointF(0.5 * c_size.width(), 0);
  path.moveTo(parentAnchor), path.lineTo(childAnchor);
  if (! p_gnode.exp_icon->isVisible()) p_gnode.exp_icon->setVisible(true);
}

void GraphView::setNodePos(const QString& _nodeId, const QPointF& _pos)
{
  QSizeF l_size, i_size, ei_size;
  qreal xi, xl, yi, yl, xei, yei;
  l_size = gnodesList[_nodeId].label->boundingRect().size();
  i_size = gnodesList[_nodeId].icon->boundingRect().size();
  ei_size = gnodesList[_nodeId].exp_icon->boundingRect().size();
  xi = _pos.x() + 0.5 * (l_size.width() - i_size.width());
  yi = _pos.y();
  xei = _pos.x() + 0.5 * (l_size.width() - ei_size.width());
  xl = _pos.x();
  yl = _pos.y() + 0.75 * i_size.height();
  yei = yl + 0.75 * l_size.height();
  gnodesList[_nodeId].label->setPos(xl, yl);
  gnodesList[_nodeId].icon->setPos(xi, yi);
  gnodesList[_nodeId].exp_icon->setPos(xei, yei);
}


void GraphView::setNodeVisible(const QString& _nodeId,
                               const QString& _parent,
                               const bool& _visible, const qint32& _level)
{
  GNodeListT::iterator gnode = gnodesList.find(_nodeId);
  if (gnode != gnodesList.end()) {
      QString edgeId = _parent + ":" + _nodeId;
      edgesList[edgeId].edge->setVisible(_visible);
      gnode->expand = _visible;
      gnode->label->setVisible(_visible);
      gnode->icon->setVisible(_visible);
      if (gnode->type == NodeType::SERVICE_NODE)
        gnode->exp_icon->setVisible(_visible);
      if (_visible) {
          QPixmap expandIcon(iconMap[PLUS], 0, Qt::AutoColor);
          gnodesList[_nodeId].exp_icon->setPixmap(expandIcon);
        }
      emit expandNode(_nodeId, _visible, _level + 1);
    }
}

void GraphView::scaleToFitViewPort(void)
{
  QSizeF viewSize = size();
  QSizeF sceneSize = graphScene->itemsBoundingRect().size();
  portViewScalingRatio = qMin(viewSize.width()/sceneSize.width(), viewSize.height()/sceneSize.height()) ;
  if (portViewScalingRatio < 1.0) {
      if (statsPanelItem)
        statsPanelItem->scale(1 / portViewScalingRatio, 1 / portViewScalingRatio);
      scale(portViewScalingRatio, portViewScalingRatio);
    } else {
      portViewScalingRatio = 1;
    }
}


void GraphView::capture(void)
{
  QPixmap pixmap(size());
  QPainter painter(&pixmap);
  QString fileName= QFileDialog::getSaveFileName(this,
                                                 tr("Select the image destination - ") + AppName,
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
