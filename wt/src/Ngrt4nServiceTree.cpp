/*
 * Ngrt4nTreeView.cpp
 *
 *  Created on: 20 mars 2012
 *      Author: chakode
 */

#include "Ngrt4nServiceTree.hpp"

Ngrt4nServiceTree::Ngrt4nServiceTree()
: WTreeView(0),
  renderingModel (new WStandardItemModel())
{
  setHeaderHeight(0) ;
  setSelectionMode(SingleSelection) ;
  setSelectable(true) ;
  setSelectionBehavior(SelectItems) ;
}

Ngrt4nServiceTree::~Ngrt4nServiceTree()
{
  delete renderingModel ;
}


WStandardItem* Ngrt4nServiceTree::createItem(const Ngrt4nServiceT & _service)
{
  WStandardItem* item = new WStandardItem() ;
  item->setText(_service.name) ;
  item->setIcon("icons/built-in/unknown.png") ;
  item->setData(_service.id, UserRole) ;
  return item ;
}

