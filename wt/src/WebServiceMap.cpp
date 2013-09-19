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
#include "WebServiceMap.hpp"
#include "MonitorBroker.hpp"

WebServiceMap::WebServiceMap()
: WPaintedWidget(0),
  scaleX (1),
  scaleY(1),
  layoutWidth(0),
  layoutHeight(0)
{
  setLayoutSizeAware(true);
  setPreferredMethod(InlineSvgVml); //TODO do that according to the user agent
  setInline(false);
}


WebServiceMap::~WebServiceMap() {}

void WebServiceMap::update(const NodeListT& _bservices,
    const NodeListT& _aservices,
    const double& _width,
    const double& _height)
{
  //FIXME: const_cast<NodeListT*>(&_bservices);
  bservices = const_cast<NodeListT*>(&_bservices);
  aservices = const_cast<NodeListT*>(&_aservices);
  width = _width;
  height = _height;
  update(); //TODO Make it dynamic
}

void WebServiceMap::update(const bool& _init)
{
  if( _init ) {
      scaleX = layoutWidth/width;
      scaleY = static_cast<double>(YSCAL_FACTOR)/XSCAL_FACTOR * scaleX;
  }

  Wt::WPaintedWidget::update();
  Wt::WPaintedWidget::resize(width * scaleX, height * scaleY );
}

void WebServiceMap::paintEvent(Wt::WPaintDevice* _pdevice)
{
  painter = new Wt::WPainter(_pdevice);
  painter->scale(scaleX, scaleY);  //TODO Make it dynamic
  painter->setRenderHint(Wt::WPainter::Antialiasing);

  /* Draw node related to business services */
  for(NodeListT::const_iterator it = bservices->begin(); it != bservices->end(); it++){
      drawNode(*it);
  }

  /* Draw node related to alarm services */
  for(NodeListT::const_iterator it = aservices->begin(); it != aservices->end(); it++){
      drawNode(*it);
  }
}

/**
 * Draw edge before node to hide some details of drawing
 */
void WebServiceMap::drawNode(const NodeT& _service)
{
//FIXME: Wt::WPointF posIcon(_service.map_x,  _service.map_y);
//  Wt::WPointF posIcon(_service.map_x,  _service.map_y);
//  Wt::WPointF posLabel(_service.map_x + 20, _service.map_y + 40); //20 = 40/2
//  Wt::WPointF posNavIcon(_service.map_x + 9, _service.map_y + 46); //10 + 40

  Wt::WPen pen(Wt::gray);
//  if( _service.status_info[MonitorBroker::NagiosCritical] ){
//      pen.setColor(Wt::red);
//  } else if(_service.status_info[MonitorBroker::NagiosWarning] ){
//      pen.setColor(Wt::darkYellow);
//  } else if( _service.status_info[MonitorBroker::NagiosOk]){
//      pen.setColor(Wt::darkGreen);
//  }
  painter->setPen(pen);

  /* Draw the edge from parent.
   * Need to be drawn before the icon for hiding some technical details
   */
  //FIXME:
  if(_service.id != "root") {
//      NodeListT::ConstIterator pService = bservices->find(_service.parent); //should find it
//      Wt::WPointF edgeP1(_service.map_x + 20, _service.map_y + 5);
//      Wt::WPointF edgeP2(pService->second.map_x + 20, pService->second.map_y + 64); // 46 + 18
//      painter->drawLine(edgeP1, edgeP2);
  }

  /* Draw icons and text
   */
//  painter->drawImage(posIcon, Wt::WPainter::Image(iconPaths[_service.icon], 40, 40));
//  painter->drawText(posLabel.x(), posLabel.y(),
//                    Wt::WLength::Auto.toPixels(),
//                    Wt::WLength::Auto.toPixels(),
//                    Wt::AlignCenter, _service.name);
//  if( _service.map_enable_nav_icon ) {
//      std::string navIcon = "icons/built-in/map-nav-minus.png";
//      painter->drawImage(posNavIcon, Wt::WPainter::Image(navIcon, 19, 18));
//  }
//  createLink(_service);
}
void WebServiceMap::createLink(const NodeT& _service)
{
  std::ostringstream tip;
//  double x = _service.map_x * scaleX;
//  double y = _service.map_y * scaleY;
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
