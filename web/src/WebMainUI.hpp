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
#include "WebNotificationManager.hpp"
#include "WebLicenseManager.hpp"
#include "WebNotificationPreferences.hpp"
#include "WebAuthPreferences.hpp"
#include "WebDataSourcePreferences.hpp"
#include "WebHostGroupServiceMap.hpp"
#include "WebCsvReportResource.hpp"
#include <Wt/WComboBox>
#include <Wt/WTimer>
#include <Wt/WApplication>
#include <Wt/WTabWidget>
#include <Wt/WContainerWidget>
#include <Wt/WSignal>
#include <Wt/WProgressBar>
#include <Wt/WDialog>
#include <Wt/Http/Request>
#include <Wt/Http/Response>
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
  void resetViewSelectionBox(void) { m_selectViewBreadCrumbsBox->setCurrentIndex(0); m_displayOnlyTroubleEventsBox->setHidden(true);}
  void showMessage(int status, const std::string& msg);

private:
  enum FileDialogAction {
    IMPORT = 0,
    OPEN = 1
  };

  /** Signals */
  Wt::Signal<void> sessionTerminated;

  typedef QMap<QString, WebDashboard*> DashboardMapT;
  typedef QMap<std::string, Wt::WTemplate*> ThumbnailMapT;


  //FIXME: clear template before clean
  ThumbnailMapT m_thumbnailItems;

  /** Private members **/
  QosDataByViewMapT m_qosDataMap;
  QMap<int,Wt::WAnchor*> m_menuLinks;
  std::string m_rootDir;
  std::string m_confdir;
  Wt::WContainerWidget m_mainWidget;
  Wt::WTemplate m_settingsPageTpl;
  Wt::WTemplate m_operatorHomeTpl;
  Wt::WTemplate m_breadcrumbsBar;
  Settings m_settings;
  Wt::WText* m_infoBox;

  WebNotificationManager* m_notificationManager;
  Wt::WWidget* m_notificationSection;
  std::map<int, Wt::WText*> m_notificationBoxes;

  AuthManager* m_authManager;
  DbSession* m_dbSession;
  WebDataSourcePreferences m_dataSourceSettingsForm;
  WebNotificationPreferences m_notificationSettingsForm;
  WebAuthPreferences m_authSettingsForm;
  WebLicenseManager* m_licenseMngtForm;
  WebHostGroupServiceMap m_autoHostgroupImporterForm;
  Wt::WTimer m_timer;
  Wt::WStackedWidget m_mainStackedContents;
  Wt::WStackedWidget m_adminStackedContents;
  Wt::WNavigationBar m_navbar;
  Wt::WStackedWidget m_dashboardStackedContents;
  Wt::WDialog m_fileUploadDialog;
  Wt::WDialog m_previewDialog;
  Wt::WFileUpload m_fileUploader;
  std::string m_selectedFile;
  DashboardMapT m_dashboards;
  DbUserManager* m_dbUserManager;
  LdapUserManager* m_ldapUserManager;
  UserFormView* m_userAccountForm;
  UserFormView* m_changePasswordPanel;
  ViewAclManagement* m_viewAccessPermissionForm;
  Wt::WDialog* m_aboutDialog;
  int m_assignedDashboardCount;
  Wt::WText m_adminPanelTitle;
  WebDashboard* m_currentDashboard;

  Wt::WComboBox* m_selectViewBreadCrumbsBox;
  Wt::WCheckBox* m_displayOnlyTroubleEventsBox;

  /** executive view widgets **/
  Wt::WContainerWidget m_eventFeedsContainer;
  Wt::WVBoxLayout m_eventFeedLayout;
  Wt::WContainerWidget m_thumbnailContainer;
  Wt::WGridLayout m_thumbnailLayout;


  /** member methods with return value*/
  Wt::WAnchor* createLogoLink(void);
  void setupSettingsPage(void);
  void setupDialogsStyle(void);
  Wt::WComboBox* createViewSelector(void);

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
  void handleImportHostGroupAsMap(void);
  void handlePreview(void);
  void handleDataSourceSetup(void);
  void handleDisplayAuthSetup(void);
  void handleDisplayNotificationSetup(void);
  void handleDisplayChangePassword(void);
  void handleChangePassword(const std::string& login, const std::string& lastpass, const std::string& pass);
  void handleDisplayUserProfile(void);
  void handleUpdateUserAccount(const DboUserT& userToUpdate);
  void handleUpdateErrcode(int code);

  /** other member functions */
  void addEvents(void);
  void setupMainUI(void);
  void setupInfoBox(void);
  void setupProfileMenus(void);
  void setupMenus(void);
  void openFileUploadDialog(void);
  void selectItem4Preview(void);
  void initOperatorDashboard(void);
  WebDashboard* loadView(const std::string& path);

  void scaleMap(double factor);
  UserFormView* createAccountPanel(void);
  UserFormView* createPasswordPanel(void);
  Wt::WDialog* createAboutDialog(void);
  void showMessageClass(const std::string& msg, std::string statusCssClass);
  void setInternalPath(const std::string& path);
  bool createDirectory(const std::string& path, bool cleanContent);
  void startDashbaordUpdate(void);
  void updateBiCharts(void);
  void updateViewBiCharts(const std::string& viewName);
  void hideAdminSettingsMenu(void);
  void showConditionalUiWidgets(void);

  void setupNavivationBar(void);
  void setupMainStackedContent(void);
  void setupBreadCrumbsBar(void);
  void setupOperatorHomePage(Wt::WContainerWidget* thumbnailsContainer, Wt::WContainerWidget* eventFeedContainer);
  WebNotificationManager* createNotificationManager(void);
  Wt::WAnchor* createShowSettingsBreadCrumbsLink(void);
  Wt::WAnchor* createShowOpsHomeBreadCrumbsLink(void);
  Wt::WComboBox* createShowViewBreadCrumbsLink(void);
  Wt::WCheckBox* createDisplayOnlyTroubleBreadCrumbsLink();
  Wt::WWidget* createNotificationSection(void);
  void updateLicenseMgntForm();
  void unbindWidgets(void);
  void unbindThumbnailWidgets(void);
  void unbindDashboardWidgets(void);
  void handleImportHostgroupSubmitted(const SourceT& srcInfo, const QString& hostgroup);
  void dbsaveBusinessServiceInfo(const CoreDataT& cdata, const QString& path);

  Wt::WTemplate* createThumbnailWidget(Wt::WLabel* titleWidget, Wt::WLabel* problemWidget, Wt::WImage* imageWidget);
  void clearThumbnailTemplate(Wt::WTemplate* tpl);
};

#endif // MAINWEBWINDOW_HPP
