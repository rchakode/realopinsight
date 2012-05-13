/*
 * SvCreator.hpp
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

#ifndef SNAVSVCREATOR_H_
#define SNAVSVCREATOR_H_

#include "Base.hpp"
#include "Parser.hpp"
#include "ServiceEditor.hpp"
#include "SvNavigatorTree.hpp"
#include "SvConfigCreator.hpp"
#include "PreferencesDialog.hpp"



class SvCreator: public QMainWindow
{
	Q_OBJECT

public:
	SvCreator(const qint32 & _user_role = OP_USER_ROLE);
	virtual ~SvCreator( );

	void load( const QString& );
	QSize minimumSizeHint() const { 	return QSize(796, 640) ; }
	QSize sizeHint() const { return QSize(796, 640) ; }


public slots:
	void newBusinessView(void);
	void newNode(void);
	void deleteNode(void);
	void deleteNode(const QString & );
	void open(void);
	void save(void);
	void saveAs(void);
	int close( const bool & = true );
	void fillEditorFromService( QTreeWidgetItem*);
	void handleReturnPressed(void) ;
	void handleSelectedNodeChanged( void ) ;
	void handleTreeNodeMoved(QString) ;
	void handleNodeTypeActivated(qint32) ;
	void handleShowOnlineResources(void) ;
	void handleShowAbout(void) ;


protected:
	void contextMenuEvent( QContextMenuEvent * );
	void closeEvent( QCloseEvent * ) ;


private:

qint32 userRole ;
qint32 hasToBeSaved ;
QString openedFile ;
QString selectedNode ;

//QString statusFile ;
QString selectedNodeId ;

Settings* settings ;
Struct* snavStruct ;
QSplitter* mainSplitter ;
MenuListT menuList ;
SubMenuListT subMenuList ;

SvNavigatorTree* navigationTree;
ServiceEditor* editor;
QMenuBar* menuBar ;
QToolBar* toolBar ;
QMenu* nodeContextMenu ;

void loadFile(const QString & );

void saveInFile(const QString & ) ;

bool updateServiceNode(NodeListT & , const QString & ) ;

void loadMenu(void) ;

void unloadMenu(void) ;

void addEvents(void);

void resize(const qint32 & , const qint32 &, const qint32 &, const qint32 &);

};

#endif /* SNAVSVCREATOR_H_ */
