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
#include "utilsCore.hpp"
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
  const QString SERVICE_OFFLINE_MSG(QObject::tr("Failed to connect to %1 (%2)"));
  const QString JSON_ERROR_MSG("{\"return_code\": \"-1\", \"message\": \""%SERVICE_OFFLINE_MSG%"\"}");
  const qint32 CHART_WIDTH = 200;
  const qint32 CHART_HEIGHT = 200;
  }

#ifndef REALOPINSIGHT_DISABLE_BROWSER
void GuiDashboard::handleSourceBxItemChanged(int index)
{
  int idx = extractSourceIndex(m_sourceSelectionBox->itemData(index).toString());
  SourceListT::Iterator src = m_sources.find(idx);
  if (src != m_sources.end()) {
    changeBrowserUrl(src->id, src->mon_url, src->icon);
  }
}
void GuiDashboard::changeBrowserUrl(const QString& sid, const QString& url, const QString& icon)
{
  m_browser->setUrl(url);
  m_viewPanel->setTabText(BrowserTab, tr("Web Browser (%1)").arg(sid));
  m_viewPanel->setTabIcon(BrowserTab, QIcon(icon));
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
#endif


StringMapT GuiDashboard::propRules() {
  StringMapT map;
  map.insert(PropRules(PropRules::Unchanged).toString(), PropRules(PropRules::Unchanged).data());
  map.insert(PropRules(PropRules::Decreased).toString(), PropRules(PropRules::Decreased).data());
  map.insert(PropRules(PropRules::Increased).toString(), PropRules(PropRules::Increased).data());
  return map;
}

StringMapT GuiDashboard::calcRules() {
  StringMapT map;
  map.insert(CalcRules(CalcRules::Worst).toString(), CalcRules(CalcRules::Worst).data());
  map.insert(CalcRules(CalcRules::Average).toString(),CalcRules(CalcRules::Average).data());
  return map;
}

GuiDashboard::GuiDashboard(const qint32& _userRole, const QString& _config)
  : DashboardBase(_config),
    m_changePasswdWindow (new GuiPreferences(_userRole, Preferences::ChangePassword)),
    m_widget (new QSplitter()),
    m_chart(new PieChart(QRectF(2, 2, CHART_WIDTH, CHART_HEIGHT), m_widget.get())),
    m_lelfSplitter (new QSplitter()),
    m_rightSplitter (new QSplitter()),
    m_viewPanel (new QTabWidget()),
    m_map (new GraphView(&m_cdata)),
    m_tree (new SvNavigatorTree(&m_cdata)),
    m_msgConsole(new MsgConsole()),
    m_trayIcon(new QSystemTrayIcon(QIcon(":images/built-in/icon.png"))),
    m_sourceSelectionBox(new QComboBox()),
    m_msgPane(new QTabWidget())
{
  m_viewPanel->addTab(m_map.get(), tr("Map"));
  m_viewPanel->setTabIcon(ConsoleTab, QIcon(":images/hierarchy.png"));

#ifndef REALOPINSIGHT_DISABLE_BROWSER
  m_browser.reset(new WebKit());
  m_viewPanel->addTab(m_browser.get(), tr("Web Browser"));
  m_viewPanel->setTabIcon(BrowserTab, QIcon(":images/web.png"));
#endif

  m_widget->addWidget(m_lelfSplitter.get());
  m_widget->addWidget(m_rightSplitter.get());

  m_lelfSplitter->addWidget(m_tree.get());
  m_lelfSplitter->addWidget(m_chart.get());
  m_lelfSplitter->setOrientation(Qt::Vertical);

  m_rightSplitter->addWidget(m_viewPanel.get());
  m_rightSplitter->addWidget(builtMsgPane());
  m_rightSplitter->setOrientation(Qt::Vertical);
  m_chart->setCoreData(&m_cdata);
  addEvents();
}

GuiDashboard::~GuiDashboard()
{
}


void GuiDashboard::addEvents(void)
{
  connect(m_viewPanel.get(), SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged(int)));
  connect(m_map.get(), SIGNAL(expandNode(QString, bool, qint32)), this, SLOT(expandNode(const QString&, const bool &, const qint32 &)));
  connect(m_tree.get(), SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(centerGraphOnNode(QTreeWidgetItem *)));
  connect(this, SIGNAL(settingsLoaded(void)), this, SLOT(handleSettingsLoaded(void)));

#ifndef REALOPINSIGHT_DISABLE_BROWSER
  connect(m_sourceSelectionBox.get(), SIGNAL(activated(int)), this, SLOT(handleSourceBxItemChanged(int)));
  connect(this, SIGNAL(updateSourceUrl(void)), this, SLOT(handleUpdateSourceUrl(void)));
#endif
}


void GuiDashboard::handleChangePasswordAction(void)
{
  m_changePasswdWindow->exec();
}

void GuiDashboard::handleShowOnlineResources(void)
{
  QDesktopServices appLauncher;
  appLauncher.openUrl(DOCS_URL);
}

void GuiDashboard::handleShowAbout(void)
{
  GuiPreferences about(m_userRole, Preferences::ShowAbout);
  about.exec();
}


void GuiDashboard::toggleTroubleView(bool _toggled)
{
  m_msgConsole->setEnabled(false);
  DashboardBase::setShowOnlyTroubles(_toggled);
  if (DashboardBase::showOnlyTroubles()) {
    m_msgConsole->clearNormalMsg();
  } else {
    for (auto it = m_cdata.cnodes.begin(), end = m_cdata.cnodes.end();
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
  m_chart->repaint();
}

void GuiDashboard::updateEventFeeds(const NodeT& node)
{
  if (node.id == ngrt4n::ROOT_ID) {
    updateTrayInfo(node);
  }
}

void GuiDashboard::buildTree(void)
{
  m_tree->build();
  m_tree->expandAll();
}

void GuiDashboard::updateTree(const NodeT& _node, const QString& _tip)
{
  m_tree->updateNodeItem(_node, _tip);
}

void GuiDashboard::updateMsgConsole(const NodeT& _node)
{
  if (! DashboardBase::showOnlyTroubles()
      || (DashboardBase::showOnlyTroubles() && _node.sev != ngrt4n::Normal))
  {
    m_msgConsole->updateNodeMsg(_node);
  }
}

void GuiDashboard::expandNode(const QString& _nodeId, const bool& _expand, const qint32& _level)
{
  auto node = m_cdata.bpnodes.find(_nodeId);
  if (node == m_cdata.bpnodes.end()) {
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
  if (!_nodeId.isEmpty()) m_selectedNode = _nodeId;
  m_map->centerOnNode(m_selectedNode);
}

void GuiDashboard::filterNodeRelatedMsg(void)
{
  m_filteredMsgConsole.reset(new MsgConsole());
  NodeListT::iterator node;
  if (ngrt4n::findNode(&m_cdata, m_selectedNode, node)) {
    filterNodeRelatedMsg(m_selectedNode);
    QSize size(750, 400);
    m_filteredMsgConsole->resize(size.width(), size.height());
    m_filteredMsgConsole->setConsoleSize(size);
    m_filteredMsgConsole->updateEntriesSize(false);
    m_filteredMsgConsole->sortByColumn(1, Qt::AscendingOrder);
    m_filteredMsgConsole->setWindowTitle(tr("Messages related to '%2' - %1").arg(APP_NAME, node->name));
  }
  m_filteredMsgConsole->show();
}

void GuiDashboard::filterNodeRelatedMsg(const QString& _nodeId)
{
  NodeListT::iterator node;
  if (ngrt4n::findNode(&m_cdata, _nodeId, node)
      && ! node->child_nodes.isEmpty()) {
    if (node->type == NodeType::ITService) {
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
  const qreal LEFT_FRAME_HEIGHT_RATE = 0.30;
  const qreal RIGHT_FRAME_HEIGHT_RATE = (1 - LEFT_FRAME_HEIGHT_RATE);
  QSize msgConsoleSize = QSize(width * RIGHT_FRAME_HEIGHT_RATE,
                               height * (1.0 - GRAPH_HEIGHT_RATE));;

  QList<qint32> horizontalFramesSize;
  QList<qint32> leftFramesSize;
  QList<qint32> rightFramesSize;

  horizontalFramesSize.push_back(width * LEFT_FRAME_HEIGHT_RATE);
  horizontalFramesSize.push_back(msgConsoleSize.width());
  m_widget->setSizes(horizontalFramesSize);

  leftFramesSize.push_back(width - m_chart->size().height());
  leftFramesSize.push_back( m_chart->size().height());
  m_lelfSplitter->setSizes(leftFramesSize);

  rightFramesSize.push_back(height * GRAPH_HEIGHT_RATE);
  rightFramesSize.push_back(msgConsoleSize.height());
  m_rightSplitter->setSizes(rightFramesSize);

  m_widget->resize(width, height * 0.85);
  m_msgConsole->setConsoleSize(msgConsoleSize);
}


void GuiDashboard::scalPaneContentsToViewPort(void) const
{
  m_map->scaleToFitViewPort();
  m_msgConsole->updateEntriesSize(false);
}

void GuiDashboard::updateTrayInfo(const NodeT& _node)
{
  QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;
  if (_node.sev == ngrt4n::Critical ||
      _node.sev == ngrt4n::Unknown) {
    icon = QSystemTrayIcon::Critical;
  } else if (_node.sev == ngrt4n::Minor ||
             _node.sev == ngrt4n::Major) {
    icon = QSystemTrayIcon::Warning;
  }
  qint32 pbCount = m_cdata.cnodes.size() - m_cdata.check_status_count[ngrt4n::Normal];
  QString title = APP_NAME%" - "%_node.name;
  QString msg = tr(" - %1 Problem%2\n"
                   " - Level of Impact: %3").arg(QString::number(pbCount), pbCount>1?tr("s"):"",
                                                 Severity(_node.sev).toString().toUpper());

  m_trayIcon->showMessage(title, msg, icon);
  m_trayIcon->setToolTip(title%"\n"%msg);
}

QTabWidget* GuiDashboard::builtMsgPane(void)
{
  QHBoxLayout* lyt(new QHBoxLayout());
  QGroupBox* wdgsGrp(new QGroupBox());
  lyt->addWidget(m_msgConsole.get(), Qt::AlignLeft);
  lyt->setMargin(0);
  lyt->setContentsMargins(QMargins(0, 0, 0, 0));
  wdgsGrp->setLayout(lyt);
  m_msgPane->addTab(wdgsGrp, tr("Message Console"));
  return m_msgPane.get();
}

void GuiDashboard::handleSettingsLoaded(void)
{
  m_sourceSelectionBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  for (SourceListT::iterator it=m_sources.begin(),
       end = m_sources.end(); it != end; ++it)
  {
    if (m_cdata.sources.contains(it->id))
    {
      switch(it->mon_type) {
      case MonitorT::Nagios:
        it->icon = ":images/nagios-logo-n.png";
        break;
      case MonitorT::Zabbix:
        it->icon = ":images/zabbix-logo-z.png";
        break;
      case MonitorT::Zenoss:
        it->icon = ":images/zenoss-logo-o.png";
        break;
      case MonitorT::Pandora:
        it->icon = ":images/pandora-logo-p.png";
        break;
      default:
        it->icon = "";
        break;
      }
      m_sourceSelectionBox->addItem(QIcon(it->icon), it->id, QVariant(it->id));
    }
  }

#ifndef REALOPINSIGHT_DISABLE_BROWSER
  handleUpdateSourceUrl();
#endif
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
