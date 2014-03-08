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

#include "WebDashboard.hpp"
#include "DbSession.hpp"
#include "UserMngtUI.hpp"
#include <Wt/WTimer>
#include <Wt/WApplication>
#include <Wt/WTabWidget>
#include <Wt/WContainerWidget>
#include <Wt/WSignal>

class AuthManager;
class ViewAssignmentUI;
class WebPreferences;

class WebMainUI : public QObject, public Wt::WContainerWidget
{
  Q_OBJECT
public:
  WebMainUI(AuthManager* authManager);
  virtual ~WebMainUI();
  void showUserHome(void);
  QString getConfig (void) const {return m_currentDashboard->config();}
  void enable(void) {m_mainWidget->enable();}
  void disbale(void) {m_mainWidget->disable();}
  void resetTimer(void);
  void handleInternalPath(void);
  Wt::WContainerWidget* get(void) {return m_mainWidget;}
  void handleRefresh(void);
  Wt::Signal<void>& terminateSession(void) {return m_terminateSession;}
  virtual void 	refresh () {handleRefresh();}


public Q_SLOTS:
  void resetTimer(qint32 interval);
  void handleLibError(QString msg) {showMessage(msg.toStdString(), "alert alert-success");}

private:
  enum FileDialogAction {
    OPEN = 1,
    IMPORT = 0
  };
  typedef std::map<std::string, WebDashboard*> DashboardListT;

  Wt::WContainerWidget* m_mainWidget;
  Settings* m_settings;
  Wt::WText* m_infoBox;
  Wt::WText* m_notificationBox;
  AuthManager* m_authManager;
  DbSession* m_dbSession;
  WebPreferences* m_preferenceForm;
  Wt::WTimer m_timer;
  Wt::WStackedWidget* m_contents;
  Wt::WStackedWidget* m_mgntContents;
  Wt::WNavigationBar* m_navbar;
  Wt::WTabWidget* m_dashtabs;
  Wt::WDialog* m_fileUploadDialog;
  Wt::WFileUpload* m_uploader;
  std::string m_selectedFile;
  DashboardListT m_dashboards;
  std::string m_confdir;
  UserMngtUI* m_userMgntUI;
  Wt::WDialog* m_accountPanel;
  Wt::WDialog* m_changePasswordPanel;
  ViewAssignmentUI* m_viewAssignmentDialog;
  Wt::WDialog* m_aboutDialog;
  int m_assignedDashboardCount;
  Wt::WVBoxLayout* m_eventFeedLayout;
  Wt::WText* m_adminPanelTitle;
  bool m_showSettingTab;
  WebDashboard* m_currentDashboard;

  /** Signals */
  Wt::Signal<void> m_terminateSession;

  void addEvents(void);
  void createMainUI(void);
  void setupProfileMenus(void);
  void setupMenus(void);
  Wt::WAnchor* createLogoLink(void);
  void openFileUploadDialog(void);
  void selectFileToOpen(void);
  void initOperatorDashboard(void);
  void loadView(const std::string& path, WebDashboard*& dashboard, int& tabIndex);
  Wt::WTemplate* thumbnail(WebDashboard* dashboard);
  void finishFileDialog(int action);
  void scaleMap(double factor);
  Wt::WWidget* createSettingPage(void);
  Wt::WAnchor* createAnchorForHomeLink(const std::string& title,const std::string& desc,const std::string& internalPath);
  void createAccountPanel(void);
  void createPasswordPanel(void);
  void createViewAssignmentDialog(void);
  void createAboutDialog(void);
  Wt::WDialog* createDialog(const std::string& title, Wt::WWidget* content=0);
  Wt::WComboBox* createViewSelector(void);
  void showMessage(const std::string& msg, std::string status);
  void setInternalPath(const std::string& path);
  bool createDirectory(const std::string& path, bool cleanContent);
  void startDashbaordUpdate(void);
  void updateEventFeeds(void);
};

#endif // MAINWEBWINDOW_HPP
