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
#include "WebUtils.hpp"
#include "WebMsgConsole.hpp"
#include "utilsClient.hpp"
#include <Wt/WDateTime>

namespace {
  const qint32 ID_COLUMN = 5;
}

WebMsgConsole::WebMsgConsole()
  : WTableView(0),
    m_rowCount(0)
{
  setSortingEnabled(true);
  setLayoutSizeAware(true);
  setColumnResizeEnabled(true);
  setSelectable(true);
  setSelectionMode(Wt::SingleSelection);
  setSelectionBehavior(Wt::SelectRows);
  setHeaderHeight(26);

  m_model = new Wt::WStandardItemModel(0, ID_COLUMN+1);
  m_model->setHeaderData(0,
                         Wt::Horizontal,
                         QObject::tr("Date & Hour").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(1,
                         Wt::Horizontal,
                         QObject::tr("Severity").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(2,
                         Wt::Horizontal,
                         QObject::tr("Host").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(3,
                         Wt::Horizontal,
                         QObject::tr("Service").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(4,
                         Wt::Horizontal,
                         QObject::tr("Message").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(ID_COLUMN,
                         Wt::Horizontal,
                         QObject::tr("Service ID").toStdString(),
                         Wt::UserRole);
  hideColumn(ID_COLUMN);

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

void  WebMsgConsole::layoutSizeChanged(int width, int)
{
  Wt::WLength em = Wt::WLength(1, Wt::WLength::FontEx);
  setColumnWidth(0, 20 * em);
  setColumnWidth(1, 10 * em);
  setColumnWidth(2, 20 * em);
  setColumnWidth(3, 90); /*size of the header image*/
  setColumnWidth(4, width - (55 * em.toPixels() + 90)); /*size of the header image*/
}

void WebMsgConsole::updateNodeMsgs(const NodeListT& _cnodes)
{
  for(NodeListT::ConstIterator node=_cnodes.begin(), end=_cnodes.end();
      node != end; ++node)
  {
    int index = findServiceRow(node->id.toStdString());
    if (index < 0) {
      addMsg(*node);
    } else {
      m_model->item(index, 0)->setText(ngrt4n::humanTimeText(node->check.last_state_change));
      updateSeverityItem(m_model->item(index, 1), node->severity);
      m_model->item(index, 2)->setText(node->check.host);
      m_model->item(index, 3)->setText(node->name.toStdString()); //optional
      m_model->item(index, 4)->setText(node->actual_msg.toStdString());
    }
  }
}

void WebMsgConsole::updateNodeMsg(const NodeT& _node)
{
  int index = findServiceRow(_node.id.toStdString());
  if (index < 0) {
    addMsg(_node);
  } else {
    m_model->item(index, 0)->setText(ngrt4n::humanTimeText(_node.check.last_state_change));
    updateSeverityItem(m_model->item(index, 1), _node.severity);
    m_model->item(index, 2)->setText(_node.check.host);
    m_model->item(index, 3)->setText(_node.name.toStdString()); //optional
    m_model->item(index, 4)->setText(Wt::WString::fromUTF8(_node.actual_msg.toStdString()));
  }
  sortByColumn(1, Wt::DescendingOrder);
}

void WebMsgConsole::addMsg(const NodeT&  _node)
{
  m_model->setItem(m_rowCount, 0, createDateTimeItem(_node.check.last_state_change, m_rowCount));
  m_model->setItem(m_rowCount, 1, createSeverityItem(_node));
  m_model->setItem(m_rowCount, 2, createItem(_node.check.host, m_rowCount));
  m_model->setItem(m_rowCount, 3, createItem(_node.name.toStdString(), m_rowCount));
  m_model->setItem(m_rowCount, 4, createItem(Wt::WString::fromUTF8(_node.actual_msg.toStdString()), m_rowCount));
  m_model->setItem(m_rowCount, 5, createItem(_node.id.toStdString(), m_rowCount));

  ++m_rowCount;
}

Wt::WStandardItem* WebMsgConsole::createItem(const Wt::WString& text, int row)
{
  Wt::WStandardItem* item = new Wt::WStandardItem(text);
  if (row & 1) item->setStyleClass(ngrt4n::severityCssClass(-1));
  return item;
}

Wt::WStandardItem* WebMsgConsole::createSeverityItem(const NodeT& _node)
{
  Wt::WStandardItem* item = new Wt::WStandardItem();
  item->setData(QString::number(_node.severity).toStdString(), Wt::UserRole);
  item->setText(ngrt4n::severityText(_node.severity).toStdString());
  updateSeverityItem(item, _node.severity);
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


void WebMsgConsole::updateSeverityItem(Wt::WStandardItem* item, int severity)
{
  item->setText(ngrt4n::severityText(severity).toStdString());
  item->setStyleClass(ngrt4n::severityCssClass(severity));
}
