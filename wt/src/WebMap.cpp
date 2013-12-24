/*
 * WebServiceMap.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 19-09-2013                                                 #
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

#include "WebUtils.hpp"
#include "WebMap.hpp"
#include <iostream>
#include "MonitorBroker.hpp"
#include "utilsClient.hpp"
#include "WebPieChart.hpp"
#include <Wt/WPointF>
#include <Wt/WRectArea>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WWidget>
#include <Wt/WScrollBar>
#include <Wt/WSvgImage>
#include<fstream>
#include <boost/filesystem/operations.hpp>

namespace {
  IconMapT m_icons = utils::nodeIcons();
}

WebMap::WebMap(CoreDataT* _cdata)
  : WPaintedWidget(0),
    m_cdata(_cdata),
    m_scaleX(1),
    m_scaleY(1),
    m_scrollArea(new Wt::WScrollArea()),
    m_initialLoading(true),
    m_containerSizeChanged(this, "containerSizeChanged"),
    m_thumbnail("")
{
  m_scrollArea->setWidget(this);
  setPreferredMethod();
  setLayoutSizeAware(true);
  setJavaScriptMember();
  m_containerSizeChanged.connect(this, &WebMap::handleScrollAreaSizeChanged);
}

WebMap::~WebMap()
{
}

void WebMap::setPreferredMethod(void)
{
  setInline(false);
  WPaintedWidget::setPreferredMethod(InlineSvgVml);
}

void WebMap::setJavaScriptMember(void)
{
  Wt::WPaintedWidget::setJavaScriptMember(WT_RESIZE_JS,"");
}

void WebMap::paintEvent(Wt::WPaintDevice* _pdevice)
{
  m_painter = new Wt::WPainter(_pdevice);
  m_painter->scale(m_scaleX, m_scaleY);
  m_painter->setRenderHint(Wt::WPainter::Antialiasing);

  // Draw edges
  for (StringListT::Iterator edge=m_cdata->edges.begin(), end=m_cdata->edges.end();
       edge != end; ++edge) { drawEdge(edge.key(), edge.value());}

  // Draw nodes
  for(const auto& node : m_cdata->bpnodes) drawNode(node);
  for(const auto& node : m_cdata->cnodes) drawNode(node);

  m_painter->end();
  delete m_painter;
}

void WebMap::createThumbnail(void)
{
  double thumbWidth = 150; //change later
  double thumbHeight = 120;
  double factor = (double)XSCAL_FACTOR/YSCAL_FACTOR;
  double thumbScaleY = thumbHeight/m_cdata->map_height;
  double thumbScaleX = factor * thumbHeight * thumbScaleY/m_cdata->map_height;
  thumbWidth = thumbScaleX*m_cdata->map_width;

  Wt::WSvgImage thumbnailImg(thumbWidth, thumbHeight);

  m_painter = new Wt::WPainter(&thumbnailImg);
  m_painter->scale(thumbScaleX, thumbScaleY);
  m_painter->setRenderHint(Wt::WPainter::Antialiasing);

  // Draw edges
  for (StringListT::Iterator edge=m_cdata->edges.begin(), end=m_cdata->edges.end();
       edge != end; ++edge) { drawEdge(edge.key(), edge.value());}

  // Draw nodes
  for(const auto& node : m_cdata->bpnodes) drawNode(node);
  for(const auto& node : m_cdata->cnodes) drawNode(node);

  // Draw the platform name in real scale
  m_painter->scale(1, 1);
  NodeT rootNode = m_cdata->bpnodes[utils::ROOT_ID];
  m_painter->setPen(Wt::WPen(WebPieChart::colorFromSeverity(rootNode.severity)));
  m_painter->drawText(Wt::WRectF(1, thumbHeight/2, thumbWidth, thumbHeight/2 + 20),
                      Wt::AlignCenter,
                      Wt::TextSingleLine,
                      Wt::WString(rootNode.name.toStdString()));

  m_painter->end();
  if (m_thumbnail.empty()) {
    m_thumbnail=boost::filesystem::unique_path(wApp->docRoot().append("/tmp/roi-thumb-%%%%%%.svg")).string();
  }
  std::ofstream output(m_thumbnail);
  thumbnailImg.write(output);
  delete m_painter;
}


void WebMap::layoutSizeChanged(int width, int height )
{
  //TODO
}

void WebMap::drawMap(void)
{
  Wt::WPaintedWidget::update(); //this call paintEvent
  Wt::WPaintedWidget::resize(m_cdata->map_width * m_scaleX,
                             m_cdata->map_height * m_scaleY);
  createThumbnail();
}

void WebMap::drawNode(const NodeT& _node)
{
  Wt::WPointF posIcon(_node.pos_x - 20,  _node.pos_y - 24);
  Wt::WPointF posLabel(_node.pos_x, _node.pos_y);
  Wt::WPointF posExpIcon(_node.pos_x - 10, _node.pos_y + 15);
  // Set painting color
  m_painter->setPen(Wt::WPen(WebPieChart::colorFromSeverity(_node.severity)));
  // Draw icon
  m_painter->drawImage(posIcon, Wt::WPainter::Image(utils::getPathFromQtResource(m_icons[_node.icon]),40,40));
  // Draw anchor icon
  if( _node.type == NodeType::SERVICE_NODE) { //FIXME:  map_enable_nav_icon
    m_painter->drawImage(posExpIcon,Wt::WPainter::Image(utils::getPathFromQtResource(m_icons[utils::MINUS]),19,18));
  }
  // Draw text
  m_painter->drawText(posLabel.x(), posLabel.y(),
                      Wt::WLength::Auto.toPixels(),
                      Wt::WLength::Auto.toPixels(),
                      Wt::AlignCenter,
                      Wt::WString(_node.name.toStdString()));
  createLink(_node);
}

void WebMap::drawEdge(const QString& _parentId, const QString& _childId)
{
  NodeListT::Iterator parent;
  NodeListT::Iterator child;
  if (utils::findNode(m_cdata->bpnodes, m_cdata->cnodes, _parentId, parent)
      && utils::findNode(m_cdata->bpnodes, m_cdata->cnodes, _childId, child))
  {
    QColor qcolor = utils::computeColor(child->prop_sev);
    Wt::WColor wcolor = Wt::WColor(qcolor.red(), qcolor.green(), qcolor.blue(), qcolor.alpha());
    Wt::WPen pen(wcolor);
    m_painter->setPen(pen);

    Wt::WPointF edgeP1(parent->pos_x, parent->pos_y + 24);
    Wt::WPointF edgeP2(child->pos_x, child->pos_y - 24);
    m_painter->drawLine(edgeP1, edgeP2);
  }
}

void WebMap::createLink(const NodeT& _node)
{
  double x = _node.pos_x * m_scaleX;
  double y = _node.pos_y * m_scaleY;
  double width = 40.0 * m_scaleX;
  double height = 40.0 * m_scaleY;
  Wt::WRectArea *area = new Wt::WRectArea(x, y, width, height);
  area->setToolTip(Wt::WString::fromUTF8(utils::getNodeToolTip(_node).toStdString()));
  area->setLink("#");
  addArea(area);
}

void WebMap::updateNode(const NodeT&, const QString&)
{
  // Empty function just conform with the polymorphism
}

void WebMap::scaleMap(double factor)
{
  m_scaleX *= factor;
  m_scaleY *= factor;
  Wt::WPaintedWidget::update();
  Wt::WPaintedWidget::resize(factor * width(), factor * height());
}

void WebMap::handleScrollAreaSizeChanged(double w, double h)
{
  if (m_initialLoading) {
    scaleMap(std::min(w/this->width().toPixels(), h/this->height().toPixels()));
    m_initialLoading = false;
  }
}
