/*
 * SvNavigator.hpp
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

#ifndef SNAV_H_
#define SNAV_H_

#include "Base.hpp"
#include "Stats.hpp"
#include "Parser.hpp"
#include "WebKit.hpp"
#include "MsgPanel.hpp"
#include "GraphView.hpp"
#include "SvNavigatorTree.hpp"
#include "PreferencesDialog.hpp"


class SvNavigator : public QMainWindow
{
	Q_OBJECT

public:
	SvNavigator( const qint32 & _user_role = OP_USER_ROLE, const QString & = "", QWidget* = 0);

	virtual ~SvNavigator();

	void load( void );
	void resize(void);


public slots:
	int monitor(void) ;
	void updateNodeStatus( QString ) ;
	void expandNode( const QString &, const bool &, const qint32 &) ;
	void centerGraphOnNode( const QString & _node_id = "") ;
	void filterNodeRelatedMsg(void) ;
	void filterNodeRelatedMsg( const QString & ) ;
	void acknowledge(void) ;
	void tabChanged( int ) ;
	void hideChart(void) ;
	void centerGraphOnNode( QTreeWidgetItem * ) ;
	void handleChangePasswordAction(void) ;
	void handleChangeMonitoringSettingsAction(void) ;
	void handleShowOnlineResources(void) ;
	void handleShowAbout(void) ;


signals:
	void hasToBeUpdate( QString ) ;
	void sortEventConsole(void) ;


protected :
	void closeEvent(QCloseEvent * ) ;
	void contextMenuEvent(QContextMenuEvent * ) ;
	void timerEvent(QTimerEvent *) ;


private:

QString configFile ;
QString openedFile ;
QString webUIUrl ;
QString selectedNodeId ;
QString statsPanelTooltip ;
qint32 userRole ;
qint32 updateInterval ;
qint32 timerId ;
Settings* settings ;
Struct* snavStruct ;
Stats* statsPanel ;
MsgPanel* filteredMsgPanel ;
QSplitter* mainSplitter ;
QSplitter* rightSplitter ;
QTabWidget * topRightPanel ;
QTabWidget* bottomRightPanel ;
WebKit* webBrowser ;
GraphView* graphView ;
SvNavigatorTree* navigationTree ;
PreferencesDialog* monPrefWindow ;
PreferencesDialog* changePasswdWindow ;
MsgPanel* msgPanel ;
QMenuBar* menuBar;
QToolBar* toolBar ;
QMenu* nodeContextMenu;
QSize msgPanelSize ;
MenuListT menuList;
SubMenuListT subMenuList;
SubMenuListT contextMenuList;


void updateNavTreeItemStatus(const NodeListT::iterator &, const QString & );
void setNodeToolTip(QString & _tool_tip, const NodeListT::iterator & _node) ;
void updateAlarmMsg(NodeListT::iterator &);
void loadMenus(void);
void unloadMenus(void);
void addEvents(void);

};

#endif /* SNAV_H_ */
