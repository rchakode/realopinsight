/*
 * WebMsgConsole.cpp
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

#include "WebMsgConsole.hpp"

WebMsgConsole::WebMsgConsole()
  : WTableView(0),
    row(0),
    _sizeChanged(this)
{
  setSortingEnabled(true);
  setLayoutSizeAware(true);
  setColumnResizeEnabled(true);
  setAlternatingRowColors(true);
  setSelectable(true);
  setSelectionMode(Wt::SingleSelection);
  setSelectionBehavior(Wt::SelectRows);
  setHeaderHeight(26);
  renderingModel = new Wt::WStandardItemModel(2, 5);
  renderingModel->setHeaderData(0, Wt::Horizontal,
                                std::string("images/built-in/tbv-date.gif"),
                                Wt::DecorationRole);
  renderingModel->setHeaderData(1,Wt::Horizontal,
                                std::string("images/built-in/tbv-host.gif"),
                                Wt::DecorationRole);
  renderingModel->setHeaderData(2, Wt::Horizontal,
                                std::string("images/built-in/tbv-service.gif"),
                                Wt::DecorationRole);
  renderingModel->setHeaderData(3, Wt::Horizontal,
                                std::string("images/built-in/tbv-status.gif"),
                                Wt::DecorationRole);
  renderingModel->setHeaderData(4, Wt::Horizontal,
                                std::string("images/built-in/tbv-message.gif"),
                                Wt::DecorationRole);

  SortingProxyModel* sortingProxy = new SortingProxyModel(this);
  sortingProxy->setSourceModel(renderingModel);
  sortingProxy->setDynamicSortFilter(true);
  sortingProxy->setFilterRole(Wt::UserRole);
  setModel(sortingProxy);
  sortByColumn(3, Wt::DescendingOrder);
}

WebMsgConsole::~WebMsgConsole()
{
  delete renderingModel;
}

void  WebMsgConsole::layoutSizeChanged (int width, int height)
{
  Wt::WLength em = Wt::WLength(1, Wt::WLength::FontEx);
  setColumnWidth(0, 20 * em);
  setColumnWidth(1, 20 * em);
  setColumnWidth(2, 20 * em);
  setColumnWidth(3, 90); /*size of the header image*/
  setColumnWidth(4, width - (65 * em.toPixels() + 90)); /*size of the header image*/
  //sizeChanged.emit(width, height);
  std::cerr << "FIXME: " << height << "\n";
}

void WebMsgConsole::update(const NodeListT& _aservices)
{
  for(NodeListT::ConstIterator srvIt = _aservices.begin(); srvIt != _aservices.end(); srvIt++){
    addMsg(*srvIt);
  }
}

void WebMsgConsole::addMsg(const NodeT&  _service)
{
  renderingModel->setItem(row, 0, createDateTimeItem(_service.check.last_state_change));
  renderingModel->setItem(row, 1, new Wt::WStandardItem(_service.check.host));
  renderingModel->setItem(row, 2, new Wt::WStandardItem(_service.name.toStdString()));
  renderingModel->setItem(row, 3, createItatusItem(_service.check.status));
  renderingModel->setItem(row, 4, new Wt::WStandardItem(_service.alarm_msg.toStdString()));

  ++row;
}

Wt::WStandardItem* WebMsgConsole::createItatusItem(const int& _status)
{
  Wt::WStandardItem * item = new Wt::WStandardItem();
  std::string icon = "images/built-in/unknown.png";
  std::string style = "unknown";
  item->setData(std::string("3"), Wt::UserRole);
  if(_status == MonitorBroker::NagiosOk){
    icon = "images/built-in/normal.png";
    style = "normal";
    item->setData(std::string("0"), Wt::UserRole);
  } else  if(_status == MonitorBroker::NagiosWarning){
    icon = "images/built-in/warning.png";
    style = "warning";
    item->setData(std::string("1"), Wt::UserRole);
  }  if(_status == MonitorBroker::NagiosCritical){
    style = "critical";
    icon = "images/built-in/critical.png";
    item->setData(std::string("2"), Wt::UserRole);
  }
  item->setIcon(icon);
  //FIXME: item->setText(Ngrt4nConfigParser::statusToString(_status));
  item->setStyleClass(style);
  return item;
}

Wt::WStandardItem* WebMsgConsole::createDateTimeItem(const std::string& _lastcheck)
{
  Wt::WStandardItem * item = new Wt::WStandardItem();
  long time = atoi(_lastcheck.c_str());
  item->setText( ctime(&time) );
  item->setData(_lastcheck, Wt::UserRole);
  return item;
}
