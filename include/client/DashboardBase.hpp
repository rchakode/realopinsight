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

class DashboardBase : public QWidget
{
  Q_OBJECT

public:
  DashboardBase(const qint32& _userRole, const QString& _config);
  virtual ~DashboardBase();

  static StringMapT propRules();
  static StringMapT calcRules();
  void initSettings(void);
  virtual void load(const QString& _file) = 0;
  virtual void resizeDashboard(void) = 0;

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
  void processZbxReply(QNetworkReply* reply, SourceT& src);
  void processZnsReply(QNetworkReply* reply, SourceT& src);
  void processRpcError(QNetworkReply::NetworkError code, const SourceT& src);
  bool allocSourceHandler(SourceT& src);
  void handleSourceSettingsChanged(QList<qint8> ids);
  virtual void handleShowAbout(void) = 0;
  virtual void handleShowOnlineResources(void) = 0;
  virtual void handleChangeMonitoringSettingsAction(void) = 0;
  virtual void handleChangePasswordAction(void) = 0;
  virtual void expandNode(const QString& _nodeId, const bool& _expand, const qint32& _level) = 0;
  virtual void centerGraphOnNode(const QString& _nodeId) = 0;
  virtual void filterNodeRelatedMsg(void) = 0;
  virtual void filterNodeRelatedMsg(const QString& _nodeId) = 0;
  virtual void handleTabChanged(int index) = 0;
  virtual void handleHideChart(void) = 0;
  virtual void toggleFullScreen(bool _toggled) = 0;
  virtual void toggleTroubleView(bool _toggled) = 0;
  virtual void toggleIncreaseMsgFont(bool _toggled) = 0;

signals:
  void hasToBeUpdate(QString);
  void sortEventConsole(void);

protected:
  qint64 updateCounter;
  CoreDataT* m_cdata;
  QString m_config;
  QString m_selectedNode;
  qint32 m_userRole;
  qint32 m_interval;
  qint32 m_timer;
  Settings* m_settings;
  Preferences* m_preferences;
  Preferences* m_changePasswdWindow;
  QSize m_msgConsoleSize;
  bool m_showOnlyTroubles;
  SourceListT m_sources;
  NodeListT::Iterator m_root;
  int m_firstSrcIndex;

  void updateCNodes(const CheckT & check, const SourceT& src);
  void computeStatusInfo(NodeListT::iterator& _node, const SourceT& src);
  void computeStatusInfo(NodeT& _node, const SourceT& src);
  QStringList getAuthInfo(int srcId);
  QStringList getAuthInfo(const QString& authString);
  void openRpcSessions(void);
  void openRpcSession(int srcId);
  void openRpcSession(SourceT& src);
  void requestZbxZnsData(SourceT& src);
  void resetInterval(void);
  void computeFirstSrcIndex(void);
  int extractSourceIndex(const QString& sid) {return sid.at(6).digitValue();}
  void updateDashboardOnError(const SourceT& src, const QString& msg);
  virtual void updateNavTreeItemStatus(const NodeListT::iterator& _node, const QString& _tip);
  virtual void updateNavTreeItemStatus(const NodeT& _node, const QString& _tip) = 0;
  virtual void updateDashboard(const NodeT & _node) = 0;
  virtual void finalizeUpdate(const SourceT& src) = 0;
  virtual void updateDashboard(NodeListT::iterator& _node);
};

#endif /* SVNAVIGATOR_HPP */
