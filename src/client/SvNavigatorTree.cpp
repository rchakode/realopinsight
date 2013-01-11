/*
 * SvNavigatorTree.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */

#include "core/ns.hpp"
#include "SvNavigatorTree.hpp"
#include <QtGui>
#include "Parser.hpp"
#include "utilsClient.hpp"

const QString SvNavigatorTree::ROOT_ID = "root";

SvNavigatorTree::SvNavigatorTree(const bool& _enable_drag, QWidget* _parent)
  : QTreeWidget(_parent), coreData(NULL)
{
  setHeaderLabel(tr("TV Explorer")) ;
  setColumnCount(1) ;
  setDragDropMode(QAbstractItemView::DragDrop) ;
  setDragEnabled(_enable_drag) ;
}

void SvNavigatorTree::showEvent(QShowEvent*)
{
  expandAll() ;
}

void SvNavigatorTree::dropEvent(QDropEvent * _event)
{
  QTreeWidgetItem* tnode = itemAt(_event->pos()) ;
  if(tnode && coreData) {
      NodeListT::iterator  node_it = coreData->bpnodes.find(tnode->data(0, QTreeWidgetItem::UserType).toString()) ;

      if(node_it != coreData->bpnodes.end()) {

          if(node_it->type != NodeType::ALARM_NODE) {
              _event->setDropAction(Qt::MoveAction) ;
              QTreeWidget::dropEvent(_event) ;
              emit treeNodeMoved(selectedNode) ;
            }else {
              utils::alert(tr("Dropping is not allowed on the target node"));
            }
        }
    }
}

void SvNavigatorTree::startDrag(Qt::DropActions _action)
{
  QList<QTreeWidgetItem*> items ;

  items = selectedItems() ;
  if(items.length()) selectedNode = items[0]->data(0, QTreeWidgetItem::UserType).toString() ;
  QTreeWidget::startDrag(_action) ;
}

void SvNavigatorTree::addNode(TreeNodeItemListT& _tree,
                              const NodeT& _node,
                              const bool& _isFirstInsertion)
{
  TreeNodeItemListT::iterator itemIt = _tree.find(_node.id) ;
  if(itemIt == _tree.end()) {

      QTreeWidgetItem * item = SvNavigatorTree::createTreeItem(_node);
      if(_isFirstInsertion) {
          TreeNodeItemListT::iterator pit = _tree.find(_node.parent) ;
          if(pit != _tree.end()) {
              _tree[_node.parent]->addChild(item);
            }
        }

      _tree.insert(_node.id, item);

    } else {
      (*itemIt)->setIcon(0, QIcon(":/images/built-in/unknown.png")) ;
      (*itemIt)->setText(0, _node.name) ;
      (*itemIt)->setData(0, QTreeWidgetItem::UserType, _node.id) ;
    }

  if(_node.type != NodeType::ALARM_NODE &&
     _node.child_nodes != "") {

      QStringList childs = _node.child_nodes.split(Parser::CHILD_SEP);

      for(QStringList::iterator uds_it = childs.begin(); uds_it != childs.end(); uds_it++) {

          QString cid = (*uds_it).trimmed() ;
          TreeNodeItemListT::iterator cit = _tree.find(cid) ;

          if(cit == _tree.end()) {
              _tree[cid] = new QTreeWidgetItem(QTreeWidgetItem::UserType) ;
              _tree[_node.id]->addChild(_tree[cid]) ;
            }
          else {
              _tree[_node.id]->addChild(*cit) ;
            }
        }

      childs.clear();
    }
}

void SvNavigatorTree::update(CoreDataT*& _data)
{
  clear() ;
  addTopLevelItem(_data->tree_items[ROOT_ID]) ;
  setCurrentItem(_data->tree_items[ROOT_ID]) ;
  expandAll() ;
  coreData = _data ;
}


QTreeWidgetItem * SvNavigatorTree::createTreeItem(const NodeT& _node) {

  QTreeWidgetItem *item = new QTreeWidgetItem(QTreeWidgetItem::UserType);
  item->setIcon(0, QIcon(":/images/built-in/unknown.png"));
  item->setText(0, _node.name);
  item->setData(0, QTreeWidgetItem::UserType, _node.id);
  return item;
}
