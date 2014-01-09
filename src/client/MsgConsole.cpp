/*
 * MsgPanel.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
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

#include "MsgConsole.hpp"
#include "StatsLegend.hpp"
#include "utilsClient.hpp"

const qint16 MsgConsole::ROW_MARGIN = 3;

namespace {
  const qint32 ID_COLUMN = 5;
}

MsgConsole::MsgConsole(QWidget * _parent)
  : QTableView(_parent),
    mmodel(new QStandardItemModel(0, ID_COLUMN+1, this)),
    mproxyModel(new MsgConsoleProxyModel)
{
  mmodel->setHeaderData(0, Qt::Horizontal, QObject::tr("Date & Hour"), Qt::DisplayRole);
  mmodel->setHeaderData(1, Qt::Horizontal, QObject::tr("Severity"), Qt::DisplayRole);
  mmodel->setHeaderData(2, Qt::Horizontal, QObject::tr("Host"), Qt::DisplayRole);
  mmodel->setHeaderData(3, Qt::Horizontal, QObject::tr("Service"), Qt::DisplayRole);
  mmodel->setHeaderData(4, Qt::Horizontal, QObject::tr("Message"), Qt::DisplayRole);
  mproxyModel->setSourceModel(mmodel);
  QTableView::setModel(mproxyModel);
  QTableView::verticalHeader()->hide();
  QTableView::hideColumn(ID_COLUMN);
  QTableView::setAlternatingRowColors(true);
  QTableView::setSelectionBehavior(QAbstractItemView::SelectRows);
  QTableView::setSortingEnabled(true);
  QTableView::setEditTriggers(QAbstractItemView::NoEditTriggers);
  connect(horizontalHeader(),SIGNAL(sectionClicked(int)), this, SLOT(sortByColumn(int)));
}


MsgConsole::~MsgConsole()
{
  delete mmodel;
  delete mproxyModel;
}

void MsgConsole::updateNodeMsg(const NodeListT::iterator& _node)
{
  updateNodeMsg(*_node);
}


void MsgConsole::updateNodeMsg(const NodeT& _node)
{
  const CheckT& check = _node.check;
  qint32 index = 0;
  QString itemText = "";
  qint32 nbRows = mmodel->rowCount();
  while(index < nbRows &&
        mmodel->item(index, ID_COLUMN) &&
        mmodel->item(index, ID_COLUMN)->data(Qt::UserRole) != _node.id) { ++index;}
  if(index >= nbRows) {
    index = 0;
    mmodel->insertRow(index);
    mmodel->setRowCount(nbRows + 1);
    mmodel->setItem(index, 0, new QStandardItem(itemText));
    mmodel->setItem(index, 1, new QStandardItem(itemText));
    mmodel->setItem(index, 2, new QStandardItem(itemText));
    mmodel->setItem(index, 3, new QStandardItem(itemText));
    mmodel->setItem(index, 4, new QStandardItem(itemText));
    mmodel->setItem(index, ID_COLUMN, new QStandardItem(itemText));
  }
  mmodel->item(index, ID_COLUMN)->setData(_node.id, Qt::UserRole);
  itemText = QString(check.last_state_change.c_str());
  mmodel->item(index, 0)->setText(itemText);
  mmodel->item(index, 0)->setData(QDateTime::fromTime_t(itemText.toUInt()), Qt::UserRole);

  mmodel->item(index, 1)->setText(utils::severityText(_node.severity));
  mmodel->item(index, 1)->setData(-1*_node.severity, Qt::UserRole);
  mmodel->item(index, 1)->setBackground(QBrush(utils::severityColor(_node.severity)));

  itemText = QString(check.host.c_str());
  mmodel->item(index, 2)->setText(itemText);
  mmodel->item(index, 2)->setData(itemText, Qt::UserRole);

  mmodel->item(index, 3)->setText(_node.name);
  mmodel->item(index, 3)->setData(_node.name, Qt::UserRole);

  if(_node.severity == MonitorBroker::Normal) {
    mmodel->item(index, 0)->setBackground(Qt::transparent);
    mmodel->item(index, 2)->setBackground(Qt::transparent);
    mmodel->item(index, 3)->setBackground(Qt::transparent);
    mmodel->item(index, 4)->setBackground(Qt::transparent);
  } else {
    mmodel->item(index, 0)->setBackground(StatsLegend::HIGHLIGHT_COLOR);
    mmodel->item(index, 2)->setBackground(StatsLegend::HIGHLIGHT_COLOR);
    mmodel->item(index, 3)->setBackground(StatsLegend::HIGHLIGHT_COLOR);
    mmodel->item(index, 4)->setBackground(StatsLegend::HIGHLIGHT_COLOR);

  }
  mmodel->item(index, 4)->setText(_node.actual_msg);
  mmodel->item(index, 4)->setData(_node.actual_msg, Qt::UserRole);

  if (getRowCount() == 1) updateEntriesSize(false);
}

void MsgConsole::clearMsg(const NodeT& _node)
{
  qint32 index = 0;
  qint32 nbRows = mmodel->rowCount();
  while (index < nbRows &&
         mmodel->item(index, ID_COLUMN)->data(Qt::UserRole) != _node.id) ++index;
  if (index < nbRows)
    mmodel->removeRow(index);
}

void MsgConsole::clearNormalMsg(void)
{
  qint32 index = 0;
  qint32 nbRows = mmodel->rowCount();
  while (index < nbRows) {
    if (mmodel->item(index, 1)->text() == utils::severityText(MonitorBroker::Normal)) {
      mmodel->removeRow(index);
      --nbRows;
    } else {
      ++index;
    }
  }
}

void MsgConsole::updateEntriesSize(bool _resizeWindow)
{
  if (_resizeWindow) window()->resize(m_consoleSize);
  QTableView::resizeColumnsToContents();
  QTableView::resizeRowsToContents();
  if(mmodel->rowCount()) {
    QTableView::setColumnWidth(4, QTableView::width() - (QTableView::columnWidth(0)
                                                         +QTableView::columnWidth(1)
                                                         +QTableView::columnWidth(2)
                                                         +QTableView::columnWidth(3)));
  }
}

void MsgConsole::useLargeFont(bool _toggled)
{
  if (_toggled) {
    QFont df =  font();
    setFont(QFont(df.family(), 16));
  } else {
    setFont(QFont());
  }
  updateEntriesSize(false);
  resizeRowsToContents();
}
