/*
 * WebMsgConsole.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 23-03-2014                                                  #
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

#include "WebUtils.hpp"
#include "WebEventConsole.hpp"
#include "utilsCore.hpp"
#include <Wt/WDateTime>

namespace {
  const qint32 ID_COLUMN = 5;
  const int TABLE_COLUMN_COUNT = ID_COLUMN + 1;
}

WebMsgConsole::WebMsgConsole()
  : WTableView(0)
{
  setSortingEnabled(true);
  setLayoutSizeAware(true);
  setColumnResizeEnabled(true);
  setSelectable(true);
  setSelectionMode(Wt::SingleSelection);
  setSelectionBehavior(Wt::SelectRows);
  setHeaderHeight(26);

  m_model = new Wt::WStandardItemModel(0, TABLE_COLUMN_COUNT);
  setModelHeaders();
  setModel();
}

WebMsgConsole::~WebMsgConsole()
{
  delete m_model;
}

void WebMsgConsole::setModel(void)
{
  SortingProxyModel* sproxy = new SortingProxyModel(this);
  sproxy->setSourceModel(m_model);
  sproxy->setDynamicSortFilter(true);
  sproxy->setFilterRole(Wt::UserRole);
  WTableView::setModel(sproxy);
}

void WebMsgConsole::setModelHeaders(void)
{
  m_model->insertColumns(0, TABLE_COLUMN_COUNT);
  m_model->setHeaderData(0, Wt::Horizontal, Q_TR("Date & Hour"), Wt::DisplayRole);
  m_model->setHeaderData(1, Wt::Horizontal, Q_TR("Severity"), Wt::DisplayRole);
  m_model->setHeaderData(2, Wt::Horizontal, Q_TR("Component"), Wt::DisplayRole);
  m_model->setHeaderData(3, Wt::Horizontal, Q_TR("Monitored item"), Wt::DisplayRole);
  m_model->setHeaderData(4, Wt::Horizontal, Q_TR("Message"), Wt::DisplayRole);
  m_model->setHeaderData(ID_COLUMN, Wt::Horizontal, Q_TR("Service ID"), Wt::UserRole);
  hideColumn(ID_COLUMN);
}

void  WebMsgConsole::layoutSizeChanged(int width, int)
{
  Wt::WLength em = Wt::WLength(1, Wt::WLength::FontEx);
  setColumnWidth(0, 25 * em);
  setColumnWidth(1, 10 * em);
  setColumnWidth(2, 30 * em);
  setColumnWidth(3, 30 * em);
  setColumnWidth(4, width - (55 * em.toPixels() + 90)); /*size of the header image*/
}



void WebMsgConsole::updateNodeMsg(const NodeT& _node)
{
  int index = findServiceRow(_node.id.toStdString());
  if (index < 0) {
    int row = m_model->rowCount();
    m_model->setItem(row, 0, createDateTimeItem(_node.check.last_state_change, row));
    m_model->setItem(row, 1, ngrt4n::createSeverityStandardItem(_node));
    m_model->setItem(row, 2, createItem(_node.check.host, row));
    m_model->setItem(row, 3, createItem(_node.name.toStdString(), row));
    m_model->setItem(row, 4, createItem(Wt::WString::fromUTF8(_node.actual_msg.toStdString()), row));
    m_model->setItem(row, 5, createItem(_node.id.toStdString(), row));
  } else {
    m_model->item(index, 0)->setText(ngrt4n::humanTimeText(_node.check.last_state_change));
    m_model->item(index, 2)->setText(_node.check.host);
    m_model->item(index, 3)->setText(_node.name.toStdString()); //optional
    m_model->item(index, 4)->setText(Wt::WString::fromUTF8(_node.actual_msg.toStdString()));

    ngrt4n::updateSeverityItem(m_model->item(index, 1), _node.sev);
  }
  sortByColumn(1, Wt::DescendingOrder);
}


Wt::WStandardItem* WebMsgConsole::createItem(const Wt::WString& text, int row)
{
  Wt::WStandardItem* item = new Wt::WStandardItem(text);
  if (row & 1) item->setStyleClass(ngrt4n::severityCssClass(-1));
  return item;
}


Wt::WStandardItem* WebMsgConsole::createDateTimeItem(const std::string& _lastcheck, int row)
{
  Wt::WStandardItem * item = new Wt::WStandardItem();
  item->setText(ngrt4n::wHumanTimeText(_lastcheck));
  item->setData(_lastcheck, Wt::UserRole);
  if (row & 1) item->setStyleClass(ngrt4n::severityCssClass(-1));
  return item;
}


int WebMsgConsole::findServiceRow(const std::string& _id)
{
  qint32 index = 0;
  qint32 nbRows = m_model->rowCount();
  while(index < nbRows &&
        m_model->item(index, ID_COLUMN) &&
        m_model->item(index, ID_COLUMN)->text() != _id) { ++index;}

  return (index >= nbRows)? -1 : index;
}
