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
#include <fstream>
#include "utilsCore.hpp"
#include "WebPieChart.hpp"
#include <Wt/WPointF>
#include <Wt/WRectArea>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WWidget>
#include <Wt/WScrollBar>
#include <Wt/WSvgImage>
#include <boost/filesystem/operations.hpp>

namespace {
  const double THUMB_BANNER_FONT_SIZE = 32;
  typedef Wt::WPainter::Image GImage;
  const double THUMBNAIL_WIDTH = 120;
  const double THUMBNAIL_HEIGHT = 60;
  const double ICON_SIZE = 40.0;
}

WebMap::WebMap(void)
  : WPaintedWidget(),
    m_scaleX(1),
    m_scaleY(1),
    m_initialLoading(true),
    m_containerSizeChanged(this, "containerSizeChanged"),
    m_thumbUrlPath("")
{
  m_scrollArea.setWidget(this);
  setPreferredMethod();
  setLayoutSizeAware(true);
  Wt::WPaintedWidget::setJavaScriptMember(WT_RESIZE_JS, "");
  m_containerSizeChanged.connect(this, &WebMap::handleContainedSizeChanged);
}


WebMap::~WebMap()
{
  removeThumdImage();
  m_scrollArea.takeWidget();
}


void WebMap::setPreferredMethod(void)
{
  setInline(false);
  WPaintedWidget::setPreferredMethod(InlineSvgVml);
}

void WebMap::paintEvent(Wt::WPaintDevice* _pdevice)
{
  m_painter.reset(new Wt::WPainter(_pdevice));
  m_painter->scale(m_scaleX, m_scaleY);
  m_painter->setRenderHint(Wt::WPainter::Antialiasing);

  // Draw edges before nodes
  for (auto edge=std::begin(m_cdata->edges); edge != std::end(m_cdata->edges); ++edge) {
    drawEdge(edge.key(), edge.value());
  }
  // Draw bpnodes
  for(const auto& node : m_cdata->bpnodes) {
    drawNode(node);
  }
  // Draw cnodes
  for(const auto& node : m_cdata->cnodes) {
    drawNode(node);
  }

  m_painter->end();

  if (m_initialLoading) m_loaded.emit();
}



void WebMap::layoutSizeChanged(int width, int height)
{
  //WPaintedWidget::layoutSizeChanged(widt
  //m_scrollArea.resize(width, height);
  //TODO layoutSizeChanged(int width, int height)
  //qDebug()<<m_cdata->map_width * m_scaleX << m_cdata->map_height * m_scaleY << width << height << m_cdata->map_width << m_cdata->map_height;
}

void WebMap::drawMap(void)
{
  Wt::WPaintedWidget::update(); //this calls paintEvent
  Wt::WPaintedWidget::resize(m_cdata->map_width * m_scaleX, m_cdata->map_height * m_scaleY);
  updateThumbnail();
}


void WebMap::drawNode(const NodeT& node, bool drawIcon)
{
  if (node.visibility & ngrt4n::Visible) {

    const double COLOR_BORDER_SIZE = 5.0;
    const double COLOR_BORDER_DOUBLE_SIZE = 2 * COLOR_BORDER_SIZE;
    const int MAX_LABEL_LENGTH = 20;

    double base_x = node.pos_x + m_cdata->min_x;
    double base_y = node.pos_y + m_cdata->min_y;
    Wt::WPointF iconPos(base_x - 20,  base_y - 24);
    Wt::WPointF labelPos(base_x, base_y);
    Wt::WPointF expIconPos(base_x - 10, base_y + 15);


    m_painter->save();

    m_painter->setPen(Wt::WPen(Wt::WColor(255, 255, 255, 0)));
    m_painter->setBrush(Wt::WBrush(ngrt4n::severityWColor(node.sev)));

    m_painter->drawRect(iconPos.x() - COLOR_BORDER_SIZE,
                        iconPos.y() - COLOR_BORDER_SIZE,
                        ICON_SIZE + COLOR_BORDER_DOUBLE_SIZE,
                        ICON_SIZE + COLOR_BORDER_DOUBLE_SIZE);

    if (drawIcon) {
      m_painter->drawImage(iconPos, GImage(ngrt4n::NodeIcons[node.icon], static_cast<int>(ICON_SIZE), static_cast<int>(ICON_SIZE)));
    } else { /* thumbnail: do nothing*/ }

    if( node.type == NodeType::BusinessService) {
      if (node.visibility & ngrt4n::Expanded) {
        m_painter->drawImage(expIconPos,GImage(ngrt4n::NodeIcons[ngrt4n::MINUS], 19, 18));
      } else {
        m_painter->drawImage(expIconPos,GImage(ngrt4n::NodeIcons[ngrt4n::PLUS], 19, 18));
      }
      createExpIconLink(node, expIconPos);
    }

    m_painter->setPen(Wt::WPen(Wt::WColor("#000000")));

    std::string label = (node.name.size() <= MAX_LABEL_LENGTH) ? node.name.toStdString() : node.name.toStdString().substr(0, MAX_LABEL_LENGTH) + "...";
    m_painter->drawText(labelPos.x(),
                        labelPos.y(),
                        Wt::WLength::Auto.toPixels(),
                        Wt::WLength::Auto.toPixels(),
                        Wt::AlignCenter,
                        label);
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

      Wt::WPointF edgeP1(parent->pos_x + m_cdata->min_x, parent->pos_y + 24 + m_cdata->min_y);
      Wt::WPointF edgeP2(child->pos_x + m_cdata->min_x, child->pos_y - 24 + m_cdata->min_y);
      m_painter->drawLine(edgeP1, edgeP2);
      m_painter->restore();
    }
  }
}

void WebMap::createNodeLink(const NodeT& node, const Wt::WPointF& pos)
{
  Wt::WRectArea* area = new Wt::WRectArea(pos.x() * m_scaleX,
                                          pos.y() * m_scaleY,
                                          ICON_SIZE * m_scaleX,
                                          ICON_SIZE * m_scaleY);
  area->setToolTip(Wt::WString::fromUTF8(node.toString().toStdString()));
  addArea(area);
}


void WebMap::createExpIconLink(const NodeT& _node, const Wt::WPointF& expIconPos)
{
  Wt::WRectArea* area = new Wt::WRectArea(expIconPos.x() * m_scaleX,
                                          expIconPos.y() * m_scaleY,
                                          20 * m_scaleX,
                                          20 * m_scaleY);
  area->setToolTip(Wt::WString::fromUTF8(_node.toString().toStdString()));
  //TODO: test the replacement expression :: area->clicked().connect(std::bind([=]() {expandCollapse(_node.id);}));
  area->clicked().connect(std::bind(&WebMap::expandCollapse, this, _node.id));
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

void WebMap:: updateThumbnail(void)
{
  static int roundCount = 0;
  double thumbScaleX = THUMBNAIL_WIDTH / m_cdata->map_width;
  double thumbScaleY = THUMBNAIL_HEIGHT / m_cdata->map_height;

  Wt::WSvgImage thumbnailImg(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);

  m_painter.reset(new Wt::WPainter(&thumbnailImg));
  m_painter->scale(thumbScaleX, thumbScaleY);
  m_painter->setRenderHint(Wt::WPainter::Antialiasing);

  // Just draw edges for thumbnails
  for (QMultiMap<QString, QString>::Iterator edge = m_cdata->edges.begin(), end = m_cdata->edges.end(); edge != end; ++edge) {
    drawEdge(edge.key(), edge.value());
  }

  m_painter->end();

  // Now save the image
  if (m_thumbUrlPath.empty()) {
    m_thumbUrlPath = QString("/run/thumb-%1.svg").arg(toBase64RootNodeName()).toStdString();
  }
  std::ofstream output(wApp->docRoot() + m_thumbUrlPath);
  thumbnailImg.write(output);
  output.close();

  m_thumbImage.setImageLink (m_thumbUrlPath+"?"+QString::number(++roundCount).toStdString());
}


void WebMap::expandCollapse(const QString& nodeId)
{
  NodeListT::Iterator node;
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
    for (const auto & childId: node.child_nodes.split(ngrt4n::CHILD_Q_SEP)) {
      NodeListT::Iterator child;
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


void WebMap::removeThumdImage(void)
{
  QFile file(m_thumbUrlPath.c_str());
  file.remove();
}


QString WebMap::toBase64RootNodeName(void)
{
  QString rootNodeName = "";
  if (m_cdata) {
    NodeListT::ConstIterator rootNodeIt;
    if (ngrt4n::findNode(m_cdata->bpnodes, ngrt4n::ROOT_ID, rootNodeIt)){
      rootNodeName = rootNodeIt->name;
    }
  }
  return ngrt4n::toByteArray(rootNodeName).toBase64();
}
