/*
 * MainWebWindow.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 12-11-2013                                                  #
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


#ifndef MAINWEBWINDOW_HPP
#define MAINWEBWINDOW_HPP

#include <Wt/WTimer>
#include <Wt/WApplication>
#include "WebDashboard.hpp"

class WebMainUI : public QObject, public Wt::WApplication
{
  Q_OBJECT
public:
  WebMainUI(const Wt::WEnvironment& env);
  virtual ~WebMainUI();
  void showHome(void);
  QString getConfig (void) const {return m_currentDashboard->config();}
  void enable(void) {m_mainWidget->enable();}
  void disbale(void) {m_mainWidget->disable();}
  void resetTimer(void);
  void handleInternalPath(void);

public Q_SLOTS:
  void resetTimer(qint32 interval);

private:
  enum FileDialogAction {
    OPEN = 1,
    IMPORT = 0
  };

  qint32 m_userRole;
  Settings* m_settings;
  Wt::WTimer* m_timer;
  Wt::WContainerWidget* createMenuBarWidget(void);
  Wt::WContainerWidget* createToolBar(void);
  Wt::WContainerWidget* m_mainWidget;
  Wt::WMenu* m_dashboardMenu;
  Wt::WDialog* m_fileUploadDialog;
  Wt::WFileUpload* m_uploader;
  std::string m_selectFile;
  Wt::WText* m_infoBox;
  WebDashboard* m_currentDashboard;
  std::map<std::string, WebDashboard*> m_dashboards;

  Wt::WPushButton* createTooBarButton(const std::string& icon);
  void handleRefresh(void);
  Wt::WAnchor* createLogoLink(void);
  void openFileUploadDialog(void);
  void selectFileToOpen(void);
  void openFile(const std::string& path);
  void finishFileDialog(int action);
  void createHomePage(void);
  void scaleMap(double factor);
  void addEvents(void);
};

#endif // MAINWEBWINDOW_HPP
