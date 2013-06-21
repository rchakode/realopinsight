/*
 * Ngrt4nMsgConsole.cpp
 *
 *  Created on: 20 mars 2012
 *      Author: chakode
 */

#include "include/Ngrt4nMsgConsole.hpp"

Ngrt4nMsgConsole::Ngrt4nMsgConsole()
: WTableView(0),
  row(0),
  _sizeChanged(this)
{
  setSortingEnabled(true) ;
  setLayoutSizeAware(true) ;
  setColumnResizeEnabled(true) ;
  setAlternatingRowColors(true) ;
  setSelectable(true) ;
  setSelectionMode(SingleSelection) ;
  setSelectionBehavior(SelectRows) ;
  setHeaderHeight(26) ;
  renderingModel = new WStandardItemModel(2, 5) ;
  renderingModel->setHeaderData(0, Horizontal, string("icons/built-in/tbv-date.gif"), DecorationRole) ;
  renderingModel->setHeaderData(1, Horizontal, string("icons/built-in/tbv-host.gif"), DecorationRole) ;
  renderingModel->setHeaderData(2, Horizontal, string("icons/built-in/tbv-service.gif"), DecorationRole) ;
  renderingModel->setHeaderData(3, Horizontal, string("icons/built-in/tbv-status.gif"), DecorationRole) ;
  renderingModel->setHeaderData(4, Horizontal, string("icons/built-in/tbv-message.gif"), DecorationRole) ;

  Ngrt4nSortingProxyModel* sortingProxy = new Ngrt4nSortingProxyModel(this);
  sortingProxy->setSourceModel(renderingModel);
  sortingProxy->setDynamicSortFilter(true);
  sortingProxy->setFilterRole(UserRole);
  setModel(sortingProxy) ;
  sortByColumn(3, DescendingOrder) ;
}

Ngrt4nMsgConsole::~Ngrt4nMsgConsole()
{
  delete renderingModel ;
}

void  Ngrt4nMsgConsole::layoutSizeChanged (int width, int height)
{
  WLength em = WLength(1, WLength::FontEx) ;
  setColumnWidth(0, 20 * em) ;
  setColumnWidth(1, 20 * em) ;
  setColumnWidth(2, 20 * em) ;
  setColumnWidth(3, 90) ; /*size of the header image*/
  setColumnWidth(4, width - (65 * em.toPixels() + 90)) ; /*size of the header image*/
  _sizeChanged.emit(width, height) ;
}

void Ngrt4nMsgConsole::update(const Ngrt4nListServicesT & _aservices)
{
  for(ServiceConstIteratorT srvIt = _aservices.begin(); srvIt != _aservices.end() ; srvIt++){
      addMsg(srvIt->second) ;
  }
}

void Ngrt4nMsgConsole::addMsg(const Ngrt4nServiceT &  _service)
{
  renderingModel->setItem(row, 0, createDateTimeItem(_service.probe.last_state_change)) ;
  renderingModel->setItem(row, 1, new WStandardItem(_service.probe.host)) ;
  renderingModel->setItem(row, 2, new WStandardItem(_service.name)) ;
  renderingModel->setItem(row, 3, createItatusItem(_service.probe.status)) ;
  renderingModel->setItem(row, 4, new WStandardItem(_service.msg)) ;

  row++ ;
}

WStandardItem* Ngrt4nMsgConsole::createItatusItem(const int & _status)
{
  WStandardItem * item = new WStandardItem() ;
  string icon = "icons/built-in/unknown.png" ;
  string style = "unknown" ;
  item->setData(string("3"), UserRole) ;
  if( _status == NAGIOS_OK){
      icon = "icons/built-in/normal.png" ;
      style = "normal" ;
      item->setData(string("0"), UserRole) ;
  } else  if( _status ==  NAGIOS_WARNING){
      icon = "icons/built-in/warning.png" ;
      style = "warning" ;
      item->setData(string("1"), UserRole) ;
  }  if( _status ==  NAGIOS_CRITICAL){
      style = "critical" ;
      icon = "icons/built-in/critical.png" ;
      item->setData(string("2"), UserRole) ;
  }
  item->setIcon(icon) ;
  item->setText(Ngrt4nConfigParser::statusToString(_status)) ;
  item->setStyleClass(style) ;
  return item ;
}

WStandardItem* Ngrt4nMsgConsole::createDateTimeItem(const string & _lastcheck)
{
  WStandardItem * item = new WStandardItem() ;
  long time = atoi(_lastcheck.c_str()) ;
  item->setText( ctime(&time) ) ;
  item->setData(_lastcheck, UserRole) ;
  return item ;
}
