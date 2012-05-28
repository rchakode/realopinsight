/*
 * MsgPanel.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 13-05-2012												   #
#																		   #
# This file is part of NGRT4N (http://ngrt4n.com).						   #
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

#ifndef SNAVMSGPANEL_HPP_
#define SNAVMSGPANEL_HPP_
#include "Base.hpp"



class MsgPanel : public QTableWidget
{
	Q_OBJECT

public:

	MsgPanel(QWidget * parent = 0 );
	virtual ~MsgPanel() {};


	static const qint16 msgPanelColumnCount;

	void addMsg(const NodeListT::iterator &);
	void resizeFields( const QSize & ,  const bool & = false );


public slots:
	void acknowledgeMsg(void) { emit acknowledgeChanged() ;}
	void sortEventConsole(void) {sortItems(MsgPanel::msgPanelColumnCount - 1, Qt::DescendingOrder) ;}

signals:
	void acknowledgeChanged(void) ;

protected :
void showEvent ( QShowEvent * ) ;

private:

QPoint charSize;
QSize windowSize ;

static const QStringList msgPanelHeaderLabels;

inline QCheckBox* msgItem(const qint32 & _row, const qint32 & _column) 	{
	return dynamic_cast<QCheckBox*>(cellWidget( _row, _column ) ) ;
}

};

#endif /* SNAVMSGPANEL_HPP_ */
