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

#include "dbo/src/DbSession.hpp"
#include "dbo/src/UserManagement.hpp"
#include "dbo/src/LdapUserManager.hpp"
#include "WebDashboard.hpp"
#include "WebPlatformStatusAnalyticsCharts.hpp"
#include "WebUtils.hpp"
#include "WebMsgDialog.hpp"
#include "WebNotificationSettings.hpp"
#include "WebAuthSettings.hpp"
#include "WebDataSourceSettings.hpp"
#include "WebDatabaseSettings.hpp"
#include "WebCsvReportResource.hpp"
#include "WebInputField.hpp"
#include "WebEditor.hpp"
#include <Wt/WComboBox.h>
#include <Wt/WTimer.h>
#include <Wt/WApplication.h>
#include <Wt/WTabWidget.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>
#include <Wt/WProgressBar.h>
#include <Wt/WDialog.h>
#include <Wt/WNavigationBar.h>

class AuthManager;
class ViewAccessControl;
class WebMainUI;


/**
 * @brief The WebMainUI class
 */
class WebMainUI : public QObject, public Wt::WContainerWidget
{
  Q_OBJECT

  enum MenuEntriesT {
    MenuWelcome,
    MenuEditor,
    MenuImport,
    MenuPreview,
    MenuViewAndAcl,
    MenuNewUser,
    MenuBuiltInUsers,
    MenuLdapUsers,
    MenuMonitoringSettings,
    MenuAuthSettings,
    MenuNotificationSettings,
    MenuDatabaseSettings,
    MenuMyAccount,
    MenuChangePassword,
    MenuExecutiveView,
    MenuPlatformStatusAnalytics
  };

public:
  WebMainUI(AuthManager* authManager);
  virtual ~WebMainUI();
  void enable(void) {
    m_mainWidgetRef->enable();
  }
  void disable(void) {
    m_mainWidgetRef->disable();
  }
  void handleRefresh(void);
  Wt::Signal<>& signoutTriggered(void) {
    return m_signoutTriggered;
  }
  virtual void 	refresh () {
    handleRefresh();
  }
  DbSession* dbSession(void) {
    return m_dbSession;
  }

public Q_SLOTS:
  void showAppBoard(WebDashboard* appBoard);
  void switchFeaturePanel(Wt::WWidget* item);
  void showMessage(int status, const std::string& msg);
  void showProgressMessage(const std::string& msg) {showMessage(ngrt4n::OperationInProgress, msg); }
  void handleDashboardSelected(std::string viewName);
  void handleReportPeriodChanged(long start, long end);
  void handleShowSettingsView(void);
  void handleBoardSelectionChanged(void);
  void handleSignout(void) {
    signoutTriggered().emit();
  }
  void removeGlobalWidget() {
    wApp->domRoot()->removeWidget(&m_previewInput);
  }

private:
  Wt::Signal<> m_signoutTriggered;

  /** Private members **/
  WebBaseSettings m_settings;
  Wt::WTimer m_globalTimer;
  Wt::WText* m_infoBoxRef;
  QMap<int,Wt::WAnchor*> m_menuLinks;
  QMap<int, std::string> m_menuLabels;
  Wt::WContainerWidget* m_mainWidgetRef;
  Wt::WTemplate* m_settingsPageRef;
  Wt::WTemplate* m_adminHomePageRef;
  AuthManager* m_authManager;
  DbSession* m_dbSession;
  std::map<int, Wt::WText*> m_notificationBoxes;
  std::unique_ptr<DbUserManager> m_dbUserManager;
  LdapUserManager* m_ldapUserManagerRef;
  UserFormView* m_accountPageRef;
  UserFormView* m_changePasswdPanelRef;
  ViewAccessControl* m_aclPageRef;
  std::shared_ptr<Wt::WDialog> m_aboutDialog;
  WebDashboard* m_currentAppBoard;
  Wt::WComboBox* m_boardSelectorRef;
  Wt::WCheckBox* m_showOnlyProblemsSelectorRef;

  WebDataSourceSettings* m_sourceConfigRef;
  WebNotificationSettings* m_notifPageRef;
  WebAuthSettings* m_authPageRef;
  WebDatabaseSettings* m_dbPageRef;
  WebEditor* m_webEditorRef;

  Wt::WNavigationBar* m_navbarRef;
  Wt::WStackedWidget* m_mainStackRef;
  Wt::WStackedWidget* m_adminStackRef;
  Wt::WStackedWidget* m_opsStackRef;

  QMap<QString, WebDashboard*> m_appBoards;
  Wt::WText* m_adminPageTitleRef;
  WebInputField m_previewInput;

  WebMsgDialog* m_notificationManager;
  Wt::WVBoxLayout* m_eventFeedLayoutRef;
  QMap<std::string, Wt::WTemplate*>  m_thumbnails;
  QMap<std::string, Wt::WLabel*>  m_thumbnailComments;
  Wt::WTemplate* m_executiveViewPageRef;
  WebPlatformStatusAnalyticsCharts* m_platformStatusAnalyticsChartsRef;
  Wt::WTemplate* m_platformStatusAnalyticsPageRef;


  /** callbacks */
  void handleAuthSystemChanged(int authSystem);
  void handleLdapUsersMenu(void);
  void handleManageBuiltinUsers(void);
  void handleNewUser(void);
  void handleViewAclMenu(void);
  void handleDeleteView(const std::string& viewName);
  void handleUserEnableStatusChanged(int status, std::string data);
  void handleShowNotificationManager(void) { m_notificationManager->show(); }
  void handleShowOnlyProblems(void);
  void handleLaunchEditor(void);
  void handlePreviewFile(const std::string& path, const std::string& option);
  void handleDataSourceSettings(void);
  void handleAuthSettings(void);
  void handleNotificationSettings(void);
  void handleDatabaseSettings(void);
  void handleDisplayChangePassword(void);
  void handleChangePassword(const std::string& login, const std::string& lastpass, const std::string& pass);
  void handleUserProfileSettings(void);
  void handleUpdateUserAccount(const DboUserT& userToUpdate);
  void handleErrcode(int errcode);
  void handleUserUpdatedCompleted(int errcode);
  void handleShowAdminHome(void);
  void handleHideInfoBox(Wt::WMouseEvent);

  /** other member functions */
  void scaleMap(double factor);
  void selectItem4Preview(void);
  void setInternalPath(const std::string& path);
  void startDashboardUpdate(void);
  void disableAdminFeatures(void);
  void setupMenus(void);
  void saveViewInfoIntoDatabase(const CoreDataT& cdata, const QString& path);
  bool createDirectory(const std::string& path, bool cleanContent);
  Wt::WTemplate* buildExecutiveViewPage(void);
  Wt::WTemplate* buildSlaAnalyticsPage(void);
  std::pair<WebDashboard*, QString> loadView(const std::string& path);
  std::unique_ptr<Wt::WWidget> createDisplayOptionsToolbar(void);
  std::shared_ptr<Wt::WDialog> createAboutDialog(void);
  std::unique_ptr<Wt::WAnchor> createLogoLink(void);
  std::unique_ptr<Wt::WWidget> createNotificationToolbar(void);
  std::unique_ptr<Wt::WTemplate> createThumb(std::string title, std::string msg, std::string imgURL, std::string imgLink);
};

#endif // MAINWEBWINDOW_HPP
