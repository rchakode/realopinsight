/*
 * GuiDashboard.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-03-2014                                                  #
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
#include "GuiDashboard.hpp"
#include "global.hpp"
#include "utilsCore.hpp"
#include "JsHelper.hpp"
#include "LsHelper.hpp"
#include <QScriptValueIterator>
#include <QNetworkCookieJar>
#include <QSystemTrayIcon>
#include <sstream>
#include <QStatusBar>
#include <QObject>
#include <iostream>
#include <algorithm>


namespace {
  const QString DEFAULT_TIP_PATTERN(QObject::tr("Service: %1\nDescription: %2\nSeverity: %3\n   Calc. Rule: %4\n   Prop. Rule: %5"));
  const QString ALARM_SPECIFIC_TIP_PATTERN(QObject::tr("\nTarget Host: %6\nData Point: %7\nRaw Output: %8\nOther Details: %9"));
  const QString SERVICE_OFFLINE_MSG(QObject::tr("Failed to connect to %1 (%2)"));
  const QString JSON_ERROR_MSG("{\"return_code\": \"-1\", \"message\": \""%SERVICE_OFFLINE_MSG%"\"}");
}

StringMapT GuiDashboard::propRules() {
  StringMapT map;
  map.insert(PropRules::label(PropRules::Unchanged), PropRules::toString(PropRules::Unchanged));
  map.insert(PropRules::label(PropRules::Decreased), PropRules::toString(PropRules::Decreased));
  map.insert(PropRules::label(PropRules::Increased), PropRules::toString(PropRules::Increased));
  return map;
}

StringMapT GuiDashboard::calcRules() {
  StringMapT map;
  map.insert(CalcRules::label(CalcRules::HighCriticity),
             CalcRules::toString(CalcRules::HighCriticity));
  map.insert(CalcRules::label(CalcRules::WeightedCriticity),
             CalcRules::toString(CalcRules::WeightedCriticity));
  return map;
}

GuiDashboard::GuiDashboard(const qint32& _userRole, const QString& _config)
  : DashboardBase(_config),
    m_changePasswdWindow (new GuiPreferences(_userRole, Preferences::ChangePassword)),
    m_chart (std::make_shared<Chart>()),
    m_filteredMsgConsole (NULL),
    m_widget (new QSplitter()),
    m_leftSplitter (new QSplitter()),
    m_rightSplitter (new QSplitter()),
    m_viewPanel (new QTabWidget()),
    m_browser (new WebKit()),
    m_map (new GraphView(m_cdata)),
    m_tree (new SvNavigatorTree(m_cdata)),
    m_msgConsole(new MsgConsole()),
    m_trayIcon(new QSystemTrayIcon(QIcon(":images/built-in/icon.png"))),
    m_bxSourceSelection(new QComboBox()),
    m_msgPane(new QTabWidget())
{
  m_viewPanel->addTab(m_map, tr("Map"));
  m_viewPanel->setTabIcon(ConsoleTab, QIcon(":images/hierarchy.png"));
  m_viewPanel->addTab(m_browser, tr("Web Browser"));
  m_viewPanel->setTabIcon(BrowserTab, QIcon(":images/web.png"));

  m_widget->addWidget(m_leftSplitter);
  m_widget->addWidget(m_rightSplitter);

  m_leftSplitter->addWidget(m_tree);
  m_leftSplitter->addWidget(m_chart.get());
  m_leftSplitter->setOrientation(Qt::Vertical);

  m_rightSplitter->addWidget(m_viewPanel);
  m_rightSplitter->addWidget(builtMsgPane());
  m_rightSplitter->setOrientation(Qt::Vertical);
  addEvents();
}

GuiDashboard::~GuiDashboard()
{
  if (m_filteredMsgConsole) delete m_filteredMsgConsole;
  delete m_msgConsole;
  m_chart.reset();
  delete m_tree;
  delete m_browser;
  delete m_map;
  delete m_cdata;
  delete m_viewPanel;
  delete m_rightSplitter;
  delete m_widget;
  delete m_changePasswdWindow;
  delete m_trayIcon;
  delete m_msgPane;
}


void GuiDashboard::handleChangePasswordAction(void)
{
  m_changePasswdWindow->exec();
}

void GuiDashboard::handleShowOnlineResources(void)
{
  QDesktopServices appLauncher;
  appLauncher.openUrl(QUrl("http://RealOpInsight.com/"));
}

void GuiDashboard::handleShowAbout(void)
{
  GuiPreferences about(m_userRole, Preferences::ShowAbout);
  about.exec();
}


void GuiDashboard::toggleTroubleView(bool _toggled)
{
  m_msgConsole->setEnabled(false);
  m_showOnlyTroubles = _toggled;
  if (m_showOnlyTroubles) {
    m_msgConsole->clearNormalMsg();
  } else {
    for (auto it = m_cdata->cnodes.begin(), end = m_cdata->cnodes.end();
         it != end; ++it) m_msgConsole->updateNodeMsg(it);
    m_msgConsole->sortByColumn(1);
  }
  m_msgConsole->setEnabled(true);
}

void GuiDashboard::toggleIncreaseMsgFont(bool _toggled)
{
  m_msgConsole->useLargeFont(_toggled);
}

void GuiDashboard::buildMap(void)
{
  m_map->drawMap();
}

void GuiDashboard::updateMap(const NodeT& _node, const QString& _tip)
{
  m_map->updateNode(_node, _tip);
}

void GuiDashboard::updateChart(void)
{
  Chart *chart = new Chart;
  chart->update(m_cdata->check_status_count, m_cdata->cnodes.size());
  m_chart.reset(chart);
  m_msgConsole->sortByColumn(1, Qt::AscendingOrder);
}

void GuiDashboard::updateEventFeeds(const NodeT& node)
{
  //TODO
}

void GuiDashboard::buildTree(void)
{
  m_tree->build();
}

void GuiDashboard::updateTree(const NodeT& _node, const QString& _tip)
{
  m_tree->updateNodeItem(_node, _tip);
}

void GuiDashboard::updateMsgConsole(const NodeT& _node)
{
  if (!m_showOnlyTroubles
      || (m_showOnlyTroubles && _node.severity != ngrt4n::Normal))
  {
    m_msgConsole->updateNodeMsg(_node);
  }
}

void GuiDashboard::expandNode(const QString& _nodeId, const bool& _expand, const qint32& _level)
{
  auto node = m_cdata->bpnodes.find(_nodeId);
  if (node == m_cdata->bpnodes.end()) {
    return;
  }
  if (!node->child_nodes.isEmpty()) {
    QStringList  childNodes = node->child_nodes.split(ngrt4n::CHILD_SEP.c_str());
    Q_FOREACH (const auto& cid, childNodes) {
      m_map->setNodeVisible(cid, _nodeId, _expand, _level);
    }
  }
}

void GuiDashboard::centerGraphOnNode(const QString& _nodeId)
{
  if (!_nodeId.isEmpty()) DashboardBase::setSelectedNode(_nodeId);
  m_map->centerOnNode(DashboardBase::selectedNode());
}

void GuiDashboard::filterNodeRelatedMsg(void)
{
  if (m_filteredMsgConsole) delete m_filteredMsgConsole;
  m_filteredMsgConsole = new MsgConsole();
  NodeListT::iterator node;
  if (ngrt4n::findNode(m_cdata, m_selectedNode, node)) {
    filterNodeRelatedMsg(m_selectedNode);
    m_filteredMsgConsole->updateEntriesSize(true);
    m_filteredMsgConsole->setWindowTitle(tr("Messages related to '%2' - %1").arg(APP_NAME, node->name));
  }
  qint32 rh = qMax(m_filteredMsgConsole->getRowCount() * m_filteredMsgConsole->rowHeight(0) + 50, 100);
  if (m_filteredMsgConsole->height() > rh) {
    m_filteredMsgConsole->resize(m_msgConsole->getConsoleSize().width(), rh);
  }
  m_filteredMsgConsole->sortByColumn(1, Qt::AscendingOrder);
  m_filteredMsgConsole->show();;
}

void GuiDashboard::filterNodeRelatedMsg(const QString& _nodeId)
{
  NodeListT::iterator node;
  if (ngrt4n::findNode(m_cdata, _nodeId, node)
      && ! node->child_nodes.isEmpty()) {
    if (node->type == NodeType::AlarmNode) {
      m_filteredMsgConsole->updateNodeMsg(node);
    } else {
      QStringList childIds = node->child_nodes.split(ngrt4n::CHILD_SEP.c_str());
      Q_FOREACH (const QString& chkid, childIds) {
        filterNodeRelatedMsg(chkid);
      }
    }
  }
}

void GuiDashboard::centerGraphOnNode(QTreeWidgetItem * _item)
{
  centerGraphOnNode(_item->data(0, QTreeWidgetItem::UserType).toString());
}

void GuiDashboard::resizeDashboard(qint32 width, qint32 height)
{
  const qreal GRAPH_HEIGHT_RATE = 0.50;
  QSize mapRectSize = QSize(width * 0.80, height * (1.0 - GRAPH_HEIGHT_RATE));;

  QList<qint32> framesSize;
  framesSize.push_back(width * 0.20);
  framesSize.push_back(mapRectSize.width());
  m_widget->setSizes(framesSize);

  framesSize[0] = (height * GRAPH_HEIGHT_RATE);
  framesSize[1] = (mapRectSize.height());

  m_leftSplitter->setSizes(framesSize);
  m_rightSplitter->setSizes(framesSize);

  m_widget->resize(width, height * 0.85);
  m_msgConsole->setConsoleSize(mapRectSize);
}


void GuiDashboard::scalPaneContentsToViewPort(void) const
{
  m_map->scaleToFitViewPort();
  m_msgConsole->updateEntriesSize(false);
}

void GuiDashboard::updateTrayInfo(const NodeT& _node)
{
  QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;
  if (_node.severity == ngrt4n::Critical ||
      _node.severity == ngrt4n::Unknown) {
    icon = QSystemTrayIcon::Critical;
  } else if (_node.severity == ngrt4n::Minor ||
             _node.severity == ngrt4n::Major) {
    icon = QSystemTrayIcon::Warning;
  }
  qint32 pbCount = m_cdata->cnodes.size() - m_cdata->check_status_count[ngrt4n::Normal];
  QString title = APP_NAME%" - "%_node.name;
  QString msg = tr(" - %1 Problem%2\n"
                   " - Level of Impact: %3").arg(QString::number(pbCount), pbCount>1?tr("s"):"",
                                                 ngrt4n::severityText(_node.severity).toUpper());

  m_trayIcon->showMessage(title, msg, icon);
  m_trayIcon->setToolTip(title%"\n"%msg);
}

QTabWidget* GuiDashboard::builtMsgPane(void)
{
  QHBoxLayout* lyt(new QHBoxLayout());
  QGroupBox* wdgsGrp(new QGroupBox());
  lyt->addWidget(m_msgConsole, Qt::AlignLeft);
  lyt->setMargin(0);
  lyt->setContentsMargins(QMargins(0, 0, 0, 0));
  wdgsGrp->setLayout(lyt);
  m_msgPane->addTab(wdgsGrp, tr("Message Console"));
  return m_msgPane;
}

void GuiDashboard::handleSettingsLoaded(void)
{
  m_bxSourceSelection->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  for (SourceListT::iterator it=m_sources.begin(),
       end = m_sources.end(); it != end; ++it)
  {
    if (m_cdata->sources.contains(it->id))
    {
      switch(it->mon_type) {
      case ngrt4n::Nagios:
        it->icon = ":images/nagios-logo-n.png";
        break;
      case ngrt4n::Zabbix:
        it->icon = ":images/zabbix-logo-z.png";
        break;
      case ngrt4n::Zenoss:
        it->icon = ":images/zenoss-logo-o.png";
        break;
      default:
        break;
      }
      m_bxSourceSelection->addItem(QIcon(it->icon), it->id, QVariant(it->id));
    }
  }
  handleUpdateSourceUrl();
}


void GuiDashboard::handleSourceBxItemChanged(int index)
{
  int idx = extractSourceIndex(m_bxSourceSelection->itemData(index).toString());
  SourceListT::Iterator src = m_sources.find(idx);
  if (src != m_sources.end()) {
    changeBrowserUrl(src->id, src->mon_url, src->icon);
  }
}

void GuiDashboard::handleUpdateSourceUrl(void)
{
  if (m_firstSrcIndex >=0 ) {
    SourceListT::Iterator first = m_sources.find(m_firstSrcIndex);
    if (first != m_sources.end()) {
      changeBrowserUrl(first->id, first->mon_url, first->icon);
    }
  }
}

void GuiDashboard::changeBrowserUrl(const QString& sid, const QString& url, const QString& icon)
{
  m_browser->setUrl(url);
  m_viewPanel->setTabText(BrowserTab, tr("Web Browser (%1)").arg(sid));
  m_viewPanel->setTabIcon(BrowserTab, QIcon(icon));
}

void GuiDashboard::setMsgPaneToolBar(const QList<QAction*>& menuAtions)
{
  QToolBar* tlbar (new QToolBar());
  tlbar->setOrientation(Qt::Vertical);
  Q_FOREACH (QAction* action, menuAtions) {
    tlbar->addAction(action);
  }
  m_msgPane->widget(0)->layout()->addWidget(tlbar);
}


bool GuiDashboard::hideChart(void)
{
  bool visible = m_chart->isVisible();
  m_chart->setVisible(! visible);
  return visible;
}

void GuiDashboard::addEvents(void)
{
  connect(m_viewPanel, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged(int)));
  connect(m_map, SIGNAL(expandNode(QString, bool, qint32)), this, SLOT(expandNode(const QString&, const bool &, const qint32 &)));
  connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(centerGraphOnNode(QTreeWidgetItem *)));
  connect(m_bxSourceSelection, SIGNAL(activated(int)), this, SLOT(handleSourceBxItemChanged(int)));
  connect(this, SIGNAL(settingsLoaded(void)), this, SLOT(handleSettingsLoaded(void)));
  connect(this, SIGNAL(updateSourceUrl(void)), this, SLOT(handleUpdateSourceUrl(void)));
  //FIXME:  connect(this, SIGNAL(hasToBeUpdate(QString)), this, SLOT(updateBpNode(QString)));
}
