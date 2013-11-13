/*
 * WebServiceTree.cpp
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

#include "WebTree.hpp"

Ngrt4nServiceTree::Ngrt4nServiceTree()
  : Wt::WTreeView(0),
  renderingModel (new Wt::WStandardItemModel())
{
  setHeaderHeight(0);
  setSelectionMode(Wt::SingleSelection);
  setSelectable(true);
  setSelectionBehavior(Wt::SelectItems);
}

Ngrt4nServiceTree::~Ngrt4nServiceTree()
{
  delete renderingModel;
}


Wt::WStandardItem* Ngrt4nServiceTree::createItem(const NodeT& _service)
{
  Wt::WStandardItem* item = new Wt::WStandardItem();
  item->setText(_service.name.toStdString());
//  item->setIcon("icons/built-in/unknown.png");
//  item->setData(_service.id, Wt::UserRole);
  return item;
}

