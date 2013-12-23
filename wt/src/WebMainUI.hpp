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

class WebMainUI : public QObject, public Wt::WContainerWidget
{
  Q_OBJECT
public:
  WebMainUI(AuthManager* authManager);
  virtual ~WebMainUI();
  void showUserHome(void);
  void showLoginHome(void);
  QString getConfig (void) const {return m_currentDashboard->config();}
  void enable(void) {m_mainWidget->enable();}
  void disbale(void) {m_mainWidget->disable();}
  void resetTimer(void);
  void handleInternalPath(void);
  Wt::WContainerWidget* get(void) {return m_mainWidget;}

  Wt::Signal<void>& terminateSession(void) {return m_terminateSession;}

public Q_SLOTS:
  void resetTimer(qint32 interval);
  void handleLibError(QString msg) {showMessage(msg.toStdString(), "alert alert-success");}

private:
  enum FileDialogAction {
    OPEN = 1,
    IMPORT = 0
  };

  Settings* m_settings;
  Wt::WTimer* m_timer;
  Wt::WContainerWidget* m_mainWidget;
  Wt::WStackedWidget* m_contents;
  Wt::WNavigationBar* m_navbar;
  Wt::WMenu* m_mgntMenu;
  Wt::WMenu* m_profileMenu;
  Wt::WMenuItem* m_mainProfileMenuItem;
  Wt::WTabWidget* m_dashtabs;
  Wt::WDialog* m_fileUploadDialog;
  Wt::WFileUpload* m_uploader;
  std::string m_selectFile;
  WebDashboard* m_currentDashboard;
  typedef std::map<std::string, WebDashboard*> DashboardListT;
  DashboardListT m_dashboards;
  AuthManager* m_authManager;
  DbSession* m_dbSession;
  std::string m_confdir;
  UserMngtUI* m_userMgntUI;
  Wt::WDialog* m_accountPanel;
  Wt::WDialog* m_changePasswordPanel;
  Wt::WDialog* m_infoMsgBox;
  ViewAssignmentUI* m_viewAssignmentDialog;
  Wt::WDialog* m_aboutDialog;
  Wt::Signal<void> m_terminateSession;

  void addEvents(void);
  void createMainUI(void);
  void setupAdminMenus(void);
  void setupProfileMenus(void);
  void setupUserMenus(void);
  Wt::WWidget* createToolBar(void);
  Wt::WPushButton* createTooBarButton(const std::string& icon);
  void handleRefresh(void);
  Wt::WAnchor* createLogoLink(void);
  void openFileUploadDialog(void);
  void selectFileToOpen(void);
  void openFile(const std::string& path);
  void finishFileDialog(int action);
  void scaleMap(double factor);
  Wt::WWidget* createUserHome(void);
  Wt::WAnchor* createAnchorForHomeLink(const std::string& title,
                                       const std::string& desc,
                                       const std::string& internalPath);
  void checkUserLogin(void);
  void showUserMngtPage(Wt::WStackedWidget* contents, int destination);
  void createAccountPanel(void);
  void createPasswordPanel(void);
  Wt::WComboBox* createViewSelector(void);
  void createInfoMsgBox(void);
  void showMessage(const std::string& msg, std::string status);
  void createViewAssignmentDialog(void);
  void createAboutDialog(void);
  void loadUserDashboard(void);
  void setInternalPath(const std::string& path);
  Wt::WDialog* createDialog(const std::string& title, Wt::WWidget* content=0);
};

#endif // MAINWEBWINDOW_HPP
