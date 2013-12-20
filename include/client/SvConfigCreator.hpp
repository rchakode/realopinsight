/*
 * SvCreator.hpp
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

#ifndef SNAVSVCREATOR_H_
#define SNAVSVCREATOR_H_

#include "Base.hpp"
#include "Parser.hpp"
#include "ServiceEditor.hpp"
#include "SvNavigatorTree.hpp"
#include "SvConfigCreator.hpp"
#include "Preferences.hpp"


class SvCreator: public QMainWindow
{
  Q_OBJECT

public:
  SvCreator(const qint32& _userRole = ngrt4n::OpUserRole);
  virtual ~SvCreator();
  void load( const QString&);
  QSize minimumSizeHint() const {return QSize(796, 640);}
  QSize sizeHint() const {return QSize(796, 640);}

public Q_SLOTS:
  void newView(void);
  void newNode(void);
  void insertFromSelected(const NodeT& node);
  void copySelected(void);
  void pasteFromSelected(void);
  void deleteNode(void);
  void deleteNode(const QString &);
  void open(void);
  void save(void);
  void saveAs(void);
  int treatCloseAction(const bool& = true);
  void fillEditorFromService( QTreeWidgetItem*);
  void handleReturnPressed(void);
  void handleSelectedNodeChanged( void);
  void handleTreeNodeMoved(QString);
  void handleNodeTypeActivated(qint32);
  void handleShowOnlineResources(void);
  void handleShowAbout(void);
  void import(void);

protected:
  virtual void contextMenuEvent( QContextMenuEvent *);
  virtual void closeEvent( QCloseEvent *);

private:
  qint32 m_userRole;
  bool m_hasLeftUpdates;
  QString m_activeConfig;
  QString m_selectedNode;
  Settings* m_settings;
  CoreDataT* m_cdata;
  QSplitter* m_mainSplitter;
  MenuListT m_menus;
  SubMenuListT m_subMenus;
  SvNavigatorTree* m_tree;
  ServiceEditor* m_editor;
  QMenuBar* m_menuBar;
  QToolBar* m_toolBar;
  QMenu* m_nodeContextMenu;
  NodeT* m_clipboardData;

  void loadFile(const QString &);
  void recordData(const QString &);
  void recordNode(QTextStream& stream, const NodeT & node);
  bool updateServiceNode(NodeListT& , const QString &);
  void loadMenu(void);
  void unloadMenu(void);
  void addEvents(void);
  void resize(void);
  NodeT* createNode(const QString& id, const QString& label, const QString& parent);
};

#endif /* SNAVSVCREATOR_H_ */
