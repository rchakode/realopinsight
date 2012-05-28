/*
 * SvNavigatorTree.hpp
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

#ifndef SNAVSVNAVIGATORTREE_H_
#define SNAVSVNAVIGATORTREE_H_

#include "Base.hpp"



class SvNavigatorTree : public QTreeWidget
{
	Q_OBJECT

public:
	SvNavigatorTree(const bool & =false, QWidget* = 0 );
    static void addNode( TreeNodeItemListT & , const NodeT &, const bool & = false );
	void update(Struct * & _snav_struct) ;

	static const QString rootID ;
	Struct* ptr2MainStruct ;

signals:
	void treeNodeMoved( QString _node_id ) ;

protected:
	void showEvent(QShowEvent *) ;
	void dropEvent(QDropEvent *) ;
	void startDrag(Qt::DropActions)  ;

private:
	QString selectedNode ;

};

#endif /* SNAVSVNAVIGATORTREE_H_ */
