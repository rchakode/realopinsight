/*
 * SvNavigatorTree.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 13-05-2012												   #
#																		   #
# This file is part of NGRT4N (http://ngrt4n.com).						   #
#																		   #
# NGRT4N is free software: you can redistribute it and/or modify		   #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.									   #
#																		   #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of		   #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	           #
# GNU General Public License for more details.							   #
#																		   #
# You should have received a copy of the GNU General Public License		   #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.		   #
#--------------------------------------------------------------------------#
 */

#include "../include/SvNavigatorTree.hpp"
#include <QtGui>

const QString SvNavigatorTree::rootID = "root";

SvNavigatorTree::SvNavigatorTree(const bool & _enable_drag, QWidget* _parent)
: QTreeWidget(_parent), ptr2MainStruct(NULL)
  {
	setHeaderLabel("TV Explorer") ;
	setColumnCount(1) ;
	setDragDropMode(QAbstractItemView::DragDrop) ;
	setDragEnabled( _enable_drag ) ;
  }

void SvNavigatorTree::showEvent(QShowEvent*)
{
	expandAll() ;
}

void SvNavigatorTree::dropEvent(QDropEvent * _event )
{
	NodeListT::iterator node_it ;
	QTreeWidgetItem* tnode ;
	QString dest_tnode_id ;

	tnode = itemAt( _event->pos() ) ;
	if( tnode && ptr2MainStruct )
	{
		node_it = ptr2MainStruct->node_list.find( tnode->text(TREE_NODE_ID_COLUMN) ) ;
		if( node_it != ptr2MainStruct->node_list.end() )
		{
			if( node_it->type != ALARM_NODE )
			{
				_event->setDropAction( Qt::MoveAction ) ;

				QTreeWidget::dropEvent( _event ) ;

				emit treeNodeMoved(selectedNode) ;
			}
			else
			{
				QMessageBox::warning(this, "Warning! | " + APP_SHORT_NAME,
						"Dropping not allowed on the target node", QMessageBox::Ok) ;
			}
		}
	}
}

void SvNavigatorTree::startDrag(Qt::DropActions _drag_action)
{
	QList<QTreeWidgetItem*> items ;

	items = selectedItems() ;
	if( items.length() ) selectedNode = items[0]->text(TREE_NODE_ID_COLUMN) ;
	QTreeWidget::startDrag(_drag_action) ;
}

void SvNavigatorTree::addNode(TreeNodeItemListT & _service_tree,
		const NodeT & _node, const bool & _first_insertion)
{
	//TODO add tooltips
	QString child_node_id ;
	QStringList child_nodes_list ;
	QStringList::iterator uds_it ;
	TreeNodeItemListT::iterator node_it, child_node_it, parent_node_it ;
	QTreeWidgetItem * item ;

	node_it = _service_tree.find( _node.id ) ;
	if( node_it == _service_tree.end() )
	{
		item = new QTreeWidgetItem( QTreeWidgetItem::UserType ) ;
		item->setIcon(0, QIcon(":/images/unknown.png")) ;
		item->setText(0, _node.name) ;
		item->setText(1, _node.id) ; // Not show in UI,  useful for handling events

		if( _first_insertion )
		{
			parent_node_it = _service_tree.find( _node.parent ) ;
			if( parent_node_it != _service_tree.end() )
			{
				_service_tree[_node.parent]->addChild(item);
			}
		}

		_service_tree[_node.id] = item ;
	}
	else
	{
		(*node_it)->setIcon(0, QIcon(":/images/unknown.png")) ;
		(*node_it)->setText(0, _node.name) ;
		(*node_it)->setText(1, _node.id) ; // Not show in UI,  useful for handling events
	}

	if( _node.type != ALARM_NODE && _node.child_nodes != "" )
	{
		child_nodes_list = _node.child_nodes.split( CHILD_NODES_SEP );

		for( uds_it = child_nodes_list.begin(); uds_it != child_nodes_list.end(); uds_it++ )
		{
			child_node_id = (*uds_it).trimmed() ;
			child_node_it = _service_tree.find( child_node_id ) ;

			if( child_node_it == _service_tree.end() )
			{
				_service_tree[child_node_id] = new QTreeWidgetItem(QTreeWidgetItem::UserType) ;
				_service_tree[_node.id]->addChild( _service_tree[child_node_id] ) ;
			}
			else
			{
				_service_tree[_node.id]->addChild( *child_node_it ) ;
			}
		}

		child_nodes_list.clear();
	}
}


void SvNavigatorTree::update(Struct* & _snav_struct)
{
	clear() ;
	addTopLevelItem( _snav_struct->tree_item_list[_snav_struct->root_id] ) ;
	setCurrentItem( _snav_struct->tree_item_list[_snav_struct->root_id] ) ;
	expandAll() ;
	ptr2MainStruct = _snav_struct ;
}

