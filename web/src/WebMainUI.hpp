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
#include "WebBiCharts.hpp"
#include "WebUtils.hpp"
#include "WebNotificationManager.hpp"
#include "WebLicenseManager.hpp"
#include <Wt/WComboBox>
#include <Wt/WTimer>
#include <Wt/WApplication>
#include <Wt/WTabWidget>
#include <Wt/WContainerWidget>
#include <Wt/WSignal>
#include <Wt/WDatePicker>
#include <Wt/WProgressBar>
#include <Wt/WDialog>
#include <Wt/Http/Request>
#include <Wt/Http/Response>

class AuthManager;
class ViewAclManagement;
class WebPreferences;
class WebMainUI;

class CsvReportResource : public Wt::WResource
{
public:
  CsvReportResource(WebMainUI* mainUiClass, const std::string& viewName, Wt::WObject *parent = 0)
    : Wt::WResource(parent),
      m_mainUiClass(mainUiClass),
      m_viewName(viewName) { suggestFileName(Wt::WString("RealOpInsight_{1}_bireport.csv").arg(QString(viewName.c_str()).replace(" ", "_").toStdString())); }
  ~CsvReportResource(){ beingDeleted(); }
  void handleRequest(const Wt::Http::Request&, Wt::Http::Response& response);
private:
  WebMainUI* m_mainUiClass;
  std::string m_viewName;
};

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
  void enable(void) {m_mainWidget->enable();}
  void disbale(void) {m_mainWidget->disable();}
  void startTimer(void);
  void handleRefresh(void);
  Wt::Signal<void>& terminateSession(void) {return m_terminateSession;}
  virtual void 	refresh () {handleRefresh();}
  DbSession* dbSession(void) {return m_dbSession;}
  long reportStartTime(void){ return Wt::WDateTime(m_reportStartDatePicker->date()).toTime_t();}
  long reportEndTime(void) {return Wt::WDateTime(m_reportEndDatePicker->date()).toTime_t();}


public Q_SLOTS:
  void resetTimer(qint32 interval);
  void handleLibError(QString msg) {showMessage(ngrt4n::OperationSucceeded, msg.toStdString());}
  void setDashboardAsFrontStackedWidget(WebDashboard* dashboard);
  void setWidgetAsFrontStackedWidget(Wt::WWidget* widget);
  void resetViewSelectionBox(void) { m_selectViewBreadCrumbsBox->setCurrentIndex(0); m_displayOnlyTroubleEventsBox->setHidden(true);}

private:
  enum FileDialogAction {
    IMPORT = 0,
    OPEN = 1
  };
  typedef QMap<QString, WebDashboard*> DashboardMapT;
  //FIXME: typedef std::map<std::string, QosTrendsChart*> QosTrendsChartMapT;
  typedef std::map<std::string, WebPieChart*> QosTrendsChartMapT;
  typedef std::map<std::string, RawQosTrendsChart*> RawQosTrendsChartMapT;
  typedef QMap<std::string, Wt::WTemplate*> ThumbnailMapT;
  ThumbnailMapT m_thumbnailItems;

  /** Signals */
  Wt::Signal<void> m_terminateSession;

  /** Private members **/
  QMap<int,Wt::WAnchor*> m_menuLinks;
  std::string m_rootDir;
  std::string m_confdir;
  Wt::WContainerWidget* m_mainWidget;
  Wt::WWidget* m_settingsPageWidget;
  Wt::WTemplate* m_opsHomeTpl;
  Settings* m_settings;
  Wt::WText* m_infoBox;

  WebNotificationManager* m_notificationManager;
  Wt::WWidget* m_notificationSection;
  std::map<int, Wt::WText*> m_notificationBoxes;

  AuthManager* m_authManager;
  DbSession* m_dbSession;
  WebPreferences* m_preferences;
  WebLicenseManager* m_licenseMngtForm;
  Wt::WTimer m_timer;
  Wt::WStackedWidget* m_mainStackedContents;
  Wt::WStackedWidget* m_adminStackedContents;
  Wt::WNavigationBar* m_navbar;
  Wt::WStackedWidget* m_dashboardStackedContents;
  Wt::WDialog* m_fileUploadDialog;
  Wt::WFileUpload* m_uploader;
  std::string m_selectedFile;
  DashboardMapT m_dashboards;
  DbUserManager* m_dbUserManager;
  LdapUserManager* m_ldapUserManager;
  UserFormView* m_userAccountForm;
  UserFormView* m_changePasswordPanel;
  ViewAclManagement* m_viewAccessPermissionForm;
  Wt::WDialog* m_aboutDialog;
  int m_assignedDashboardCount;
  Wt::WText* m_adminPanelTitle;
  WebDashboard* m_currentDashboard;
  Wt::WVBoxLayout* m_eventFeedLayout;
  QosTrendsChartMapT m_qosCharts;
  RawQosTrendsChartMapT m_rawQosCharts;
  QosDataByViewMapT m_qosData;

  Wt::WDatePicker* m_reportStartDatePicker;
  Wt::WDatePicker* m_reportEndDatePicker;
  Wt::WAnchor* m_reportApplyAnchor;

  Wt::WComboBox* m_selectViewBreadCrumbsBox;
  Wt::WCheckBox* m_displayOnlyTroubleEventsBox;

  /** member methods with return value*/
  Wt::WAnchor* createLogoLink(void);
  Wt::WWidget* createSettingsPage(void);
  Wt::WDialog* createDialog(const std::string& title, Wt::WWidget* content=0);
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
  void handleImportation(void);
  void handlePreview(void);

  /** other member functions */
  void addEvents(void);
  void setupMainUI(void);
  void setupInfoBox(void);
  void setupProfileMenus(void);
  void setupMenus(void);
  void openFileUploadDialog(void);
  void selectFileToOpen(void);
  void initOperatorDashboard(void);
  void loadView(const std::string& path, WebDashboard*& dashboard);

  void scaleMap(double factor);
  UserFormView* createAccountPanel(void);
  UserFormView* createPasswordPanel(void);
  Wt::WDialog* createAboutDialog(void);
  void showMessage(int status, const std::string& msg);
  void showMessageClass(const std::string& msg, std::string statusCssClass);
  void setInternalPath(const std::string& path);
  bool createDirectory(const std::string& path, bool cleanContent);
  void startDashbaordUpdate(void);
  void updateBiCharts(void);
  void updateViewBiCharts(const std::string& viewName);
  void hideAdminSettingsMenu(void);
  void showConditionalUiWidgets(void);

  WebNotificationManager* createNotificationManager(void);
  Wt::WNavigationBar* createNavivationBar(void);
  Wt::WWidget* createBreadCrumbsBar(void);
  Wt::WStackedWidget* createMainStackedContent(void);
  Wt::WAnchor* createShowSettingsBreadCrumbsLink(void);
  Wt::WAnchor* createShowOpsHomeBreadCrumbsLink(void);
  Wt::WComboBox* createShowViewBreadCrumbsLink(void);
  Wt::WCheckBox* createDisplayOnlyTroubleBreadCrumbsLink();
  Wt::WDatePicker* createReportDatePicker(long epochDatetime);
  Wt::WContainerWidget* createReportSectionHeader(void);
  Wt::WContainerWidget* createReportExportLinks(const std::string& viewName);
  Wt::WWidget* createNotificationSection(void);
  Wt::WTemplate* createOpsHomeTpl(Wt::WContainerWidget* thumbnailsContainer, Wt::WContainerWidget* eventFeedContainer);
  void updateLicenseMgntForm();
};

#endif // MAINWEBWINDOW_HPP
