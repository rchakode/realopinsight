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
    m_rowCount(0)
{
  setSortingEnabled(true);
  setLayoutSizeAware(true);
  setColumnResizeEnabled(true);
  setAlternatingRowColors(true);
  setSelectable(true);
  setSelectionMode(Wt::SingleSelection);
  setSelectionBehavior(Wt::SelectRows);
  setHeaderHeight(26);
  m_model = new Wt::WStandardItemModel(2, 5);
  //    m_model->setHeaderData(0, Wt::Horizontal,
  //                                  std::string("images/built-in/tbv-date.gif"),
  //                                  Wt::DecorationRole);
  //    m_model->setHeaderData(1,Wt::Horizontal,
  //                                  std::string("images/built-in/tbv-host.gif"),
  //                                  Wt::DecorationRole);
  //    m_model->setHeaderData(2, Wt::Horizontal,
  //                                  std::string("images/built-in/tbv-service.gif"),
  //                                  Wt::DecorationRole);
  //    m_model->setHeaderData(3, Wt::Horizontal,
  //                                  std::string("images/built-in/tbv-status.gif"),
  //                                  Wt::DecorationRole);
  //    m_model->setHeaderData(4, Wt::Horizontal,
  //                                  std::string("images/built-in/tbv-message.gif"),
  //                                  Wt::DecorationRole);

  m_model->setHeaderData(0,
                         Wt::Horizontal,
                         QObject::tr("Timestamp").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(1,
                         Wt::Horizontal,
                         QObject::tr("Hostname").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(2,
                         Wt::Horizontal,
                         QObject::tr("Service").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(3,
                         Wt::Horizontal,
                         QObject::tr("Status").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(4,
                         Wt::Horizontal,
                         QObject::tr("Details").toAscii(),
                         Wt::DisplayRole);

  SortingProxyModel* sortingProxy = new SortingProxyModel(this);
  sortingProxy->setSourceModel(m_model);
  sortingProxy->setDynamicSortFilter(true);
  sortingProxy->setFilterRole(Wt::UserRole);
  setModel(sortingProxy);
  sortByColumn(3, Wt::DescendingOrder);
}

WebMsgConsole::~WebMsgConsole()
{
  delete m_model;
}

void  WebMsgConsole::layoutSizeChanged(int width, int)
{
  Wt::WLength em = Wt::WLength(1, Wt::WLength::FontEx);
  setColumnWidth(0, 20 * em);
  setColumnWidth(1, 20 * em);
  setColumnWidth(2, 20 * em);
  setColumnWidth(3, 90); /*size of the header image*/
  setColumnWidth(4, width - (65 * em.toPixels() + 90)); /*size of the header image*/
  //emit sizeChanged(width, height);
}

void WebMsgConsole::update(const NodeListT& _cnodes)
{
  for(NodeListT::ConstIterator node = _cnodes.begin(); node != _cnodes.end(); node++){
    addMsg(*node);
  }
}

void WebMsgConsole::addMsg(const NodeT&  _node)
{
  m_model->setItem(m_rowCount, 0, createDateTimeItem(_node.check.last_state_change));
  m_model->setItem(m_rowCount, 1, new Wt::WStandardItem(_node.check.host));
  m_model->setItem(m_rowCount, 2, new Wt::WStandardItem(_node.name.toStdString()));
  m_model->setItem(m_rowCount, 3, createItatusItem(_node.check.status));
  m_model->setItem(m_rowCount, 4, new Wt::WStandardItem(_node.alarm_msg.toStdString()));

  ++m_rowCount;
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
