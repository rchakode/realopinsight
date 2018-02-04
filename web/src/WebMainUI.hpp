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

#include "dbo/DbSession.hpp"
#include "dbo/UserManagement.hpp"
#include "dbo/LdapUserManager.hpp"
#include "WebDashboard.hpp"
#include "WebBiDashlet.hpp"
#include "WebUtils.hpp"
#include "WebMsgDialog.hpp"
#include "WebNotificationSettings.hpp"
#include "WebAuthSettings.hpp"
#include "WebDataSourceSettings.hpp"
#include "WebDatabaseSettings.hpp"
#include "WebCsvReportResource.hpp"
#include "WebEditor.hpp"
#include <Wt/WComboBox>
#include <Wt/WTimer>
#include <Wt/WApplication>
#include <Wt/WTabWidget>
#include <Wt/WContainerWidget>
#include <Wt/WSignal>
#include <Wt/WProgressBar>
#include <Wt/WDialog>
#include <Wt/WNavigationBar>
#include <Wt/WFileUpload>

class AuthManager;
class ViewAclManagement;
class WebMainUI;


/**
 * @brief The WebMainUI class
 */
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
    MenuNotificationSettings,
    MenuDatabaseSettings,
    MenuMyAccount,
    MenuChangePassword
  };

public:
  WebMainUI(AuthManager* authManager);
  virtual ~WebMainUI();
  void showUserHome(void);
  QString getConfig (void) const {return m_currentDashboard->config();}
  void enable(void) {m_mainWidget.enable();}
  void disbale(void) {m_mainWidget.disable();}
  void startTimer(void);
  void handleRefresh(void);
  Wt::Signal<void>& terminateSession(void) {return sessionTerminated;}
  virtual void 	refresh () {handleRefresh();}
  DbSession* dbSession(void) {return m_dbSession;}

public Q_SLOTS:
  void resetTimer(qint32 interval);
  void handleLibError(QString msg) {showMessage(ngrt4n::OperationSucceeded, msg.toStdString());}
  void setDashboardAsFrontStackedWidget(WebDashboard* dashboard);
  void setWidgetAsFrontStackedWidget(Wt::WWidget* widget);
  void resetViewSelectionBox(void) { m_selectViewBox->setCurrentIndex(0); m_displayOnlyTroubleEventsBox->setHidden(true);}
  void showMessage(int status, const std::string& msg);
  void handleDashboardSelected(std::string viewName);
  void handleReportPeriodChanged(long start, long end);
  void handleShowExecutiveView(void);
  void handleShowSettingsView(void);
  void handleNewViewSelected(void);



private:
  enum FileDialogAction
  {
    IMPORT = 0,
    OPEN = 1
  };
  /** Signals */
  Wt::Signal<void> sessionTerminated;
  typedef QMap<QString, WebDashboard*> DashboardMapT;
  typedef QMap<std::string, Wt::WTemplate*> ThumbnailMapT;


  /** Private members **/
  SettingFactory m_settings;
  Wt::WTimer m_globalTimer;
  Wt::WText m_infoBox;
  QMap<int,Wt::WAnchor*> m_menuLinks;
  std::string m_rootDir;
  std::string m_confdir;
  Wt::WContainerWidget m_mainWidget;
  Wt::WTemplate m_settingsMainPageTpl;
  Wt::WTemplate m_operatorHomeTpl;
  Wt::WTemplate m_adminHomePageTpl;
  Wt::WTemplate* m_breadcrumbsBar;
  AuthManager* m_authManager;
  DbSession* m_dbSession;
  std::map<int, Wt::WText*> m_notificationBoxes;
  DbUserManager* m_dbUserManager;
  LdapUserManager* m_ldapUserManager;
  UserFormView* m_userAccountForm;
  UserFormView* m_changePasswordPanel;
  ViewAclManagement* m_viewAccessPermissionForm;
  Wt::WDialog* m_aboutDialog;
  WebDashboard* m_currentDashboard;
  Wt::WComboBox* m_selectViewBox;
  Wt::WCheckBox* m_displayOnlyTroubleEventsBox;

  WebDataSourceSettings m_dataSourceSettingsForm;
  WebNotificationSettings m_notificationSettingsForm;
  WebAuthSettings m_authSettingsForm;
  WebDatabaseSettings m_databaseSettingsForm;
  WebEditor m_webEditor;

  Wt::WNavigationBar m_navbar;
  Wt::WStackedWidget m_mainStackedContents;
  Wt::WStackedWidget m_adminStackedContents;
  Wt::WStackedWidget m_dashboardStackedContents;

  DashboardMapT m_dashboardMap;
  Wt::WText m_adminPanelTitle;
  Wt::WDialog m_previewDialog;
  std::string m_fileToPreview;

  /** For file upload **/
  Wt::WFileUpload* m_fileUploader;
  Wt::WPushButton m_uploadSubmitButton;
  Wt::WPushButton m_uploadCancelButton;
  Wt::WText m_uploadFormTitle;
  Wt::WDialog m_uploadForm;

  /** Executive View widgets **/
  Wt::WGridLayout* m_thumbsLayout;
  WebMsgDialog* m_notificationManager;
  Wt::WWidget* m_notificationSection;
  Wt::WVBoxLayout* m_eventFeedLayout;
  Wt::WContainerWidget m_eventFeedsContainer;
  Wt::WContainerWidget m_thumbsContainer;
  ThumbnailMapT m_thumbsWidgets;
  WebBiDashlet m_biDashlet;


  /** callbacks */
  void updateEventFeeds(void);
  void handleAuthSystemChanged(int authSystem);
  void handleLdapUsersMenu(void);
  void handleBuiltInUsersMenu(void);
  void handleNewUserMenu(void);
  void handleViewAclMenu(void);
  void handleDeleteView(const std::string& viewName);
  void handleUserEnableStatusChanged(int status, std::string data);
  void handleShowNotificationManager(void) { m_notificationManager->show(); }
  void handleDisplayOnlyTroubleStateChanged(void);
  void handleImportDescriptionFile(void);
  void handleLaunchEditor(void);
  void handlePreview(void);
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
  //FIXME void handleImportHostgroup(const SourceT& srcInfo, const QString& hostgroup) no longer needed?
  void handleImportHostgroup(const SourceT& srcInfo, const QString& hostgroup);
  void handleUploadSubmitButton(void);
  void handleUploadCancelButton(void);
  void handleUploadFileTooLarge(void);
  void handleShowUploadForm(void);

  /** other member functions */
  void scaleMap(double factor);
  void addEvents(void);
  void selectItem4Preview(void);
  void initOperatorDashboard(void);
  void setInternalPath(const std::string& path);
  void startDashbaordUpdate(void);
  void hideAdminSettingsMenu(void);
  void showConditionalUiWidgets(void);
  void showMessageClass(const std::string& msg, std::string statusCssClass);
  void setupSettingsPage(void);
  void setupDialogsStyle(void);
  void setupMainUI(void);
  void setupInfoBox(void);
  void setupProfileMenus(void);
  void setupMenus(void);
  void setupUploadForm(void);
  void setupNavivationBar(void);
  void setupMainStackedContent(void);
  void unbindWidgets(void);
  void bindExecutiveViewWidgets(void);
  void unbindExecutiveViewWidgets(void);
  void unbindDashboardWidgets(void);
  void unbindStackedWidgets(void);
  void saveViewInfoIntoDatabase(const CoreDataT& cdata, const QString& path);
  void clearThumbnailTemplate(Wt::WTemplate* tpl);
  void fetchQosData(QosDataListMapT& qosDataMap, long start, long end);
  bool createDirectory(const std::string& path, bool cleanContent);
  void resetFileUploader(void);
  WebDashboard* loadView(const std::string& path);
  Wt::WTemplate* createBreadCrumbsBarTpl(void);
  WebMsgDialog* createNotificationManager(void);
  Wt::WComboBox* createViewSelector(void);
  UserFormView* createAccountPanel(void);
  UserFormView* createPasswordPanel(void);
  Wt::WDialog* createAboutDialog(void);
  Wt::WAnchor* createLogoLink(void);
  Wt::WWidget* createNotificationSection(void);
  Wt::WTemplate* createThumbnailWidget(Wt::WLabel* titleWidget, Wt::WLabel* problemWidget, Wt::WImage* imageWidget);
};

#endif // MAINWEBWINDOW_HPP
