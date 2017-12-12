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

#include <QString>
#include "Base.hpp"
#include "Parser.hpp"
#include "Preferences.hpp"
#include "ZbxHelper.hpp"
#include "ZnsHelper.hpp"

class QScriptValueIterator;
class QSystemTrayIcon;

class DashboardBase : public QObject
{
  Q_OBJECT

public:
  DashboardBase(const QString& descriptionFile);
  virtual ~DashboardBase();

  static StringMapT propRules();
  static StringMapT calcRules();
  void initSettings(Preferences* preferencePtr);
  qint64 updateCounter(void) const {return m_updateCounter;}
  QString config(void) const {return m_descriptionFile;}
  void setSelectedNode(const QString& nodeid) {m_selectedNode = nodeid;}
  QString selectedNode(void) const {return m_selectedNode;}
  void setTimerId(qint32 id) {m_timerId = id;}
  qint32 timerId(void) const {return m_timerId;}
  qint32 timerInterval(void) const {return m_interval;}
  NodeT rootNode(void);
  bool lastErrorState() const {return m_lastErrorState;}
  QString lastErrorMsg(void) const {return m_lastErrorMsg;}
  void extractStatsData(CheckStatusCountT& statsData, qint32& count);

public Q_SLOTS:
  void runMonitor();
  void runMonitor(SourceT& src);
  void runDataSourceUpdate(const SourceT& srcInfo);
  void resetStatData(void);
  void prepareUpdate(const SourceT& src);
  ngrt4n::AggregatedSeverityT computeNodeSeverity(const QString& _node);
  void checkStandaloneSourceType(SourceT& src);
  void handleSourceSettingsChanged(QList<qint8> ids);
  void handleErrorOccurred(QString msg) {m_lastErrorMsg  = msg;}
  virtual void initialize(Preferences* preferencePtr);
  qint32 userRole(void) const {return m_userRole;}
  bool showOnlyTroubles(void) const {return m_showOnlyTroubles;}
  void setShowOnlyTroubles(bool value) {m_showOnlyTroubles = value;}
  SourceListT sources(void) {return m_sources;}
  int firstSrcIndex(void) {return m_firstSrcIndex;}

Q_SIGNALS:
  void sortEventConsole(void);
  void updateStatusBar(const QString& msg);
  void settingsLoaded(void);
  void updateSourceUrl(void);
  void timerIntervalChanged(qint32 interval);
  void errorOccurred(QString msg);
  void dashboardLinkSelected(void);
  void updateInprogress(void);
  void updateFinished(void);

protected:
  void computeStatusInfo(NodeT& _node, const SourceT& src);
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

protected:
  QString m_descriptionFile;
  CoreDataT m_cdata;
  qint32 m_timerId;
  qint64 m_updateCounter;
  QString m_selectedNode;
  qint32 m_userRole;
  qint32 m_interval;
  QSize m_msgConsoleSize;
  bool m_showOnlyTroubles;
  SourceListT m_sources;
  int m_firstSrcIndex;
  bool m_lastErrorState;
  QString m_lastErrorMsg;
  Preferences* m_preferences;

protected:
  void resetInterval(void);
  void updateCNodesWithCheck(const CheckT & check, const SourceT& src);
  void updateCNodesWithChecks(const ChecksT& checks, const SourceT& src);
  QStringList getAuthInfo(int srcId);
  void computeFirstSrcIndex(void);
  void updateDashboardOnError(const SourceT& src, const QString& msg);
};

#endif /* SVNAVIGATOR_HPP */
