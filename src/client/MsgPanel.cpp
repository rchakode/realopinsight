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

#include "MsgPanel.hpp"
#include "StatsLegend.hpp"
#include <ctime>
#include "Utils.hpp"

const qint16 MsgPanel::msgPanelColumnCount = 7;

//EVENT CONSOLE META-MESSAGES
const QString MsgPanel::HOSTNAME_META_MSG_PATERN = "\\{hostname\\}";
const QString MsgPanel::SERVICE_META_MSG_PATERN = "\\{check_name\\}";
const QString MsgPanel::THERESHOLD_META_MSG_PATERN = "\\{threshold\\}";
const QString MsgPanel::PLUGIN_OUTPUT_META_MSG_PATERN = "\\{plugin_output\\}";

const QStringList MsgPanel::msgPanelHeaderLabels = QStringList() <<"Date & Hour"<<"Status"<<"Host"<<"Service"<<"Message";

MsgPanel::MsgPanel(QWidget * _parent)
    : QTableWidget(0, msgPanelColumnCount, _parent),
      charSize(QPoint(QFontMetrics(QFont()).charWidth("c", 0),
                      QFontMetrics(QFont()).height()))
{
    verticalHeader()->hide();
    hideColumn(msgPanelColumnCount - 2);
    hideColumn(msgPanelColumnCount - 1);
    setHorizontalHeaderLabels(msgPanelHeaderLabels);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(horizontalHeader(),SIGNAL(sectionClicked(int)), this, SLOT(sortByColumn(int)));
}


void MsgPanel::showEvent (QShowEvent *)
{
    sortItems(1, Qt::AscendingOrder); //TODO
}

void MsgPanel::addMsg(const NodeListT::iterator & _node)
{
    const qint32 idColumn = msgPanelColumnCount - 2;
    const qint32 dateColumn = msgPanelColumnCount - 1;


    time_t time = atol(_node->check.last_state_change.c_str());
    QString time_ = ctime(&time);
    QString line[msgPanelColumnCount];
    line[0] = time_.remove("\n");
    line[1] = Utils::statusToString(_node->criticity);
    line[2] = QString(_node->check.host.c_str());
    line[3] = " " + _node->name;

    if(_node->criticity == MonitorBroker::CRITICITY_NORMAL) {
        line[4] = (_node->notification_msg.trimmed().length() != 0) ? _node->notification_msg : QString(_node->check.alarm_msg.c_str());
    } else {
        line[4] = (_node->alarm_msg.trimmed().length() != 0 && _node->criticity != MonitorBroker::NAGIOS_UNKNOWN)? _node->alarm_msg :  QString(_node->check.alarm_msg.c_str());
    }

    line[idColumn] = _node->id;
    line[dateColumn] = "";

    qint32 i = 0;
    qint32 nbRows = this->rowCount();
    //    setSortingEnabled(false);
    while(i < nbRows) {
        if(item(i, idColumn)->text() != _node->id) {
            i++;
            continue;
        }
        removeRow(i);
        nbRows--;
    }

    insertRow(0);
    setRowCount(nbRows + 1);
    setRowHeight(0, charSize.y() + 3);

    //TODO deal with data for sorting
    QTableWidgetItem* items[msgPanelColumnCount];
    for(i = 0; i < msgPanelColumnCount; i ++) {
        setCellWidget(0, i, new QLabel(""));
        items[i] = new QTableWidgetItem(line[i]);
        items[i]->setData(Qt::UserRole, line[i]);
        setItem(0, i, items[i]);
        if(_node->criticity != MonitorBroker::CRITICITY_NORMAL) {
            item(0, i)->setBackground(StatsLegend::HIGHLIGHT_COLOR);
        }
    }

    item(0, 1)->setBackground(QBrush(Utils::getColor(_node->criticity)));
    //TODO: check the default sorting
    //SortItems(MsgPanel::msgPanelColumnCount - 1, Qt::DescendingOrder);
}

void MsgPanel::resizeFields(const QSize & _window_size, const bool & _resize_window)
{
    resizeColumnsToContents();

    if(rowCount()) {
        qint32  msgWidth = (_window_size.width() - cellWidget(0, 4)->pos().x());
        setColumnWidth(4, msgWidth);
    }
    if (_resize_window) window()->resize(_window_size);
}
