/*
 * WebServiceTree.hpp
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

#ifndef WEBSERVICETREE_HPP
#define WEBSERVICETREE_HPP


#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WModelIndex>
#include "Base.hpp"

class WebTree : public Wt::WTreeView
{
public:
  WebTree(CoreDataT* _cdata);
  virtual ~WebTree();

  Wt::WStandardItemModel* renderingModel(void) const {return m_model;}
  void setRenderingModel(Wt::WStandardItem* _item) const {return m_model->appendRow(_item);}
  void updateNodeItem(const NodeT& _node, const QString& _tip);


  void build(void);
  void update(void);
  void update(Wt::WStandardItem * _rItem) { setRenderingModel(_rItem); update(); }

private:
  typedef QMap<QString,  Wt::WStandardItem*> WebTreeItemsT;
  Wt::WStandardItemModel* m_model;
  CoreDataT* m_cdata;
  WebTreeItemsT m_items;

  Wt::WStandardItem* createItem(const NodeT& _node);
  Wt::WStandardItem* findNodeItem(const QString& _nodeId);
};

#endif /* WEBSERVICETREE_HPP */
