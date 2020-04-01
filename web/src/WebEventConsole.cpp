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
#include <Wt/WDateTime.h>

namespace {
  const qint32 ID_COLUMN = 5;
  const int TABLE_COLUMN_COUNT = ID_COLUMN + 1;
}

WebMsgConsole::WebMsgConsole()
  : WTableView()
{
  setSortingEnabled(true);
  setLayoutSizeAware(true);
  setColumnResizeEnabled(true);
  setSelectable(true);
  setSelectionMode(Wt::SelectionMode::Single);
  setSelectionBehavior(Wt::SelectionBehavior::Rows);
  setHeaderHeight(26);

  auto model = std::make_shared<Wt::WStandardItemModel>(0, TABLE_COLUMN_COUNT);
  m_modelRef = model.get();
  setModelHeaders();

  auto proxyModel = std::make_shared<SortingProxyModel>();
  proxyModel->setSourceModel(model);
  proxyModel->setDynamicSortFilter(true);
  proxyModel->setFilterRole(Wt::ItemDataRole::User);
  setModel(proxyModel);
}

WebMsgConsole::~WebMsgConsole(){}


void WebMsgConsole::clearAll(void)
{
  m_modelRef->clear();
  setModelHeaders();
}

void WebMsgConsole::setModelHeaders()
{
  m_modelRef->insertColumns(0, TABLE_COLUMN_COUNT);
  m_modelRef->setHeaderData(0, Wt::Orientation::Horizontal, Q_TR("Date & Hour"), Wt::ItemDataRole::Display);
  m_modelRef->setHeaderData(1, Wt::Orientation::Horizontal, Q_TR("Severity"), Wt::ItemDataRole::Display);
  m_modelRef->setHeaderData(2, Wt::Orientation::Horizontal, Q_TR("Component"), Wt::ItemDataRole::Display);
  m_modelRef->setHeaderData(3, Wt::Orientation::Horizontal, Q_TR("Monitored item"), Wt::ItemDataRole::Display);
  m_modelRef->setHeaderData(4, Wt::Orientation::Horizontal, Q_TR("Message"), Wt::ItemDataRole::Display);
  m_modelRef->setHeaderData(ID_COLUMN, Wt::Orientation::Horizontal, Q_TR("Service ID"), Wt::ItemDataRole::User);
  hideColumn(ID_COLUMN);
}

void  WebMsgConsole::layoutSizeChanged(int width, int)
{
  Wt::WLength em = Wt::WLength(1, Wt::LengthUnit::FontEx);
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
    int row = m_modelRef->rowCount();
    m_modelRef->setItem(row, 0, createDateItem(_node.check.last_state_change, row));
    m_modelRef->setItem(row, 1, ngrt4n::createSeverityStandardItem(_node));
    m_modelRef->setItem(row, 2, createItem(_node.check.host, row));
    m_modelRef->setItem(row, 3, createItem(_node.name.toStdString(), row));
    m_modelRef->setItem(row, 4, createItem(Wt::WString::fromUTF8(_node.actual_msg.toStdString()), row));
    m_modelRef->setItem(row, 5, createItem(_node.id.toStdString(), row));
  } else {
    m_modelRef->item(index, 0)->setText(ngrt4n::humanTimeText(_node.check.last_state_change));
    m_modelRef->item(index, 2)->setText(_node.check.host);
    m_modelRef->item(index, 3)->setText(_node.name.toStdString()); //optional
    m_modelRef->item(index, 4)->setText(Wt::WString::fromUTF8(_node.actual_msg.toStdString()));

    ngrt4n::updateSeverityItem(m_modelRef->item(index, 1), _node.sev);
  }
  sortByColumn(1, Wt::SortOrder::Descending);
}


std::unique_ptr<Wt::WStandardItem> WebMsgConsole::createItem(const Wt::WString& text, int row)
{
  auto item = std::make_unique<Wt::WStandardItem>(text);
  if (row & 1) {
    item->setStyleClass(ngrt4n::severityCssClass(-1));
  }
  return std::move(item);
}


std::unique_ptr<Wt::WStandardItem> WebMsgConsole::createDateItem(const std::string& _lastcheck, int row)
{
  auto item = std::make_unique<Wt::WStandardItem>();
  item->setText(ngrt4n::wHumanTimeText(_lastcheck));
  item->setData(_lastcheck, Wt::ItemDataRole::User);
  if (row & 1) {
    item->setStyleClass(ngrt4n::severityCssClass(-1));
  }
  return std::move(item);
}


int WebMsgConsole::findServiceRow(const std::string& _id)
{
  qint32 index = 0;
  qint32 nbRows = m_modelRef->rowCount();
  while(index < nbRows &&
        m_modelRef->item(index, ID_COLUMN) &&
        m_modelRef->item(index, ID_COLUMN)->text() != _id) { ++index;}

  return (index >= nbRows)? -1 : index;
}
