/*
 * DashboardBase.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-04-2014                                                  #
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

#include "Base.hpp"
#include "Parser.hpp"
#include "BaseSettings.hpp"
#include "ZbxHelper.hpp"
#include "ZnsHelper.hpp"
#include "dbo/src/DbSession.hpp"
#include <QString>

class QScriptValueIterator;
class QSystemTrayIcon;

class DashboardBase : public QObject
{
  Q_OBJECT

public:
  DashboardBase(DbSession* dbSession);
  virtual ~DashboardBase();

  static StringMapT propRules();
  static StringMapT calcRules();
  void setSelectedNode(const QString& nodeid) {m_selectedNode = nodeid;}
  QString selectedNode(void) const {return m_selectedNode;}
  void setTimerId(qint32 id) {m_timerId = id;}
  qint32 timerId(void) const {return m_timerId;}
  qint32 timerInterval(void) const {return m_interval;}
  NodeT rootNode(void);
  int extractStatsData(CheckStatusCountT& statsData);
  void setDbSession(DbSession* dbSession) {m_dbSession = dbSession;}

  std::pair<int, QString> loadDataSources(void);
  std::pair<int, QString> updateAllNodesStatus(void);

public Q_SLOTS:
  void runGenericViewUpdate(const SourceT& srcInfo);
  void runDynamicViewByGroupUpdate(const SourceT& srcInfo);
  void resetStatData(void);
  ngrt4n::AggregatedSeverityT computeBpNodeStatus(const QString& _node, DbSession* p_dbSession);
  virtual std::pair<int, QString> initialize(BaseSettings* p_settings, const QString& viewFile);
  qint32 userRole(void) const {return m_userRole;}
  SourceListT sources(void) {return m_sources;}
  void setShowOnlyProblemMsgsState(bool state) {m_showOnlyProblemMsgsState = state;}

Q_SIGNALS:
  void sortEventConsole(void);
  void updateMessageChanged(const std::string& msg);
  void dashboardLinkSelected(void);

protected:
  CoreDataT m_cdata;
  bool m_showOnlyProblemMsgsState;

  void updateNodeStatusInfo(NodeT& _node, const SourceT& src);
  int extractSourceIndex(const QString& sid) {return sid.at(6).digitValue();}
  virtual void updateDashboard(const NodeT& _node);
  virtual void buildMap(void) = 0;
  virtual void updateMap(const NodeT& _node, const QString& _tip) = 0;
  virtual void buildTree(void) = 0;
  virtual void updateTree(const NodeT& _node, const QString& _tip) = 0;
  virtual void updateMsgConsole(const NodeT& _node) = 0;
  virtual void finalizeUpdate(const SourceT& src);
  virtual void updateChart(void) = 0;
  virtual void updateEventFeeds(const NodeT& node) = 0;

private:
  DbSession* m_dbSession;
  qint32 m_timerId;
  QString m_selectedNode;
  qint32 m_userRole;
  qint32 m_interval;
  QSize m_msgConsoleSize;
  SourceListT m_sources;
  void signalUpdateProcessing(const SourceT& src);
  void updateCNodesWithCheck(const CheckT & check, const SourceT& src);
  void updateCNodesWithChecks(const ChecksT& checks, const SourceT& src);
  void computeFirstSrcIndex(void);
  void updateDashboardOnError(const SourceT& src, const QString& msg);
};

#endif /* SVNAVIGATOR_HPP */
