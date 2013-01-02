/*
 * SvNavigator.hpp
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

#ifndef SVNAVIGATOR_HPP
#define SVNAVIGATOR_HPP

#include <QString>
#include "Base.hpp"
#include "Stats.hpp"
#include "Parser.hpp"
#include "WebKit.hpp"
#include "MsgPanel.hpp"
#include "GraphView.hpp"
#include "SvNavigatorTree.hpp"
#include "Preferences.hpp"
#include "Socket.hpp"
#include "ZbxHelper.hpp"
#include "ZnsHelper.hpp"

class QScriptValueIterator;

class SvNavigator : public QMainWindow
{
  Q_OBJECT

public:
  SvNavigator(const qint32& _userRole = Auth::OP_USER_ROLE,
              const QString& _config = "",
              QWidget* = 0);
  virtual ~SvNavigator();
  void load(const QString& _file = "");

  void resize(void);

  static StringMapT propRules();
  static StringMapT calcRules();

  static QString getNodeToolTip(const NodeT& _node);

public slots:
  void startMonitor();
  int runNagiosMonitor(void);
  void resetStatData(void);
  void updateBpNode(QString);
  void expandNode(const QString& _nodeId,
                  const bool& _expand,
                  const qint32& _level);
  void centerGraphOnNode(const QString& _node_id = "");
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
  void processZbxReply(QNetworkReply* reply);
  void processZnsReply(QNetworkReply* reply);
  void processRpcError(QNetworkReply::NetworkError code);

signals:
  void hasToBeUpdate(QString);
  void sortEventConsole(void);

protected:
  void closeEvent(QCloseEvent *);
  void contextMenuEvent(QContextMenuEvent *);
  void timerEvent(QTimerEvent *);


private:
  CoreDataT* mcoreData;
  QString mconfigFile;
  QString mactiveFile;
  QString mmonitorBaseUrl;
  QString mselectedNode;
  QString mstatsInfo;
  qint32 muserRole;
  qint32 mupdateInterval;
  qint32 mtimer;
  Settings* msettings;
  Stats* mstatsPanel;
  MsgPanel* mfilteredMsgPanel;
  QSplitter* mmainSplitter;
  QSplitter* mrightSplitter;
  QTabWidget* mtopRightPanel;
  QTabWidget* mbottomRightPanel;
  WebKit* mbrowser;
  GraphView* mmap;
  SvNavigatorTree* mtree;
  Preferences* mprefWindow;
  Preferences* mchangePasswdWindow;
  MsgPanel* mmsgPanel;
  QMenu* mnodeContextMenu;
  QSize mmsgPanelSize;
  MenuListT mmenuList;
  SubMenuListT msubMenuList;
  SubMenuListT mcontextMenuList;
  QString mserverAddr;
  QString mserverPort;
  QString mserverUrl;
  QString mserverAuthChain;
  Socket* msocket;
  ZbxHelper* mzbxHelper;
  QString mzbxAuthToken;
  qint32 mhostLeft;
  bool mupdateSucceed;
  ZnsHelper* mznsHelper;
  bool misLogged;


  void addEvents(void);
  void loadMenus(void);
  void unloadMenus(void);
  void updateMonitoringSettings();
  void updateNavTreeItemStatus(const NodeListT::iterator& _node, const QString& _tip);
  void updateNavTreeItemStatus(const NodeT& _node, const QString& _tip);
  void setStatusInfo(NodeListT::iterator& _node);
  void setStatusInfo(NodeT& _node);
  void updateDashboard(NodeListT::iterator& _node);
  void updateDashboard(const NodeT & _node);
  void updateCNodes(const MonitorBroker::CheckT & check);
  void updateStats(void);
  void updateStatusBar(const QString& msg);
  void closeZbxSession(void);
  void openRpcSession(void);
  void postRpcDataRequest(void);
  QStringList getAuthInfo(void);
};

#endif /* SVNAVIGATOR_HPP */
