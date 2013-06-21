/*
 * Ngrt4nTreeView.hpp
 *
 *  Created on: 20 mars 2012
 *      Author: chakode
 */

#ifndef NGRT4NSERVICETREE_HPP_
#define NGRT4NSERVICETREE_HPP_


#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WModelIndex>
#include "Ngrt4nCoreDataStructures.hpp"

using namespace Wt ;

class Ngrt4nServiceTree : public WTreeView
{
public:
  Ngrt4nServiceTree();
  virtual
  ~Ngrt4nServiceTree();

  WStandardItemModel* getRenderingModel(void) const {return renderingModel ;}
  void setRenderingModel(WStandardItem* _item) const {return renderingModel->appendRow(_item) ;}
  static WStandardItem* createItem(const Ngrt4nServiceT & _service) ;

  void update(void) { setModel(renderingModel) ; expandToDepth(2) ;} //TODO check before
  void update(WStandardItem * _rItem) { setRenderingModel(_rItem) ; update() ; }

private:
  WStandardItemModel* renderingModel ;

};

#endif /* NGRT4NSERVICETREE_HPP_ */
