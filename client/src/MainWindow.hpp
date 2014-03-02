/*
 * MainWindow.hpp
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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "Base.hpp"
#include "GuiDashboard.hpp"
#include "utilsClient.hpp"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(const qint32& _userRole, const QString& _config);
  virtual ~MainWindow();

public Q_SLOTS:
  void handleUpdateStatusBar(const QString& msg);
  void toggleFullScreen(bool _toggled);
  void render(void);
  void handleTabChanged(int index);
  void handleHideChart(void);
  void handleRefresh(void);
  void resetTimer(qint32 interval);
  void handleErrorOccurred(QString msg) {ngrt4n::alert(msg);}
  void handleChangeMonitoringSettingsAction(void);

protected:
  virtual void closeEvent(QCloseEvent*);
  virtual void contextMenuEvent(QContextMenuEvent* event);
  virtual void timerEvent(QTimerEvent*);
  virtual void showEvent(QShowEvent*);

private:
  GuiPreferences* m_preferences;
  GuiDashboard* m_dashboard;
  QMenu* m_contextMenu;
  MenuListT m_menus;
  SubMenuListT m_subMenus;
  SubMenuListT m_contextMenuList;
  void loadMenus(void);
  void unloadMenus(void);
  void addEvents(void);
};

#endif /* MAINWINDOW_HPP*/
