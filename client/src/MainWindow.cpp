/*
 * MainWindow.cpp
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

#include "MainWindow.hpp"
#include "utilsCore.hpp"


MainWindow::MainWindow(const qint32& _userRole,
                       const QString& _config)
  : QMainWindow(0),
    m_preferences(new GuiPreferences(_userRole, Preferences::ChangeMonitoringSettings)),
    m_dashboard(new GuiDashboard(_userRole, _config)),
    m_contextMenu(new QMenu(this))
{
  QMainWindow::setWindowTitle(tr("%1 - Operations Console").arg(APP_NAME));
  loadMenus();
  setCentralWidget(m_dashboard->getWidget());
  handleTabChanged(0);
  addEvents();
  m_dashboard->initialize(m_preferences);
  m_dashboard->setTimerId( startTimer(1000 * m_preferences->updateInterval()) );
}


MainWindow::~MainWindow()
{
  unloadMenus();
  delete m_preferences;
  delete m_dashboard;
  delete m_contextMenu;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  QMainWindow::closeEvent(event);
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
  //FIXME: Fix when a node was selected on the tree
  // and the cursor is into the map
  QPoint pos = event->globalPos();
  QList<QTreeWidgetItem*> treeNodes = m_dashboard->getTreeSelectedItem();
  QGraphicsItem* graphNode = m_dashboard->getMapNodeAt(pos);
  if (treeNodes.length() || graphNode) {
    if (graphNode) {
      QString itemId = graphNode->data(0).toString();
      m_dashboard->setSelectedNode(itemId.left(itemId.indexOf(":")));
    } else {
      m_dashboard->setSelectedNode(treeNodes[0]->data(0, QTreeWidgetItem::UserType).toString());
    }
    m_contextMenu->exec(pos);
  }
}

void MainWindow::timerEvent(QTimerEvent*)
{
  handleRefresh();
}

void MainWindow::showEvent(QShowEvent*)
{
  if (m_dashboard->updateCounter()==0) {
    std::unique_ptr<QSplashScreen> info(ngrt4n::infoScreen());
    info->showMessage(tr("Please wait for initialization, it may take a while..."),
                      Qt::AlignCenter|Qt::AlignCenter);

    m_dashboard->initSettings(m_preferences);

    info->finish(0);
    m_dashboard->showTrayIcon();
    m_dashboard->setTrayIconTooltip(APP_NAME);
    QMainWindow::resize(1024, 768);
    m_dashboard->resizeDashboard(size().width(), size().height());
  }
}

void MainWindow::loadMenus(void)
{
  QMenuBar* menuBar = QMainWindow::menuBar();
  m_menus["FILE"] = menuBar->addMenu(tr("&File")),
      m_subMenus["Refresh"] = m_menus["FILE"]->addAction(QIcon(":images/built-in/refresh.png"),tr("&Refresh Screen")),
      m_subMenus["Capture"] = m_menus["FILE"]->addAction(QIcon(":images/built-in/camera.png"),tr("&Save Map as Image"));
  m_menus["FILE"]->addSeparator(),
      m_subMenus["Quit"] = m_menus["FILE"]->addAction(tr("&Quit")),
      m_subMenus["Capture"]->setShortcut(QKeySequence::Save),
      m_subMenus["Refresh"]->setShortcut(QKeySequence::Refresh),
      m_subMenus["Quit"]->setShortcut(QKeySequence::Quit);
  m_menus["CONSOLE"] = menuBar->addMenu(tr("&Console")),
      m_subMenus["ZoomIn"] = m_menus["CONSOLE"]->addAction(QIcon(":images/built-in/zoomin.png"),tr("Map Zoom &In")),
      m_subMenus["ZoomOut"] = m_menus["CONSOLE"]->addAction(QIcon(":images/built-in/zoomout.png"),tr("Map Zoom &Out")),
      m_subMenus["HideChart"] = m_menus["CONSOLE"]->addAction(tr("Hide &Chart")),
      m_subMenus["ZoomIn"]->setShortcut(QKeySequence::ZoomIn),
      m_subMenus["ZoomOut"]->setShortcut(QKeySequence::ZoomOut);
  m_menus["CONSOLE"]->addSeparator(),
      m_subMenus["FullScreen"] = m_menus["CONSOLE"]->addAction(QIcon(":images/built-in/fullscreen.png"),tr("&Full Screen")),
      m_subMenus["FullScreen"]->setCheckable(true);
  m_menus["CONSOLE"]->addSeparator(),
      m_subMenus["TroubleView"] = m_menus["CONSOLE"]->addAction(QIcon(":images/built-in/alert-circle.png"),tr("&Show only trouble messages")),
      m_subMenus["TroubleView"]->setCheckable(true),
      m_subMenus["IncreaseMsgFont"] = m_menus["CONSOLE"]->addAction(QIcon(":images/built-in/incr-font-size.png"),tr("&Increase message &font")),
      m_subMenus["IncreaseMsgFont"]->setCheckable(true);
  m_menus["PREFERENCES"] = menuBar->addMenu(tr("&Preferences")),
      m_subMenus["ChangePassword"] = m_menus["PREFERENCES"]->addAction(tr("Change &Password")),
      m_subMenus["ChangeMonitoringSettings"] = m_menus["PREFERENCES"]->addAction(QIcon(":images/built-in/system-preferences.png"),tr("&Monitoring Settings")),
      m_subMenus["ChangeMonitoringSettings"]->setShortcut(QKeySequence::Preferences);
  m_menus["BROWSER"] = menuBar->addMenu(tr("&Browser")),
      m_subMenus["BrowserBack"] = m_menus["BROWSER"]->addAction(QIcon(":images/built-in/browser-back.png"),tr("Bac&k")),
      m_subMenus["BrowserForward"] = m_menus["BROWSER"]->addAction(QIcon(":images/built-in/browser-forward.png"),tr("For&ward"));
  m_subMenus["BrowserStop"] = m_menus["BROWSER"]->addAction(QIcon(":images/built-in/browser-stop.png"),tr("Sto&p"));
  m_menus["HELP"] = menuBar->addMenu(tr("&Help")),
      m_subMenus["ShowOnlineResources"] = m_menus["HELP"]->addAction(tr("Online &Resources")),
      m_menus["HELP"]->addSeparator(),
      m_subMenus["ShowAbout"] = m_menus["HELP"]->addAction(tr("&About %1").arg(APP_NAME)),
      m_subMenus["ShowOnlineResources"]->setShortcut(QKeySequence::HelpContents);
  m_contextMenuList["FilterNodeRelatedMessages"] = m_contextMenu->addAction(tr("&Filter related messages")),
      m_contextMenuList["CenterOnNode"] = m_contextMenu->addAction(tr("Center Graph &On")),
      m_contextMenuList["Cancel"] = m_contextMenu->addAction(tr("&Cancel"));
  QToolBar* toolBar = addToolBar(APP_NAME);
  toolBar->setIconSize(QSize(16,16)),
      toolBar->addAction(m_subMenus["Refresh"]),
      toolBar->addAction(m_subMenus["ZoomIn"]),
      toolBar->addAction(m_subMenus["ZoomOut"]),
      toolBar->addAction(m_subMenus["Capture"]),
      toolBar->addSeparator(),
      toolBar->addAction(m_subMenus["BrowserBack"]),
      toolBar->addAction(m_subMenus["BrowserForward"]),
      toolBar->addAction(m_subMenus["BrowserStop"]);
  toolBar->addSeparator(),
      toolBar->addAction(m_subMenus["FullScreen"]);
  m_subMenus["SourceSelectionBx"] = toolBar->addWidget(m_dashboard->getSourceSelectionBox());
}

void MainWindow::unloadMenus(void)
{
  m_subMenus.clear();
  m_menus.clear();
  delete m_contextMenu;
}

void MainWindow::handleUpdateStatusBar(const QString& msg)
{
  statusBar()->showMessage(msg);
}

void MainWindow::toggleFullScreen(bool _toggled)
{
  _toggled? showFullScreen() : showNormal();
}

void MainWindow::render(void)
{
  show();
  if (! m_dashboard->lastErrorState()) {
    m_dashboard->scalPaneContentsToViewPort();
    QMainWindow::setWindowTitle(tr("%1 - %2 - Operations Console")
                                .arg(m_dashboard->rootNode().name, APP_NAME));
    handleRefresh();
  } else {
    ngrt4n::alert(m_dashboard->lastErrorMsg());
  }
}

void MainWindow::handleTabChanged(int _index)
{
  switch(_index) {
  case 0:
    m_subMenus["Refresh"]->setEnabled(true);
    m_subMenus["Capture"]->setEnabled(true);
    m_subMenus["ZoomIn"]->setEnabled(true);
    m_subMenus["ZoomOut"]->setEnabled(true);
    m_menus["BROWSER"]->setEnabled(false);
    m_subMenus["BrowserBack"]->setEnabled(false);
    m_subMenus["BrowserForward"]->setEnabled(false);
    m_subMenus["BrowserStop"]->setEnabled(false);
    m_subMenus["SourceSelectionBx"]->setEnabled(false);
    break;
  case 1:
    m_menus["BROWSER"]->setEnabled(true);
    m_subMenus["BrowserBack"]->setEnabled(true);
    m_subMenus["BrowserForward"]->setEnabled(true);
    m_subMenus["BrowserStop"]->setEnabled(true);
    m_subMenus["SourceSelectionBx"]->setEnabled(true);
    m_subMenus["Refresh"]->setEnabled(false);
    m_subMenus["Capture"]->setEnabled(false);
    m_subMenus["ZoomIn"]->setEnabled(false);
    m_subMenus["ZoomOut"]->setEnabled(false);
    break;
  default:
    break;
  }
}

void MainWindow::handleHideChart(void)
{
  m_dashboard->hideChart()? m_subMenus["HideChart"]->setIcon(QIcon(":images/check.png")):
    m_subMenus["HideChart"]->setIcon(QIcon(""));
}

void MainWindow::handleRefresh(void)
{
  handleUpdateStatusBar(tr("updating..."));
  m_dashboard->runMonitor();
  handleUpdateStatusBar(tr("update completed"));
}

void MainWindow::resetTimer(qint32 interval)
{
  if (m_dashboard->timerId() < 0)
    killTimer(m_dashboard->timerId());
  m_dashboard->setTimerId( startTimer(1000 * interval) );
}


void MainWindow::handleChangeMonitoringSettingsAction(void)
{
  m_preferences->clearUpdatedSources();
  m_preferences->exec();
}


void MainWindow::addEvents(void)
{
  QList<QAction*> actions;
  actions.push_back(m_subMenus["TroubleView"]);
  actions.push_back(m_subMenus["IncreaseMsgFont"]);
  m_dashboard->setMsgPaneToolBar(actions);

  connect(m_preferences, SIGNAL(errorOccurred(QString)), this, SLOT(handleErrorOccurred(QString)));
  connect(m_preferences, SIGNAL(sourcesChanged(QList<qint8>)), m_dashboard, SLOT(handleSourceSettingsChanged(QList<qint8>)));
  connect(m_subMenus["Quit"], SIGNAL(triggered(bool)), qApp, SLOT(quit()));
  connect(m_subMenus["Capture"], SIGNAL(triggered(bool)), m_dashboard->getMap(), SLOT(capture()));
  connect(m_subMenus["ZoomIn"], SIGNAL(triggered(bool)), m_dashboard->getMap(), SLOT(zoomIn()));
  connect(m_subMenus["ZoomOut"], SIGNAL(triggered(bool)), m_dashboard->getMap(), SLOT(zoomOut()));
  connect(m_subMenus["HideChart"], SIGNAL(triggered(bool)), this, SLOT(handleHideChart()));
  connect(m_subMenus["Refresh"], SIGNAL(triggered(bool)), this, SLOT(handleRefresh()));
  connect(m_subMenus["ChangePassword"], SIGNAL(triggered(bool)), m_dashboard, SLOT(handleChangePasswordAction(void)));
  connect(m_subMenus["ChangeMonitoringSettings"], SIGNAL(triggered(bool)), this, SLOT(handleChangeMonitoringSettingsAction(void)));
  connect(m_subMenus["ShowAbout"], SIGNAL(triggered(bool)), m_dashboard, SLOT(handleShowAbout()));
  connect(m_subMenus["ShowOnlineResources"], SIGNAL(triggered(bool)), m_dashboard, SLOT(handleShowOnlineResources()));
  connect(m_subMenus["FullScreen"], SIGNAL(toggled(bool)), this, SLOT(toggleFullScreen(bool)));
  connect(m_subMenus["TroubleView"], SIGNAL(toggled(bool)), m_dashboard, SLOT(toggleTroubleView(bool)));
  connect(m_subMenus["IncreaseMsgFont"], SIGNAL(toggled(bool)), m_dashboard, SLOT(toggleIncreaseMsgFont(bool)));
  connect(m_contextMenuList["FilterNodeRelatedMessages"], SIGNAL(triggered(bool)), m_dashboard, SLOT(filterNodeRelatedMsg()));
  connect(m_contextMenuList["CenterOnNode"], SIGNAL(triggered(bool)), m_dashboard, SLOT(centerGraphOnNode()));
  connect(m_dashboard, SIGNAL(centralTabChanged(int)), this, SLOT(handleTabChanged(int)));
  connect(m_dashboard, SIGNAL(updateStatusBar(const QString&)), this, SLOT(handleUpdateStatusBar(const QString&)));
  connect(m_dashboard, SIGNAL(updateInprogress()), this, SLOT(handleUpdateIntprogress()));
  connect(m_dashboard, SIGNAL(updateFinished()), this, SLOT(handleUpdateFinished()));


#ifndef REALOPINSIGHT_DISABLE_BROWSER
  connect(m_subMenus["BrowserBack"], SIGNAL(triggered(bool)), m_dashboard->getBrowser(), SLOT(back()));
  connect(m_subMenus["BrowserForward"], SIGNAL(triggered(bool)), m_dashboard->getBrowser(), SLOT(forward()));
  connect(m_subMenus["BrowserStop"], SIGNAL(triggered(bool)), m_dashboard->getBrowser(), SLOT(stop()));
#endif
}
