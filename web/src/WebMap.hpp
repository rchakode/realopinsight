/*
 * WebServiceMap.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 23-03-2014                                                 #
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

#include <QString>
#include <Wt/WPaintedWidget.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WObject.h>
#include <Wt/WLength.h>
#include <Wt/WSignal.h>
#include <Wt/WImage.h>

struct CoreDataT;
struct NodeT;

class WebMap : public Wt::WPaintedWidget
{
public:
  WebMap(CoreDataT* cdata);
  virtual ~WebMap();
  void drawMap(void);
  void updateNode(const NodeT& _node, const QString& _toolTip);
  void scaleMap(double factor);
  void setScaleFactor(double factorX, double factorY);
  void updateThumb(void);
  Wt::JSignal<double, double, double, double>& containerSizeChanged(void) {
    return m_containerSizeChanged;
  }
  std::string thumbURL(void) {
    return m_thumbURL;
  }


protected:
  void paintEvent(Wt::WPaintDevice *paintDevice);

private:
  CoreDataT* m_cdata;
  double m_scaleX;
  double m_scaleY;
  std::shared_ptr<Wt::WPainter> m_painter;
  bool m_initialLoading;
  Wt::JSignal<double, double, double, double> m_containerSizeChanged;
  Wt::Signal<> m_loaded;
  std::string m_thumbURL;
  double m_translateY;
  std::string m_thumbImg;

  void drawNode(const NodeT& node, bool drawIcon = true);
  void drawEdge(const QString& parentId, const QString& childId);
  void createNodeLink(const NodeT& node, const Wt::WPointF& pos);
  void createExpIconLink(const NodeT& node, const Wt::WPointF& expIconPos);
  void setPreferredMethod(void);
  void handleContainedSizeChanged(double mapW, double mapH, double winW, double winH);
  void expandCollapse(const QString& nodeId);
  void applyVisibilityToChild(const NodeT& node, qint8 mask);
  void removeThumbImage(void);
  QString toBase64RootNodeName(void);
};

#endif /* WEBSERVICEMAP_HPP */
