/*
 * SvNavigator.hpp
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

#ifndef SVNAVIGATOR_HPP
#define SVNAVIGATOR_HPP

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

class QScriptValueIterator;
class QSystemTrayIcon;

class SvNavigator : public QMainWindow
{
  Q_OBJECT

public:
  SvNavigator(const qint32& _userRole = Auth::OpUserRole,
              const QString& _config = "",
              QWidget* = 0);
  virtual ~SvNavigator();
  void load(const QString& _file);
  void resizeDashboard(void);
  static StringMapT propRules();
  static StringMapT calcRules();
  static QString getNodeToolTip(const NodeT& _node);
  void initSettings(void);

public slots:
  void runMonitor();
  void runMonitor(SourceT& src);
  void runNagiosUpdate(int srcId);
  void runNagiosUpdate(const SourceT& src);
  void runLivestatusUpdate(int srcId);
  void runLivestatusUpdate(const SourceT& src);
  void resetStatData(void);
  void prepareUpdate(const SourceT& src);
  void updateBpNode(const QString& _node);
  void expandNode(const QString& _nodeId, const bool& _expand, const qint32& _level);
  void centerGraphOnNode(const QString& _nodeId = "");
  void filterNodeRelatedMsg(void);
  void filterNodeRelatedMsg(const QString &);
  void acknowledge(void);
  void tabChanged(int);
  void hideChart(void);
  void centerGraphOnNode(QTreeWidgetItem *);
  void handleChangePasswordAction(void);
  void handleChangeMonitoringSettingsAction(void);
  void handleShowOnlineResources(void);
  void handleShowAbout(void);
  void toggleFullScreen(bool _toggled);
  void toggleTroubleView(bool _toggled);
  void toggleIncreaseMsgFont(bool _toggled);
  void processZbxReply(QNetworkReply* reply, SourceT& src);
  void processZnsReply(QNetworkReply* reply, SourceT& src);
  void processRpcError(QNetworkReply::NetworkError code, const SourceT& src);
  bool allocSourceHandler(SourceT& src);
  void handleSourcesChanged(QList<qint8> ids);

signals:
  void hasToBeUpdate(QString);
  void sortEventConsole(void);

protected:
  virtual void closeEvent(QCloseEvent *);
  virtual void contextMenuEvent(QContextMenuEvent *);
  virtual void timerEvent(QTimerEvent *);
  virtual void showEvent(QShowEvent *);


private:
  CoreDataT* m_cdata;
  QString m_configFile;
  QString m_selectedNode;
  qint32 m_userRole;
  qint32 m_interval;
  qint32 m_timer;
  Settings* m_settings;
  std::shared_ptr<Chart> m_chart;
  MsgConsole* m_filteredMsgConsole;
  QSplitter* m_mainSplitter;
  QSplitter* m_rightSplitter;
  QTabWidget* m_viewPanel;
  WebKit* m_browser;
  GraphView* m_map;
  SvNavigatorTree* m_tree;
  Preferences* m_preferences;
  Preferences* m_changePasswdWindow;
  MsgConsole* m_msgConsole;
  QMenu* m_contextMenu;
  QSize m_msgConsoleSize;
  MenuListT m_menus;
  SubMenuListT m_subMenus;
  SubMenuListT m_contextMenuList;
  QSystemTrayIcon* m_trayIcon;
  bool m_showOnlyTroubles;
  SourceListT m_sources;
  NodeListT::Iterator m_root;

  void addEvents(void);
  void loadMenus(void);
  void unloadMenus(void);
  void updateNavTreeItemStatus(const NodeListT::iterator& _node, const QString& _tip);
  void updateNavTreeItemStatus(const NodeT& _node, const QString& _tip);
  void computeStatusInfo(NodeListT::iterator& _node, const SourceT& src);
  void computeStatusInfo(NodeT& _node, const SourceT& src);
  void updateDashboard(NodeListT::iterator& _node);
  void updateDashboard(const NodeT & _node);
  void updateCNodes(const CheckT & check, const SourceT& src);
  void finalizeUpdate(const SourceT& src);
  void updateStatusBar(const QString& msg);
  QStringList getAuthInfo(int srcId);
  QStringList getAuthInfo(const QString& authString);
  void openRpcSessions(void);
  void openRpcSession(int srcId);
  void openRpcSession(SourceT& src);
  void requestZbxZnsData(SourceT& src);
  void updateDashboardOnError(const SourceT& src, const QString& msg);
  void updateTrayInfo(const NodeT& _node);
  QTabWidget* newMsgConsole();
  void resetInterval(void);
};

#endif /* SVNAVIGATOR_HPP */
