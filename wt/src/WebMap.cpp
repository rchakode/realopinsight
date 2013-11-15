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

#include <Wt/WPointF>
#include <Wt/WRectArea>
#include "WebMap.hpp"
#include "MonitorBroker.hpp"
#include "utilsClient.hpp"

WebMap::WebMap(CoreDataT* _cdata)
  : WPaintedWidget(0),
    m_cdata(_cdata),
    scaleX (1),
    scaleY(1),
    layoutWidth(0),
    layoutHeight(0),
    m_icons(utils::nodeIcons())
{
  setLayoutSizeAware(true);
  setPreferredMethod(InlineSvgVml); //TODO do that according to the user agent
  setInline(false);
}


WebMap::~WebMap()
{
  m_icons.clear();
}

void WebMap::drawMap(const bool& _init)
{
  if(_init) {
    scaleX = layoutWidth/m_cdata->map_width;
    scaleY = static_cast<double>(YSCAL_FACTOR)/XSCAL_FACTOR * scaleX;
  }

  Wt::WPaintedWidget::update(); //this call paintEvent
  Wt::WPaintedWidget::resize(m_cdata->map_width * scaleX, m_cdata->map_height * scaleY );
}

void WebMap::paintEvent(Wt::WPaintDevice* _pdevice)
{
  painter = new Wt::WPainter(_pdevice);
  painter->scale(scaleX, scaleY);  //TODO Make it dynamic
  painter->setRenderHint(Wt::WPainter::Antialiasing);

  // Draw edges
  // Must to be drawn before the icon for hiding some technical details
  for (StringListT::Iterator edge = m_cdata->edges.begin(), end = m_cdata->edges.end();
       edge != end; ++edge) {
    drawEdge(edge.key(), edge.value());
  }

  /* Draw node related to business services */
  for(NodeListT::ConstIterator node=m_cdata->bpnodes.begin(), end=m_cdata->bpnodes.end();
      node != end; ++node) {
    drawNode(*node);
  }

  /* Draw node related to alarm services */
  for(NodeListT::ConstIterator node=m_cdata->cnodes.begin(),end=m_cdata->cnodes.end();
      node != end; ++node) {
    drawNode(*node);
  }
}

/**
 * Draw edge before node to hide some details of drawing
 */
void WebMap::drawNode(const NodeT& _node)
{
  Wt::WPointF posIcon(_node.label_x,  _node.label_y);
  Wt::WPointF posLabel(_node.label_x, _node.label_y); //20 = 40/2
  Wt::WPointF posNavIcon(_node.label_x, _node.label_y); //10 + 40

  // Draw icon
  painter->drawImage(posIcon,
                     Wt::WPainter::Image(utils::getResourcePath(m_icons[_node.icon]),40,40)
                     );

  // Draw anchor icon
  if( _node.type == NodeType::SERVICE_NODE) { //FIXME:  map_enable_nav_icon
    painter->drawImage(posNavIcon, Wt::WPainter::Image(utils::getResourcePath(m_icons[utils::MINUS]), 19, 18));
  }

  // Draw text
  painter->drawText(posLabel.x(), posLabel.y(),
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
      && utils::findNode(m_cdata->bpnodes, m_cdata->cnodes, _childId, child)) {

    Wt::WPen pen;
    if(child->prop_sev == MonitorBroker::Critical){ //FIXME: child->prop_sev == MonitorBroker::Critical
      pen.setColor(Wt::red);
    } else if(child->prop_sev == MonitorBroker::Major){
      pen.setColor(Wt::darkYellow);
    } else if(child->prop_sev == MonitorBroker::Normal){
      pen.setColor(Wt::darkGreen);
    } else {
      pen.setColor(Wt::gray);
    }
    painter->setPen(pen);

    Wt::WPointF edgeP1(parent->label_x, parent->label_y);
    Wt::WPointF edgeP2(child->label_x, child->label_y); // 46 + 18
    painter->drawLine(edgeP1, edgeP2);
  }

}

void WebMap::createLink(const NodeT& _service)
{
  std::ostringstream tip;
  //  double x = _service.label_x * scaleX;
  //  double y = _service.label_y * scaleY;
  //  double width = 40.0 * scaleX;
  //  double height = 60.0 * scaleY;
  //  Wt::WRectArea *area = new Wt::WRectArea(x, y, width, height);

  tip << "Service : " << _service.name.toStdString() << " ";
  //  //FIXME:    << "\nStatus : " << Ngrt4nConfigParser::statusToString(_service.status_info) << " "
  //      << "\nDetails : " << _service.msg;

  //  area->setToolTip(tip.str());
  //  area->setLink("http://ngrt4n.com");
  //  addArea(area);
}

//FIXME: void WebMap::msgPanelSizedChanged(int width)
//void WebMap::msgPanelSizedChanged(int width)
//{
//  layoutWidth = width;
//  drawMap(layoutWidth, layoutHeight, true);
//}
