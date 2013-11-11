/*
 * WebServiceMap.hpp
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

#ifndef WEBSERVICEMAP_HPP
#define WEBSERVICEMAP_HPP

#include <Wt/WPaintedWidget>
#include <Wt/WContainerWidget>
#include <Wt/WPainter>
#include <Wt/WObject>
#include <Wt/WLength>
#include <Wt/WSignal>
#include "Base.hpp"

class WebServiceMap : public Wt::WPaintedWidget
{
public:
  WebServiceMap();
  virtual ~WebServiceMap();
  void setWidth(const double& _width) {width =  _width;}
  void setHeight(const double& _height) {height =  _height;}
  void update(const bool& _init = false);
  void update(const NodeListT& _bservices, const NodeListT& _aservices,const double& _width, const double& _height);
  void  msgPanelSizedChanged(int width){ layoutWidth = width; update(true);}

protected:
  void paintEvent(Wt::WPaintDevice *paintDevice);

private:
  double width;
  double height;
  double scaleX;
  double scaleY;
  double layoutWidth;
  double layoutHeight;
  Wt::WPainter* painter;
  NodeListT* aservices;
  NodeListT* bservices;

  void drawNode(const NodeT& _service);
  void createLink(const NodeT& _service);
};

#endif /* WEBSERVICEMAP_HPP */
