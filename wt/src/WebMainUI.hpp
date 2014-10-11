/*
 * MainWebWindow.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
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


#ifndef MAINWEBWINDOW_HPP
#define MAINWEBWINDOW_HPP

#include "WebDashboard.hpp"
#include "DbSession.hpp"
#include "UserManagement.hpp"
#include <Wt/WTimer>
#include <Wt/WApplication>
#include <Wt/WTabWidget>
#include <Wt/WContainerWidget>
#include <Wt/WSignal>

class AuthManager;
class ViewAclManagement;
class WebPreferences;

class WebMainUI : public QObject, public Wt::WContainerWidget
{
  Q_OBJECT

  enum MenuEntriesT {
    MenuWelcome,
    MenuImport,
    MenuPreview,
    MenuViewAndAcl,
    MenuNewUser,
    MenuBuiltInUsers,
    MenuLdapUsers,
    MenuMonitoringSettings,
    MenuAuthSettings,
    MenuMyAccount,
    MenuChangePassword
  };

  enum OperationStatusT {
    OperationSuccess,
    OperationError
  };

public:
  WebMainUI(AuthManager* authManager);
  virtual ~WebMainUI();
  void showUserHome(void);
  QString getConfig (void) const {return m_currentDashboardPtr->config();}
  void enable(void) {m_mainWidget->enable();}
  void disbale(void) {m_mainWidget->disable();}
  void startTimer(void);
  Wt::WContainerWidget* get(void) {return m_mainWidget;}
  void handleRefresh(void);
  Wt::Signal<void>& terminateSession(void) {return m_terminateSession;}
  virtual void 	refresh () {handleRefresh();}


public Q_SLOTS:
  void resetTimer(qint32 interval);
  void handleLibError(QString msg) {showMessage(msg.toStdString(), OperationSuccess);}
  void openViewTab(Wt::WWidget* viewWidget) {m_dashtabs->setCurrentWidget(viewWidget);}

private:
  enum FileDialogAction {
    IMPORT = 0,
    OPEN = 1
  };
  typedef std::map<QString, WebDashboard*> DashboardListT;
  typedef std::map<QString, Wt::WMenuItem*> DashTabWidgetsT;

  /** Signals */
  Wt::Signal<void> m_terminateSession;

  /** Private members **/
  QMap<int,Wt::WAnchor*> m_menuLinks;
  std::string m_rootDir;
  std::string m_confdir;
  Wt::WContainerWidget* m_mainWidget;
  Settings* m_settings;
  Wt::WText* m_infoBox;
  std::map<int, Wt::WText*> m_notificationBoxes;
  AuthManager* m_authManager;
  DbSession* m_dbSession;
  WebPreferences* m_preferences;
  Wt::WTimer m_timer;
  Wt::WStackedWidget* m_contents;
  Wt::WStackedWidget* m_mgntContentWidgets;
  Wt::WNavigationBar* m_navbar;
  Wt::WTabWidget* m_dashtabs;
  Wt::WDialog* m_fileUploadDialog;
  Wt::WFileUpload* m_uploader;
  std::string m_selectedFile;
  DashboardListT m_dashboards;
  DbUserManager* m_dbUserManager;
  LdapUserManager* m_ldapUserManager;
  UserFormView* m_userAccountForm;
  UserFormView* m_changePasswordPanel;
  ViewAclManagement* m_viewAccessPermissionForm;
  Wt::WDialog* m_aboutDialog;
  int m_assignedDashboardCount;
  Wt::WText* m_adminPanelTitle;
  bool m_showSettingTab;
  WebDashboard* m_currentDashboardPtr;
  Wt::WVBoxLayout* m_eventFeedLayout;
  DashTabWidgetsT m_dashTabWidgets;


  /** member methods with return value*/
  Wt::WAnchor* createLogoLink(void);
  Wt::WTemplate* getDashboardThumbnail(WebDashboard* dashboard);
  Wt::WWidget* createSettingPage(void);
  Wt::WDialog* createDialog(const std::string& title, Wt::WWidget* content=0);
  Wt::WComboBox* createViewSelector(void);

  /** member methods without return value*/
  void addEvents(void);
  void createMainUI(void);
  void setupProfileMenus(void);
  void setupMenus(void);
  void openFileUploadDialog(void);
  void selectFileToOpen(void);
  void initOperatorDashboard(void);
  void loadView(const std::string& path, WebDashboard*& dashboard);
  void finishFileDialog(int action);
  void scaleMap(double factor);
  void createAccountPanel(void);
  void createPasswordPanel(void);
  void createAboutDialog(void);
  void showMessage(const std::string& msg, int status);
  void showMessageClass(const std::string& msg, std::string statusCssClass);
  void setInternalPath(const std::string& path);
  bool createDirectory(const std::string& path, bool cleanContent);
  void startDashbaordUpdate(void);
  void updateEventFeeds(void);
  void handleInternalPath(void);
  void handleAuthSystemChanged(int authSystem);
  void handleLdapUsersMenu(void);
  void handleBuiltInUsersMenu(void);
  void handleNewUserMenu(void);
  void handleViewAclMenu(void);
  void handleUserEnableStatusChanged(int status, std::string data);
};

#endif // MAINWEBWINDOW_HPP
