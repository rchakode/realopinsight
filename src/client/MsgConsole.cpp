/*
 * MsgPanel.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */

#include "MsgConsole.hpp"
#include "StatsLegend.hpp"
#include <ctime>
#include "utilsClient.hpp"

const QString MsgConsole::TAG_HOSTNAME = "\\{hostname\\}";
const QString MsgConsole::TAG_HOSTNAME_ZABBIX = "\\{HOST.NAME\\}";
const QString MsgConsole::TAG_CHECK = "\\{check_name\\}";
const QString MsgConsole::TAG_THERESHOLD = "\\{threshold\\}";
const QString MsgConsole::TAG_PLUGIN_OUTPUT = "\\{plugin_output\\}";
const qint16 MsgConsole::NUM_COLUMNS = 6;
const qint32 ID_COLUMN = MsgConsole::NUM_COLUMNS - 1;

MsgConsole::MsgConsole(QWidget * _parent)
  : QTableView(_parent),
    mmodel(new QStandardItemModel(0, NUM_COLUMNS, this)),
    mproxyModel(new MsgConsoleProxyModel),
    charSize(QPoint(QFontMetrics(QFont()).charWidth("c", 0), QFontMetrics(QFont()).height()))
{
  mmodel->setHeaderData(0, Qt::Horizontal, QObject::tr("Date & Hour"), Qt::DisplayRole);
  mmodel->setHeaderData(1, Qt::Horizontal, QObject::tr("Criticity"), Qt::DisplayRole);
  mmodel->setHeaderData(2, Qt::Horizontal, QObject::tr("Host"), Qt::DisplayRole);
  mmodel->setHeaderData(3, Qt::Horizontal, QObject::tr("Service"), Qt::DisplayRole);
  mmodel->setHeaderData(4, Qt::Horizontal, QObject::tr("Message"), Qt::DisplayRole);
  mproxyModel->setSourceModel(mmodel);
  QTableView::setModel(mproxyModel);
  QTableView::verticalHeader()->hide();
  QTableView::hideColumn(NUM_COLUMNS - 1);
  QTableView::setAlternatingRowColors(true);
  QTableView::setSelectionBehavior(QAbstractItemView::SelectRows);
  QTableView::setSortingEnabled(true);
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
  const MonitorBroker::CheckT& check = _node.check;
  qint32 index = 0;
  QString itemText = "";
  qint32 nbRows = mmodel->rowCount();
  while(index < nbRows &&
        mmodel->item(index, ID_COLUMN) &&
         mmodel->item(index, ID_COLUMN)->data(Qt::UserRole) != _node.id) { index++;}
  if(index >= nbRows) {
      index = 0;
      mmodel->insertRow(index);
      mmodel->setRowCount(nbRows + 1);
      QTableView::setRowHeight(index, charSize.y() + 3);
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
  mmodel->item(index, 0)->setData(QDateTime::fromString(itemText), Qt::UserRole);

  mmodel->item(index, 1)->setText(utils::criticityToText(_node.criticity));
  mmodel->item(index, 1)->setData(-1*_node.criticity, Qt::UserRole);
  mmodel->item(index, 1)->setBackground(QBrush(utils::computeColor(_node.criticity)));

  itemText = QString(check.host.c_str());
  mmodel->item(index, 2)->setText(itemText);
  mmodel->item(index, 2)->setData(itemText, Qt::UserRole);

  mmodel->item(index, 3)->setText(_node.name);
  mmodel->item(index, 3)->setData(_node.name, Qt::UserRole);

  if(_node.criticity == MonitorBroker::CriticityNormal) {
      itemText = (_node.notification_msg.trimmed().length() != 0)?
            _node.notification_msg:QString(_node.check.alarm_msg.c_str());
      mmodel->item(index, 0)->setBackground(Qt::transparent);
      mmodel->item(index, 2)->setBackground(Qt::transparent);
      mmodel->item(index, 3)->setBackground(Qt::transparent);
      mmodel->item(index, 4)->setBackground(Qt::transparent);
    } else {
      itemText = (_node.alarm_msg.trimmed().length() &&
                  _node.criticity != MonitorBroker::CriticityUnknown)?
            _node.alarm_msg: QString(_node.check.alarm_msg.c_str());
      mmodel->item(index, 0)->setBackground(StatsLegend::HIGHLIGHT_COLOR);
      mmodel->item(index, 2)->setBackground(StatsLegend::HIGHLIGHT_COLOR);
      mmodel->item(index, 3)->setBackground(StatsLegend::HIGHLIGHT_COLOR);
      mmodel->item(index, 4)->setBackground(StatsLegend::HIGHLIGHT_COLOR);

    }
  mmodel->item(index, 4)->setText(itemText);
  mmodel->item(index, 4)->setData(itemText, Qt::UserRole);
}

void MsgConsole::updateColumnWidths(const QSize& _windowSize, const bool& _resizeWindow)
{
  if (_resizeWindow) window()->resize(_windowSize);
  QTableView::resizeColumnsToContents();
  if(mmodel->rowCount()) {
      qint32 msgWidth = QTableView::width() - (QTableView::columnWidth(0)
                                               +QTableView::columnWidth(1)
                                               +QTableView::columnWidth(2)
                                               +QTableView::columnWidth(3));
      QTableView::setColumnWidth(4, msgWidth);
    }

}
