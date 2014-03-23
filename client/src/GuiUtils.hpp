/*
#  GuiUtils.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
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

#ifndef GUIUTILS_HPP
#define GUIUTILS_HPP
#include <QtGui>

struct GNodeT {
  QGraphicsTextItem* label;
  QGraphicsPixmapItem* icon;
  QGraphicsPixmapItem* exp_icon;
  qint32 type;
  bool expand;
};
struct GEdgeT {
  QGraphicsPathItem* edge;
};
typedef QHash<QString, GNodeT> GNodeListT;
typedef QHash<QString, GEdgeT> GEdgeListT;
typedef QHash<QString, QTreeWidgetItem*> GuiTreeItemListT;
typedef QMap<QString, QMenu*> MenuListT;
typedef QMap<QString, QAction*> SubMenuListT;

namespace ngrt4n {
  const QColor COLOR_NORMAL =  Qt::green;
  const QColor COLOR_MINOR = Qt::yellow;
  const QColor COLOR_MAJOR = QColor("#FF8C00");
  const QColor COLOR_CRITICAL = Qt::red;
  const QColor COLOR_UNKNOWN = Qt::lightGray;
  const QColor HIGHLIGHT_COLOR = QColor(255, 255, 200);
  QSplashScreen* infoScreen(const QString & msg="");
  QColor severityQColor(const int& _criticity);
  void alert(const QString & msg);
  QIcon severityIcon(int _severity);
  QString getWelcomeMsg(const QString& utility);
}

#endif // GUIUTILS_HPP
