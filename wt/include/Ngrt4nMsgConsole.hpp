/*
 * Ngrt4nMsgConsole.hpp
 *
 *  Created on: 20 mars 2012
 *      Author: chakode
 */

#ifndef NGRT4NMSGPANEL_HPP_
#define NGRT4NMSGPANEL_HPP_
#include <Wt/WTableView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WSortFilterProxyModel>
#include "Ngrt4nCoreDataStructures.hpp"
#include "Ngrt4nConfigParser.hpp"
#include <boost/any.hpp>
using namespace Wt ;

class Ngrt4nMsgConsole : public WTableView
{
public:
  Ngrt4nMsgConsole();
  virtual
  ~Ngrt4nMsgConsole();

  WStandardItemModel* getRenderingModel(void) const {return renderingModel ;}
  void addMsg(const Ngrt4nServiceT &  _node);
  void update(const Ngrt4nListServicesT & _aservices);
  WStandardItem* createItatusItem(const int & _status) ;
  WStandardItem* createDateTimeItem(const string & _lastcheck) ;
  Wt::Signal<int, int>& sizeChanged() {return this->_sizeChanged; };

protected:
  void  layoutSizeChanged (int width, int height);

private:
  WStandardItemModel* renderingModel ;
  int row ;

  Wt::Signal<int, int> _sizeChanged ;

  class Ngrt4nSortingProxyModel :public WSortFilterProxyModel {
  public:
    Ngrt4nSortingProxyModel(WObject* _parent=0) : WSortFilterProxyModel(_parent) { }
    virtual ~Ngrt4nSortingProxyModel() {}
  protected:
   virtual bool lessThan( const WModelIndex &lhs, const WModelIndex & rhs) const {
      if( lhs.data(UserRole).empty() || rhs.data(UserRole).empty())
        return lhs < rhs ;
      return atol(boost::any_cast<string>(lhs.data(UserRole)).c_str()) <  atol(boost::any_cast<string>(rhs.data(UserRole)).c_str());
    }

  };
};

#endif /* NGRT4NMSGPANEL_HPP_ */
