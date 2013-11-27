/*
 * GuiDashboard.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 19-09-2013                                                  #
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

#ifndef GUIDASHBOOARD_HPP
#define GUIDASHBOOARD_HPP

#include <QString>
#include "Base.hpp"
#include "Chart.hpp"
#include "Parser.hpp"
#include "WebKit.hpp"
#include "MsgConsole.hpp"
#include "GraphView.hpp"
#include "SvNavigatorTree.hpp"
#include "Preferences.hpp"
#include "ZmqSocket.hpp"
#include "ZbxHelper.hpp"
#include "ZnsHelper.hpp"
#include "DashboardBase.hpp"

class QScriptValueIterator;
class QSystemTrayIcon;

class GuiDashboard : public DashboardBase
{
  Q_OBJECT

public:
  GuiDashboard(const qint32& _userRole, const QString& _config);
  virtual ~GuiDashboard();

  QWidget* getWidget(void) const {return m_widget;}
  void resizeDashboard(qint32 width, qint32 height);
  void showTrayIcon(void) const {m_trayIcon->show();}
  void setTrayIconTooltip(const QString& msg) {m_trayIcon->setToolTip(msg);}
  void scalPaneContentsToViewPort(void) const;
  QList<QTreeWidgetItem*> getTreeSelectedItem() const {return m_tree->selectedItems();}
  QGraphicsItem* getMapNodeAt(const QPoint& pos) const {return m_map->nodeAtGlobalPos(pos);}
  QComboBox* getSourceSelectionBox(void) const {return m_bxSourceSelection;}
  MsgConsole* getFilteredMsgConsole(void) const {return m_filteredMsgConsole;}
  bool hideChart(void) {return m_map->hideChart();}
  void setMsgPaneToolBar(const QList<QAction*>& menuAtions);
  GraphView* getMap(void) const {return m_map;}
  WebKit* getBrowser(void) const {return m_browser;}
  static StringMapT propRules();
  static StringMapT calcRules();

public slots:
  void handleShowAbout(void);
  void handleShowOnlineResources(void);
  void handleChangeMonitoringSettingsAction(void);
  void handleChangePasswordAction(void);
  void expandNode(const QString& _nodeId, const bool& _expand, const qint32& _level);
  void centerGraphOnNode(void) { centerGraphOnNode(m_selectedNode); }
  void centerGraphOnNode(const QString& _nodeId);
  void centerGraphOnNode(QTreeWidgetItem* _item);
  void filterNodeRelatedMsg(void);
  void filterNodeRelatedMsg(const QString& _nodeId);
  void handleTabChanged(int index) {emit centralTabChanged(index);}
  void toggleTroubleView(bool _toggled);
  void toggleIncreaseMsgFont(bool _toggled);
  void handleSourceBxItemChanged(int index);
  void handleUpdateSourceUrl(void);
  void handleSettingsLoaded(void);
  //  void updateDashboard(const NodeT & _node);

signals:
  void hasToBeUpdate(const QString& nodeid);
  void sortEventConsole(void);
  void centralTabChanged(int);

protected:
  virtual void updateTrayInfo(const NodeT& _node);
  QTabWidget* builtMsgPane(void);
  void changeBrowserUrl(const QString& sid, const QString& url, const QString& icon);
  virtual void buildMap(void);
  virtual void updateMap(const NodeT& _node, const QString& _tip);
  virtual void buildTree(void);
  virtual void updateTree(const NodeT& _node, const QString& _tip);
  virtual void updateMsgConsole(const NodeT& _node);
  virtual void updateChart(void);

private:
  enum {
    ConsoleTab=0,
    BrowserTab=1
  };
  std::shared_ptr<Chart> m_chart;
  MsgConsole* m_filteredMsgConsole;
  QSplitter* m_widget;
  QSplitter* m_rightSplitter;
  QTabWidget* m_viewPanel;
  WebKit* m_browser;
  GraphView* m_map;
  SvNavigatorTree* m_tree;
  MsgConsole* m_msgConsole;
  QSystemTrayIcon* m_trayIcon;
  QComboBox* m_bxSourceSelection;
  QTabWidget* m_msgPane;
  void addEvents(void);
  QTabWidget* newMsgConsole(void);
  void setBrowserSourceSelectionBx(void);
};
#endif /* GUIDASHBOOARD_HPP */
