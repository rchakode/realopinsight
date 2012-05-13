/*
 * MsgPanel.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 13-05-2012												   #
#																		   #
# This file is part of NGRT4N (http://ngrt4n.com).	   					   #
#																		   #
# NGRT4N is free software: you can redistribute it and/or modify		   #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.									   #
#																		   #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of		   #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	           #
# GNU General Public License for more details.							   #
#																		   #
# You should have received a copy of the GNU General Public License		   #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.		   #
#--------------------------------------------------------------------------#
 */

#include "../include/MsgPanel.hpp"

const qint16 MsgPanel::msgPanelColumnCount = 7;

const QStringList MsgPanel::msgPanelHeaderLabels =
		QStringList() <<"Date & Hour"
		<<"Status"
		<<"Host"
		<<"Service"
		<<"Message"
		;

MsgPanel::MsgPanel(QWidget * _parent)
: QTableWidget( 0, msgPanelColumnCount, _parent ),
  charSize(QPoint(QFontMetrics(QFont()).charWidth("c", 0), QFontMetrics(QFont()).height()))
  {
	verticalHeader()->hide() ;
	hideColumn( msgPanelColumnCount - 2 ) ;
	hideColumn( msgPanelColumnCount - 1 ) ;
	setHorizontalHeaderLabels( msgPanelHeaderLabels );
	setAlternatingRowColors( true ) ;
  }


void MsgPanel::showEvent ( QShowEvent * )
{
	sortItems(msgPanelColumnCount - 1);
}

void MsgPanel::addMsg(const NodeListT::iterator & _node_it)
{
	const qint32 id_column = msgPanelColumnCount - 2 ;
	const qint32 date_column = msgPanelColumnCount - 1 ;

	time_t i_time ;
	qint32 i, row_count ;
	QString line[ msgPanelColumnCount ], s_time ;
	QStringList str_list ;
	QTableWidgetItem* row_items[msgPanelColumnCount] ;

	setSortingEnabled( false ) ;

	i_time = _node_it->check.last_state_change.toLong(), s_time = ctime(&i_time) ;
	str_list = _node_it->check.id.split("/") ;

	line[0] = s_time.replace("\n", "") ;
	line[1] = Utils::statusToString(_node_it->status) ;
	line[2] = ( str_list.length() )?  str_list[0] : "" ;
	line[3] = " " + _node_it->name ;

	if( _node_it->status == NAGIOS_OK ){
		line[4] = _node_it->notification_msg.length() ? _node_it->notification_msg :  _node_it->check.alarm_msg ;
	}
	else {
		line[4] = _node_it->alarm_msg.length() ? _node_it->alarm_msg :  _node_it->check.alarm_msg ;
	}

	line[id_column] = _node_it->id ;
	line[date_column] = _node_it->check.last_state_change ;

	i = 0 ;
	row_count = rowCount();
	while( i < row_count )
	{
		if ( item(i, id_column)->text() != _node_it->id )
		{
			i ++ ;
			continue ;
		}
		removeRow(i) ;
		row_count -- ;
	}

	insertRow( 0 ) ;
	setRowCount( row_count + 1) ;
	setRowHeight(0, charSize.y() + 3) ;

	for(i = 0; i < msgPanelColumnCount ; i ++)
	{
		setCellWidget(0, i, new QLabel( "" ) ) ;
		row_items[i] = new QTableWidgetItem(line[i]) ;
		setItem(0, i, row_items[i]) ;
		if( _node_it->status != NAGIOS_OK )
		{
			item(0, i)->setBackground(HIGHLIGHT_COLOR) ;
		}
	}

	switch(_node_it->status)
	{
	case NAGIOS_OK:
		item(0, 1)->setBackground(QBrush(OK_COLOR)) ;
		break;

	case NAGIOS_WARNING:
		item(0, date_column)->setText(QString::number(-1 * NAGIOS_WARNING)) ;
		item(0, 1)->setBackground(QBrush(WARNING_COLOR)) ;
		break;

	case NAGIOS_CRITICAL:
		item(0, date_column)->setText(QString::number(-1 * NAGIOS_CRITICAL)) ;
		item(0, 1)->setBackground(QBrush(CRITICAL_COLOR)) ;
		break;

	case NAGIOS_UNKNOWN:
		item(0, date_column)->setText(QString::number(-1 * NAGIOS_UNKNOWN)) ;
		item(0, 1)->setBackground(QBrush(UNKNOWN_COLOR)) ;
		break;

	default:
		break;
	}
	sortItems(MsgPanel::msgPanelColumnCount - 1, Qt::DescendingOrder) ;
}

void MsgPanel::resizeFields( const QSize & _window_size, const bool & _resize_window )
{
	qint32 msg_width ;

	resizeColumnsToContents() ;

	if( rowCount() )
	{
		msg_width = (_window_size.width() - cellWidget(0, 4)->pos().x() ) ;
		setColumnWidth(4, msg_width ) ;
	}
	if ( _resize_window ) window()->resize( _window_size ) ;
}
