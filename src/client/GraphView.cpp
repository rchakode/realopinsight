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

const qreal GraphView::XSCAL_FACTOR = 72.0;
const qreal GraphView::YSCAL_FACTOR = 100.0;
const QString GraphView::LABEL_NODE = ":LABEL";
const QString GraphView::ICON_NODE = ":ICON";
const QString GraphView::EXPICON_NODE = ":EXPICON";
const QString GraphView::PLUS = "plus";
const QString GraphView::MINUS = "minus";
const QString GraphView::DEFAULT_ICON = NodeType::toString(NodeType::SERVICE_NODE);
const float SCALIN_FACTOR = 1.1;
const float SCALOUT_FACTOR = 1/SCALIN_FACTOR;

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
  icons["Network Bandwith"] = ":/images/network-usage.png";
  icons["CPU"] = ":/images/cpu.png";
  icons["CPU Load"] = ":/images/performance-level.png";
  icons["Memory"] = ":/images/memory.png";
  icons["Memory Usage"] = ":/images/memory-usage.png";
  icons["Resource Utilization"] = ":/images/resource-usage.png";
  icons["Performance"] = ":/images/performance.png";
  return icons;
}

GraphView::GraphView(QWidget* _parent)
  : QGraphicsView(_parent),
    mchart (0),
    mviewScalFactor (1),
    mchartScalFactor (1),
    misAjustedChartSize (false)
{
  mchart = NULL;
  mscene = new QGraphicsScene() , setScene(mscene);
  micons = nodeIcons();
  micons[PLUS] = ":/images/plus.png";
  micons[MINUS] = ":/images/minus.png";
}

GraphView::~GraphView()
{
  delete mscene;
  mgnodes.clear();
  medges.clear();
  micons.clear();
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
              if (mgnodes[nodeId].expand) {
                  exp_icon.load(micons[MINUS], 0, Qt::AutoColor);
                  mgnodes[nodeId].expand = false;
                } else {
                  exp_icon.load(micons[PLUS], 0, Qt::AutoColor);
                  mgnodes[nodeId].expand = true;
                }
              mgnodes[nodeId].exp_icon->setPixmap(exp_icon);
              emit expandNode(nodeId, mgnodes[nodeId].expand, 1);
            }
        }
    }
}

void GraphView::mouseDoubleClickEvent(QMouseEvent * _event)
{
  QPointF pos = mapToScene(QPoint(_event->pos()));
  QGraphicsItem* item = mscene->itemAt(pos);
  if (item) centerOn(pos);

}

void GraphView::zoomIn()
{
  scale(SCALIN_FACTOR, SCALIN_FACTOR);
  if (mchart) {
      setStatsPanelPos();
      mchart->scale(SCALOUT_FACTOR, SCALOUT_FACTOR);
    }
}

void GraphView::zoomOut()
{
  scale(SCALOUT_FACTOR, SCALOUT_FACTOR);
  if (mchart) {
      mchart->scale(SCALIN_FACTOR, SCALIN_FACTOR);
      setStatsPanelPos();
    }
}

void GraphView::updateStatsPanel(Chart * _statsPanel)
{
  bool visible = true;
  if (mchart) {
      visible = mchart->isVisible();
      mchart->setWidget(_statsPanel);
      mchart->setVisible(visible);
    } else {
      mchart = mscene->addWidget(_statsPanel);
      mchartArea = new QGraphicsRectItem();
      mchartArea->setBrush(Qt::transparent);
      mchartArea->setPen(QColor(Qt::transparent));
      mscene->addItem(mchartArea);
    }

  if (mchart) { //Mandatory
      setStatsPanelPos();
      if (!misAjustedChartSize) ajustStatsPanelSize();
    }
}


void GraphView::ajustStatsPanelSize(void)
{
  if (mchart) {
      QSizeF viewSize = size();
      QSizeF statPanelSize = mchart->size();
      mchartScalFactor = qMin(viewSize.width()/statPanelSize.width(), viewSize.height()/statPanelSize.height())/4;
      if (mchartScalFactor < 1) {
          if (mviewScalFactor < 1) 	mchart->scale(1/mviewScalFactor, 1/mviewScalFactor);
          mchart->scale(mchartScalFactor, mchartScalFactor);
        }
      misAjustedChartSize = true;
      setStatsPanelPos();
    }
}

void GraphView::setStatsPanelPos(void)
{
  if (mchart) {
      qreal xp = size().width() - mchart->size().width() *  mchartScalFactor - 2;
      QPointF pos = mapToScene(QPoint(xp, 0));
      mchart->setPos(pos);
      Chart* w = dynamic_cast<Chart*>(mchart->widget());
      mchartArea->setRect(w->x(), w->y(), Chart::DefaultWidth, Chart::DefaultHeight);
    }
}

bool GraphView::hideChart(void)
{
  bool visible = mchart->isVisible();
  mchart->setVisible(! visible);
  return visible;
}


void GraphView::load(const QString& _dotFile,
                     const NodeListT& _bpnodes,
                     const NodeListT& _cnodes)
{
  auto dotParser = std::unique_ptr<QProcess>(new QProcess());
  mgphCoordFile = _dotFile%".plain";
  QStringList arguments = QStringList() << "-Tplain"<< "-o" << mgphCoordFile << _dotFile;
  int exitCode = dotParser->execute("dot", arguments);
  dotParser->waitForFinished(60000);
  if (!exitCode) {
      drawMap(_bpnodes, _cnodes);
      mscene->setSceneRect(mscene->itemsBoundingRect());
    } else {
      utils::alert(tr("The graph engine exited with the code %1").arg(exitCode));
      exit(exitCode);
    }

  dotParser.reset(NULL);
}

void GraphView::drawMap(const NodeListT& _bpnodes, const NodeListT& _cnodes)
{
  QString line;
  QRegExp regexSep;
  QStringList splitedLine;
  QPen pen;
  QFile coodFile(mgphCoordFile);
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
              NodeListT::const_iterator node;
              if (!utils::findNode(_bpnodes, _cnodes, nid, node)) continue;
              mgnodes[nid].type = node->type;
              mgnodes[nid].expand = true;
              QPointF labelOrigin = QPointF(x_corner * XSCAL_FACTOR, y_corner * YSCAL_FACTOR);
              drawNode(*node);
              setNodePos(nid, labelOrigin);
            } else if (splitedLine[0] == "edge") {
              QPainterPath path;
              setEdgePath(splitedLine[1], splitedLine[2], path);
              QString eid =  splitedLine[1]%":"%splitedLine[2];
              medges[eid].edge = new QGraphicsPathItem(path),
                  medges[eid].edge->setPen(pen),
                  mscene->addItem(medges[eid].edge),
                  medges[eid].edge->setZValue(-20);
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
  icon.load(micons[_node.icon], 0, Qt::AutoColor);
  expIcon.load(micons[PLUS], 0, Qt::AutoColor);
  //FIXME: take care with background color
  QString label = "<span style=\"background: '#F8F8FF'\">&nbsp;"%_node.name%"&nbsp;</span>";
  QString nodeData = _node.id%LABEL_NODE;

  mgnodes[_node.id].label = new QGraphicsTextItem(),
      mgnodes[_node.id].label->setHtml(label),
      mgnodes[_node.id].label->setData(0, nodeData),
      mscene->addItem(mgnodes[_node.id].label),
      mgnodes[_node.id].label->setZValue(-5);
  nodeData = _node.id%ICON_NODE;
  mgnodes[_node.id].icon = new QGraphicsPixmapItem(icon),
      mgnodes[_node.id].icon->setData(0, nodeData),
      mscene->addItem(mgnodes[_node.id].icon),
      mgnodes[_node.id].icon->setZValue(-10);
  nodeData = _node.id%EXPICON_NODE;
  mgnodes[_node.id].exp_icon = new QGraphicsPixmapItem(expIcon),
      mgnodes[_node.id].exp_icon->setData(0, nodeData),
      mscene->addItem(mgnodes[_node.id].exp_icon),
      mgnodes[_node.id].exp_icon->setZValue(0);
  if (mgnodes[_node.id].type == NodeType::ALARM_NODE)
    mgnodes[_node.id].exp_icon->setVisible(false);

  QString msg =  SvNavigator::getNodeToolTip(_node);
  mgnodes[_node.id].icon->setToolTip(msg);
  mgnodes[_node.id].label->setToolTip(msg);
}

void GraphView::updateNode(const NodeListT::iterator& _node, const QString& _toolTip)
{
  updateNode(*_node, _toolTip);
}

void GraphView::updateNode(const NodeT& _node, const QString& _toolTip)
{
  QString label = "<span style=\"background: '"%utils::computeColor(_node.severity).name()
      %"'\">&nbsp;" %_node.name%"&nbsp;</span>";
  GNodeListT::iterator gnodeIt =  mgnodes.find(_node.id);
  if (gnodeIt != mgnodes.end()) {
      gnodeIt->label->setHtml(label);
      gnodeIt->icon->setToolTip(_toolTip);
      gnodeIt->label->setToolTip(_toolTip);
      GEdgeListT::iterator edge = medges.find(_node.parent + ":" + _node.id);
      if (edge != medges.end())
        edge->edge->setPen(utils::computeColor(_node.prop_sev));
    }
}

void GraphView::setEdgePath(const QString& _parentVertex,
                            const QString& _childVertex,
                            QPainterPath& path)
{
  QPointF parentAnchor, childAnchor;
  QSizeF p_size, c_size;
  _GNode& p_gnode = mgnodes[_parentVertex];
  _GNode& c_gnode = mgnodes[_childVertex];
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
  l_size = mgnodes[_nodeId].label->boundingRect().size();
  i_size = mgnodes[_nodeId].icon->boundingRect().size();
  ei_size = mgnodes[_nodeId].exp_icon->boundingRect().size();
  xi = _pos.x() + 0.5 * (l_size.width() - i_size.width());
  yi = _pos.y();
  xei = _pos.x() + 0.5 * (l_size.width() - ei_size.width());
  xl = _pos.x();
  yl = _pos.y() + 0.75 * i_size.height();
  yei = yl + 0.75 * l_size.height();
  mgnodes[_nodeId].label->setPos(xl, yl);
  mgnodes[_nodeId].icon->setPos(xi, yi);
  mgnodes[_nodeId].exp_icon->setPos(xei, yei);
}


void GraphView::setNodeVisible(const QString& _nodeId,
                               const QString& _parent,
                               const bool& _visible, const qint32& _level)
{
  GNodeListT::iterator gnode = mgnodes.find(_nodeId);
  if (gnode != mgnodes.end()) {
      QString edgeId = _parent + ":" + _nodeId;
      medges[edgeId].edge->setVisible(_visible);
      gnode->expand = _visible;
      gnode->label->setVisible(_visible);
      gnode->icon->setVisible(_visible);
      if (gnode->type == NodeType::SERVICE_NODE)
        gnode->exp_icon->setVisible(_visible);
      if (_visible) {
          QPixmap expandIcon(micons[PLUS], 0, Qt::AutoColor);
          mgnodes[_nodeId].exp_icon->setPixmap(expandIcon);
        }
      emit expandNode(_nodeId, _visible, _level + 1);
    }
}

void GraphView::scaleToFitViewPort(void)
{
  QSizeF viewSize = size();
  QSizeF sceneSize = mscene->itemsBoundingRect().size();
  mviewScalFactor = qMin(viewSize.width()/sceneSize.width(), viewSize.height()/sceneSize.height()) ;
  if (mviewScalFactor < 1.0) {
      if (mchart)
        mchart->scale(1 / mviewScalFactor, 1 / mviewScalFactor);
      scale(mviewScalFactor, mviewScalFactor);
    } else {
      mviewScalFactor = 1;
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
