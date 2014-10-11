/*
 * WebServiceMap.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
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

#include "WebUtils.hpp"
#include "WebMap.hpp"
#include <iostream>
#include "utilsCore.hpp"
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
  const IconMapT ICONS = ngrt4n::nodeIcons();
  const double THUMB_BANNER_FONT_SIZE = 32;
  typedef Wt::WPainter::Image GImage;
  const std::string WWW_ROOT = REALOPINSIGHT_WWW_ROOT;
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
  m_painter.reset(new Wt::WPainter(_pdevice));
  m_painter->scale(m_scaleX, m_scaleY);
  m_painter->setRenderHint(Wt::WPainter::Antialiasing);

  // Draw edges
  for (auto edge=std::begin(m_cdata->edges); edge != std::end(m_cdata->edges); ++edge) { drawEdge(edge.key(), edge.value());}

  // Draw nodes
  for(const auto& node : m_cdata->bpnodes) drawNode(node);
  for(const auto& node : m_cdata->cnodes) drawNode(node);

  m_painter->end();

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

/**
 * @brief WebMap::drawNode
 * Set node position, set pen, then draw image, nav-icon and text.
 * The order is important !
 * @param node
 * @param drawIcon
 */
void WebMap::drawNode(const NodeT& node, bool drawIcon)
{
  if (node.visibility & ngrt4n::Visible) {

    m_painter->save();

    Wt::WPointF iconPos(node.pos_x - 20 + m_translateX,  node.pos_y - 24 + m_translateY);
    Wt::WPointF labelPos(node.pos_x + m_translateX, node.pos_y + m_translateY);
    Wt::WPointF expIconPos(node.pos_x - 10 + m_translateX, node.pos_y + 15 + m_translateY);

    m_painter->setPen(Wt::WPen(Wt::WColor(255, 255, 255, 0)));
    m_painter->setBrush(Wt::WBrush(ngrt4n::severityWColor(node.sev)));

    m_painter->drawRect(labelPos.x() - node.text_w / 2,
                        labelPos.y() - node.text_h / 2,
                        node.text_w,
                        node.text_h);

    if (drawIcon) {
      m_painter->drawImage(iconPos, GImage(ngrt4n::getPathFromQtResource(ICONS[node.icon]),40,40));
    } else { /* thumbnail: do nothing*/ }

    if( node.type == NodeType::BusinessService) {
      if (node.visibility & ngrt4n::Expanded) {
        m_painter->drawImage(expIconPos,GImage(ngrt4n::getPathFromQtResource(ICONS[ngrt4n::MINUS]),19,18));
      } else {
        m_painter->drawImage(expIconPos,GImage(ngrt4n::getPathFromQtResource(ICONS[ngrt4n::PLUS]),19,18));
      }
      createExpIconLink(node, expIconPos);
    }


    m_painter->setPen(Wt::WPen(Wt::WColor("#000000")));
    m_painter->drawText(labelPos.x(), labelPos.y(),
                        Wt::WLength::Auto.toPixels(), Wt::WLength::Auto.toPixels(),
                        Wt::AlignCenter,
                        Wt::WString(node.name.toStdString()));
    createNodeLink(node, iconPos);

    m_painter->restore();
  }
}

void WebMap::drawEdge(const QString& parentId, const QString& childId)
{
  NodeListT::Iterator parent;
  NodeListT::Iterator child;
  if (ngrt4n::findNode(m_cdata->bpnodes, m_cdata->cnodes, parentId, parent)
      && ngrt4n::findNode(m_cdata->bpnodes, m_cdata->cnodes, childId, child))
  {
    if (parent->visibility & ngrt4n::Expanded) {
      m_painter->save();
      Wt::WPen pen(ngrt4n::severityWColor(child->sev_prop));
      m_painter->setPen(pen);

      Wt::WPointF edgeP1(parent->pos_x + m_translateX, parent->pos_y + 24 + m_translateY);
      Wt::WPointF edgeP2(child->pos_x + m_translateX, child->pos_y - 24 + m_translateY);
      m_painter->drawLine(edgeP1, edgeP2);
      m_painter->restore();
    }
  }
}

void WebMap::createNodeLink(const NodeT& node, const Wt::WPointF& pos)
{
  Wt::WRectArea* area = new Wt::WRectArea(pos.x() * m_scaleX, pos.y() * m_scaleY,
                                          40 * m_scaleX, 40 * m_scaleY);
  area->setToolTip(Wt::WString::fromUTF8(node.toString().toStdString()));
  addArea(area);
}

void WebMap::createExpIconLink(const NodeT& _node, const Wt::WPointF& expIconPos)
{
  Wt::WRectArea* area = new Wt::WRectArea(expIconPos.x() * m_scaleX, expIconPos.y() * m_scaleY,
                                          20 * m_scaleX, 20 * m_scaleY);
  area->setToolTip(Wt::WString::fromUTF8(_node.toString().toStdString()));
  area->clicked().connect(std::bind([=]() {expandCollapse(_node.id);}));
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
  double factor = (double)ngrt4n::XSCAL_FACTOR/ngrt4n::YSCAL_FACTOR;
  double thumbScaleY = thumbHeight/m_cdata->map_height;
  double thumbScaleX = factor * thumbHeight * thumbScaleY/m_cdata->map_height;
  thumbWidth = thumbScaleX*m_cdata->map_width + 20;

  m_translateY = 10;
  m_translateY = 0;
  Wt::WSvgImage thumbnailImg(thumbWidth, thumbHeight + m_translateY);

  m_painter.reset(new Wt::WPainter(&thumbnailImg));
  m_painter->scale(thumbScaleX, thumbScaleY);
  m_painter->setRenderHint(Wt::WPainter::Antialiasing);

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
    m_thumbnailPath=boost::filesystem::unique_path("run/thumb-%%%%%%.svg").string();
  }
  std::ofstream output(WWW_ROOT + "/" + m_thumbnailPath);
  thumbnailImg.write(output);

  m_thumbnail->setImageLink(m_thumbnailPath+"?"+QString::number(++roundCount).toStdString());
}


void WebMap::expandCollapse(const QString& nodeId)
{
  NodeListIteratorT node;
  if (ngrt4n::findNode(m_cdata, nodeId, node)) {
    qint8 childMask = 0x0;
    if (node->visibility & ngrt4n::Expanded) {
      childMask = ngrt4n::Hidden;
      node->visibility &= (ngrt4n::Collapsed | ngrt4n::Visible);
    } else {
      childMask = ngrt4n::Visible;
      node->visibility |= ngrt4n::Expanded;
    }
    applyVisibilityToChild(*node, childMask);
    drawMap();
  }
}


void WebMap::applyVisibilityToChild(const NodeT& node, qint8 mask)
{
  if (node.type != NodeType::ITService && ! node.child_nodes.isEmpty()) {
    for (const auto & childId: node.child_nodes.split(ngrt4n::CHILD_SEP.c_str())) {
      NodeListIteratorT child;
      if(ngrt4n::findNode(m_cdata, childId, child)) {
        if (node.visibility & ngrt4n::Expanded) {
          child->visibility |= mask;
        } else {
          child->visibility &= mask;
        }
        applyVisibilityToChild(*child, mask);
      }
    }
  }
}
