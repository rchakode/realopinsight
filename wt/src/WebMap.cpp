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
  const double THUMB_BANNER_FONT_SIZE = 32;
}

WebMap::WebMap(CoreDataT* _cdata)
  : WPaintedWidget(0),
    m_cdata(_cdata),
    m_scaleX(1),
    m_scaleY(1),
    m_scrollArea(new Wt::WScrollArea()),
    m_initialLoading(true),
    m_containerSizeChanged(this, "containerSizeChanged"),
    m_loaded(this),
    m_thumbnailPath(""),
    m_translateX(0),
    m_translateY(0),
    m_thumbnail(new Wt::WImage())
{
  m_scrollArea->setWidget(this);
  setPreferredMethod();
  setLayoutSizeAware(true);
  setJavaScriptMember();
  m_containerSizeChanged.connect(this, &WebMap::handleContainedSizeChanged);
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
  m_translateX = 0;
  m_translateY = 0;
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

  if(m_initialLoading) {
    m_loaded.emit();
  }
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
  updateThumbnail();
}


void WebMap::drawNode(const NodeT& _node, bool drawIcon)
{
  m_painter->save();
  Wt::WPointF posIcon(_node.pos_x - 20 + m_translateX,  _node.pos_y - 24 + m_translateY);
  Wt::WPointF posLabel(_node.pos_x + m_translateX, _node.pos_y + m_translateY);
  Wt::WPointF posExpIcon(_node.pos_x - 10 + m_translateX, _node.pos_y + 15 + m_translateY);

  // Set pen, then draw image, nav-icon and text. The order is important !

  m_painter->setPen(Wt::WPen(WebPieChart::colorFromSeverity(_node.severity)));

  if (drawIcon) {
    m_painter->drawImage(posIcon, Wt::WPainter::Image(utils::getPathFromQtResource(m_icons[_node.icon]),40,40));
  } else {
    //TODO
  }

  if( _node.type != NodeType::SERVICE_NODE) { //FIXME:  map_enable_nav_icon
    m_painter->drawImage(posExpIcon,Wt::WPainter::Image(utils::getPathFromQtResource(m_icons[utils::MINUS]),19,18));
  }

  m_painter->drawText(posLabel.x(), posLabel.y(),
                      Wt::WLength::Auto.toPixels(),
                      Wt::WLength::Auto.toPixels(),
                      Wt::AlignCenter,
                      Wt::WString(_node.name.toStdString()));
  createLink(_node);
  m_painter->restore();
}

void WebMap::drawEdge(const QString& _parentId, const QString& _childId)
{
  m_painter->save();
  NodeListT::Iterator parent;
  NodeListT::Iterator child;
  if (utils::findNode(m_cdata->bpnodes, m_cdata->cnodes, _parentId, parent)
      && utils::findNode(m_cdata->bpnodes, m_cdata->cnodes, _childId, child))
  {
    QColor qcolor = utils::computeColor(child->prop_sev);
    Wt::WColor wcolor = Wt::WColor(qcolor.red(), qcolor.green(), qcolor.blue(), qcolor.alpha());
    Wt::WPen pen(wcolor);
    m_painter->setPen(pen);

    Wt::WPointF edgeP1(parent->pos_x + m_translateX, parent->pos_y + 24 + m_translateY);
    Wt::WPointF edgeP2(child->pos_x + m_translateX, child->pos_y - 24 + m_translateY);
    m_painter->drawLine(edgeP1, edgeP2);
  }
  m_painter->restore();
}

void WebMap::createLink(const NodeT& _node)
{
  double x = (_node.pos_x + m_translateX) * m_scaleX;
  double y = (_node.pos_y + m_translateY) * m_scaleY;
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

void WebMap::handleContainedSizeChanged(double w, double h)
{
  if (m_initialLoading) {
    scaleMap(std::min(w/this->width().toPixels(), h/this->height().toPixels()));
    m_initialLoading = false;
  }
}

void WebMap::updateThumbnail(void)
{
  static int roundCount = 0;
  double thumbWidth = 150; //change later
  double thumbHeight = 120;
  double factor = (double)XSCAL_FACTOR/YSCAL_FACTOR;
  double thumbScaleY = thumbHeight/m_cdata->map_height;
  double thumbScaleX = factor * thumbHeight * thumbScaleY/m_cdata->map_height;
  thumbWidth = thumbScaleX*m_cdata->map_width + 20;

  m_translateY = 10;
  m_translateY = THUMB_BANNER_FONT_SIZE/thumbScaleY;
  Wt::WSvgImage thumbnailImg(thumbWidth, thumbHeight + m_translateY);

  m_painter = new Wt::WPainter(&thumbnailImg);
  m_painter->scale(thumbScaleX, thumbScaleY);
  m_painter->setRenderHint(Wt::WPainter::Antialiasing);


  drawThumbnailBanner(thumbWidth, thumbHeight, thumbScaleX, thumbScaleY, m_translateY);

  // Draw edges
  for (StringListT::Iterator edge=m_cdata->edges.begin(), end=m_cdata->edges.end();
       edge != end; ++edge) { drawEdge(edge.key(), edge.value());}

  // Draw nodes
  bool drawIcon = false;
  for(const auto& node : m_cdata->bpnodes) drawNode(node, drawIcon);
  for(const auto& node : m_cdata->cnodes) drawNode(node, drawIcon);

  m_painter->end();

  // Now save the image
  if (m_thumbnailPath.empty()) {
    m_thumbnailPath=boost::filesystem::unique_path(wApp->docRoot().append("/tmp/roi-thumb-%%%%%%.svg")).string();
  }
  std::ofstream output(m_thumbnailPath);
  thumbnailImg.write(output);
  delete m_painter;

  m_thumbnail->setImageLink(m_thumbnailPath+"?"+QString::number(++roundCount).toStdString());
  m_thumbnail->setToolTip(utils::severity2Str(m_cdata->root->severity).toStdString());
}

void WebMap::drawThumbnailBanner(double thumbWidth, double thumbHeight,
                                 double scaleX, double scaleY,
                                 double fontSize)
{
  m_painter->save();
  Wt::WFont font;
  std::string text = m_cdata->root->name.toStdString();
  font.setSize(fontSize);
  double textLength = thumbWidth/scaleX;// fontSize;
  Wt::WColor brushColor = WebPieChart::colorFromSeverity(m_cdata->root->severity);
  Wt::WRectF bannerArea(0, 0, textLength, fontSize);
  m_painter->setFont(font);
  m_painter->setPen(Wt::WPen(brushColor));
  m_painter->setBrush(Wt::WBrush(brushColor));
  m_painter->drawRect(bannerArea);
  m_painter->setPen(Wt::WPen(Wt::black));
  m_painter->drawText(bannerArea,Wt::AlignCenter|Wt::AlignMiddle,
                      Wt::TextSingleLine,
                      m_cdata->root->name.toStdString());
  m_painter->restore();
}
