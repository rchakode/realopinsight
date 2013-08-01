/*
 * SvNavigatorTree.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
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

#ifndef SNAVSVNAVIGATORTREE_H_
#define SNAVSVNAVIGATORTREE_H_

#include "Base.hpp"

class SvNavigatorTree : public QTreeWidget
{
  Q_OBJECT

public:
  static const QString RootId ;

  SvNavigatorTree(const bool & =false, QWidget* = 0 );
  static void addNode( TreeNodeItemListT & , const NodeT &, const bool & = false );
  void update(CoreDataT * & _coreData) ;
  static QTreeWidgetItem * createTreeItem(const NodeT & _node);

signals:
  void treeNodeMoved( QString _nodeId ) ;

protected:
  void showEvent(QShowEvent *) ;
  void dropEvent(QDropEvent *) ;
  void startDrag(Qt::DropActions)  ;

private:
  QString m_selectedNode ;
  CoreDataT* m_cdata ;
};

#endif /* SNAVSVNAVIGATORTREE_H_ */
