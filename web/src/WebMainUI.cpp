/*
 * MainWebWindow.cpp
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

#include "dbo/ViewAclManagement.hpp"
#include "AuthManager.hpp"
#include "WebMainUI.hpp"
#include "utilsCore.hpp"
#include "WebUtils.hpp"
#include <Wt/WApplication>
#include <Wt/WToolBar>
#include <Wt/WPushButton>
#include <Wt/WPopupMenu>
#include <functional>
#include <Wt/WNavigationBar>
#include <Wt/WMessageBox>
#include <Wt/WLineEdit>
#include <Wt/WImage>
#include <Wt/WFileUpload>
#include <Wt/WProgressBar>
#include <Wt/WDialog>
#include <Wt/WSelectionBox>
#include <Wt/WTemplate>
#include <Wt/WHBoxLayout>
#include <Wt/WEvent>

#define RESIZE_PANES \
  "var top = $(\"#ngrt4n-content-pane\").offset().top;" \
  "var windowHeight = $(window).height() - 40;" \
  "$(\"#ngrt4n-content-pane\").height(windowHeight - top);" \
  "$(\"#ngrt4n-side-pane\").height(windowHeight - top);"

#define LAST_30_DAYS time(NULL) - 30 * 24 * 3600

void CsvReportResource::handleRequest(const Wt::Http::Request&, Wt::Http::Response& response)
{
  response.setMimeType("text/csv");
  response.out() << "Timestamp,View,Status,Normal (%),Minor (%),Major (%),Critical (%),Unknown (%)\n";
  QosDataByViewMapT qosData;
  if (m_mainUiClass->dbSession()->listQosData(qosData,
                                              m_viewName,
                                              m_mainUiClass->reportStartTime(),
                                              m_mainUiClass->reportEndTime()) == 0) {
    for(const auto& entry: qosData[m_viewName])
      response.out() << entry.toString() << std::endl;
  }
}

WebMainUI::WebMainUI(AuthManager* authManager)
  : Wt::WContainerWidget(),
    m_terminateSession(this),
    m_rootDir("/opt/realopinsight"),
    m_confdir(m_rootDir.append("/data")),
    m_mainWidget(new Wt::WContainerWidget(this)),
    m_settingsPageWidget(NULL),
    m_opsHomeTpl(NULL),
    m_settings (new Settings()),
    m_notificationManager(NULL),
    m_notificationSection(NULL),
    m_authManager(authManager),
    m_dbSession(m_authManager->session()),
    m_licenseMngtForm(new WebLicenseManager(PKG_VERSION)),
    m_dashboardStackedContents(new Wt::WStackedWidget()),
    m_fileUploadDialog(createDialog(tr("Select file to preview | %1").arg(APP_NAME).toStdString())),
    m_currentDashboard(NULL),
    m_eventFeedLayout(NULL),
    m_reportStartDatePicker(NULL),
    m_reportEndDatePicker(NULL),
    m_reportApplyAnchor(NULL)
{
  setupMainUI();
  m_userAccountForm = createAccountPanel();
  m_changePasswordPanel = createPasswordPanel();
  m_aboutDialog = createAboutDialog();
  setupMenus();
  setupInfoBox();
  showUserHome();
  addEvents();
  doJavaScript(RESIZE_PANES);
}

WebMainUI::~WebMainUI()
{
  unbindWidgets();
  delete m_licenseMngtForm;
  delete m_fileUploadDialog;
  delete m_navbar;
  delete m_mainStackedContents;
  delete m_mainWidget;
  CORE_LOG("debug", "Session closed");
}


void WebMainUI::addEvents(void)
{
  m_dataSourceSettingsForm.operationCompleted().connect(this, &WebMainUI::showMessage);
  m_notificationSettingsForm.operationCompleted().connect(this, &WebMainUI::showMessage);
  m_authSettingsForm.operationCompleted().connect(this, &WebMainUI::showMessage);
  m_authSettingsForm.authSystemChanged().connect(this, &WebMainUI::handleAuthSystemChanged);
  m_timer.timeout().connect(this, &WebMainUI::handleRefresh);
  m_licenseMngtForm->licenseKeyChanged().connect(this, &WebMainUI::showMessage);
  connect(m_settings, SIGNAL(timerIntervalChanged(qint32)), this, SLOT(resetTimer(qint32)));
  wApp->globalKeyPressed().connect(std::bind([=](const Wt::WKeyEvent& event){}, std::placeholders::_1));
}

void WebMainUI::showUserHome(void)
{
  std::string homeTabTitle = "Home";
  if (m_dbSession->isLoggedAdmin()) {
    homeTabTitle = tr("%1 - Administration").arg(APP_NAME).toStdString();
  } else {
    homeTabTitle =  tr("%1 - Operations Console").arg(APP_NAME).toStdString();
  }
  
  std::string pageTitle = homeTabTitle;
  pageTitle.append(" - ").append(m_dbSession->loggedUser().username);

  wApp->setTitle(pageTitle);
  
  // data for CSS styling
  m_mainWidget->setId("maincontainer");
  m_dashboardStackedContents->addStyleClass("wrapper-container");
  m_dashboardStackedContents->addWidget(m_settingsPageWidget = createSettingsPage());

  if (! m_dbSession->isLoggedAdmin()) {
    initOperatorDashboard();
    m_dashboardStackedContents->setCurrentWidget(m_opsHomeTpl);
  }
}

void WebMainUI::setupMainUI(void)
{
  m_mainWidget->addWidget( createNavivationBar() );
  m_mainWidget->addWidget( createBreadCrumbsBar() );
  m_mainWidget->addWidget( createMainStackedContent() );
}


Wt::WNavigationBar* WebMainUI::createNavivationBar(void)
{
  m_navbar = new Wt::WNavigationBar();
  m_navbar->setResponsive(true);
  m_navbar->addWidget(createLogoLink(), Wt::AlignLeft);
  m_navbar->addWidget(new Wt::WTemplate(Wt::WString::tr("beta-message")));

  return m_navbar;
}

Wt::WWidget* WebMainUI::createBreadCrumbsBar(void)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("breadcrumbs-bar.tpl"));

  tpl->bindWidget("show-settings-link", createShowSettingsBreadCrumbsLink());
  tpl->bindWidget("display-view-selection-box", m_selectViewBreadCrumbsBox = createShowViewBreadCrumbsLink());
  tpl->bindWidget("display-only-trouble-event-box", m_displayOnlyTroubleEventsBox = createDisplayOnlyTroubleBreadCrumbsLink());
  if (! m_dbSession->isLoggedAdmin()) {
    tpl->bindWidget("show-home-link", createShowOpsHomeBreadCrumbsLink());
  } else {
    tpl->bindEmpty("show-home-link");
  }
  return tpl;
}

Wt::WStackedWidget* WebMainUI::createMainStackedContent(void)
{
  m_mainStackedContents = new Wt::WStackedWidget();
  m_mainStackedContents->setId("stackcontentarea");
  m_mainStackedContents->addWidget(m_dashboardStackedContents);

  return m_mainStackedContents;
}


Wt::WAnchor* WebMainUI::createShowSettingsBreadCrumbsLink(void)
{
  Wt::WAnchor* link = new Wt::WAnchor("#", "Settings");
  link->clicked().connect(std::bind([=]{
    if (m_settingsPageWidget) {
      if (! m_dbSession->isLoggedAdmin()) {
        hideAdminSettingsMenu();
      }
      setWidgetAsFrontStackedWidget(m_settingsPageWidget);
      resetViewSelectionBox();
    }
  }));
  return link;
}

Wt::WAnchor* WebMainUI::createShowOpsHomeBreadCrumbsLink(void)
{
  Wt::WAnchor* link = new Wt::WAnchor("#", "Ops Home");
  link->clicked().connect(std::bind([=]{
    if (m_opsHomeTpl) {
      setWidgetAsFrontStackedWidget(m_opsHomeTpl);
      resetViewSelectionBox();
    }
  }));
  return link;
}

Wt::WComboBox* WebMainUI::createShowViewBreadCrumbsLink(void)
{
  Wt::WComboBox* selectionBox = new Wt::WComboBox();
  if (! m_dbSession->isLoggedAdmin()) {
    selectionBox->addItem(Q_TR("Executive View"));
  } else {
    selectionBox->addItem(Q_TR("Admin Home"));
  }

  selectionBox->changed().connect(std::bind([=]() {
    QString selectedViewName = QString::fromStdString( selectionBox->currentText().toUTF8() );
    DashboardMapT::Iterator dashboardIter = m_dashboards.find(selectedViewName);
    if (dashboardIter != m_dashboards.end()) {
      setDashboardAsFrontStackedWidget(*dashboardIter);
      m_displayOnlyTroubleEventsBox->setHidden(false);
    } else {
      m_currentDashboard = NULL;
      m_displayOnlyTroubleEventsBox->setHidden(true);
      if (! m_dbSession->isLoggedAdmin()) {
        setWidgetAsFrontStackedWidget(m_opsHomeTpl);
      } else {
        setWidgetAsFrontStackedWidget(m_settingsPageWidget);
      }
    }
  }));
  return selectionBox;
}

Wt::WCheckBox* WebMainUI::createDisplayOnlyTroubleBreadCrumbsLink()
{
  Wt::WCheckBox* checkBox = new Wt::WCheckBox(Q_TR("Display only trouble events"));
  checkBox->changed().connect(this, &WebMainUI::handleDisplayOnlyTroubleStateChanged);
  checkBox->setHidden(true);
  return checkBox;
}

void WebMainUI::handleDisplayOnlyTroubleStateChanged(void)
{
  if (m_displayOnlyTroubleEventsBox && m_currentDashboard) {
    m_currentDashboard->handleShowOnlyTroubleEvents(m_displayOnlyTroubleEventsBox->checkState() == Wt::Checked);
  }
}

void WebMainUI::setupInfoBox(void)
{
  m_infoBox = new Wt::WText();
  m_infoBox->hide();
  m_infoBox->clicked().connect(std::bind([=](){m_infoBox->hide();}));
}

void WebMainUI::setupProfileMenus(void)
{
  Wt::WMenu* profileMenu = new Wt::WMenu();
  m_navbar->addMenu(profileMenu, Wt::AlignRight);
  
  if (! m_dbSession->isLoggedAdmin()) {
    m_notificationManager = createNotificationManager();
    m_notificationSection = createNotificationSection();
    m_notificationSection->setToolTip(Q_TR("Manage notifications"));
    m_navbar->addWidget(m_notificationSection, Wt::AlignRight);
  }
  
  Wt::WMenuItem* profileMenuItem = new Wt::WMenuItem(tr("Signed in as %1").arg(m_dbSession->loggedUserName()).toStdString());
  Wt::WPopupMenu* profilePopupMenu = new Wt::WPopupMenu();
  profileMenuItem->setMenu(profilePopupMenu);
  profileMenu->addItem(profileMenuItem);

  Wt::WMenuItem* currentMenuItem = profilePopupMenu->addItem(tr("Help").toStdString());
  currentMenuItem->setLink(Wt::WLink(Wt::WLink::Url, REALOPINSIGHT_GET_HELP_URL));
  currentMenuItem->setLinkTarget(Wt::TargetNewWindow);

  profilePopupMenu->addItem("About")->triggered().connect(m_aboutDialog, &Wt::WDialog::show);
}

Wt::WWidget* WebMainUI::createNotificationSection(void)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("notification.block.tpl"));

  m_notificationBoxes[ngrt4n::Normal] = new Wt::WText("0");
  m_notificationBoxes[ngrt4n::Normal]->setStyleClass("btn btn-normal");
  m_notificationBoxes[ngrt4n::Normal]->setHidden(true);
  tpl->bindWidget("normal-count", m_notificationBoxes[ngrt4n::Normal]);

  m_notificationBoxes[ngrt4n::Minor] = new Wt::WText("0");
  m_notificationBoxes[ngrt4n::Minor]->setStyleClass("btn btn-minor");
  m_notificationBoxes[ngrt4n::Minor]->setHidden(true);
  tpl->bindWidget("minor-count", m_notificationBoxes[ngrt4n::Minor]);

  m_notificationBoxes[ngrt4n::Major] = new Wt::WText("0");
  m_notificationBoxes[ngrt4n::Major]->setStyleClass("btn btn-major");
  m_notificationBoxes[ngrt4n::Major]->setHidden(true);
  tpl->bindWidget("major-count", m_notificationBoxes[ngrt4n::Major]);

  m_notificationBoxes[ngrt4n::Critical] = new Wt::WText("0");
  m_notificationBoxes[ngrt4n::Critical]->setStyleClass("btn btn-critical");
  m_notificationBoxes[ngrt4n::Critical]->setHidden(true);
  tpl->bindWidget("critical-count", m_notificationBoxes[ngrt4n::Critical]);

  m_notificationBoxes[ngrt4n::Unknown] = new Wt::WText("0");
  m_notificationBoxes[ngrt4n::Unknown]->setStyleClass("btn btn-unknown");
  m_notificationBoxes[ngrt4n::Unknown]->setHidden(true);
  tpl->bindWidget("unknown-count", m_notificationBoxes[ngrt4n::Unknown]);

  tpl->clicked().connect(this, &WebMainUI::handleShowNotificationManager);

  return tpl;
}



void WebMainUI::hideAdminSettingsMenu(void)
{
  m_notificationSettingsForm.setHidden(true);
  m_authSettingsForm.setHidden(true);
  wApp->doJavaScript("$('#userMenuBlock').hide();"
                     "$('#viewMenuBlock').hide();"
                     "$('#menu-auth-settings').hide();"
                     "$('#menu-notification-settings').hide();");
}



void WebMainUI::setupMenus(void)
{
  setupProfileMenus();
  
  //FIXME: add this after the first view loaded
  Wt::WText* text = ngrt4n::createFontAwesomeTextButton("fa fa-refresh", "Refresh the console map");
  text->clicked().connect(this, &WebMainUI::handleRefresh);
  m_navbar->addWidget(text);
  
  text = ngrt4n::createFontAwesomeTextButton("icon-zoom-in", "Zoom the console map in");
  text->clicked().connect(std::bind(&WebMainUI::scaleMap, this, ngrt4n::SCALIN_FACTOR));
  m_navbar->addWidget(text);
  
  text = ngrt4n::createFontAwesomeTextButton("icon-zoom-out","Zoom the console map out");
  text->clicked().connect(std::bind(&WebMainUI::scaleMap, this, ngrt4n::SCALOUT_FACTOR));
  m_navbar->addWidget(text);
}

void WebMainUI::startTimer(void)
{
  m_timer.setInterval(1000*m_settings->updateInterval());
  m_timer.start();
}

void WebMainUI::resetTimer(qint32 interval)
{
  m_timer.stop();
  m_timer.setInterval(interval);
  m_timer.start();
}

void WebMainUI::handleRefresh(void)
{
  m_timer.stop();

  std::map<int, int> problemTypeCount;
  problemTypeCount[ngrt4n::Normal]   = 0;
  problemTypeCount[ngrt4n::Minor]    = 0;
  problemTypeCount[ngrt4n::Major]    = 0;
  problemTypeCount[ngrt4n::Critical] = 0;
  problemTypeCount[ngrt4n::Unknown]  = 0;

  // clear the notification manager when applicable
  if (m_notificationManager) {
    m_notificationManager->clearAllServicesData();
  }

  int currentView = 1;
  for (auto& dashboard : m_dashboards) {
    dashboard->initSettings(& m_dataSourceSettingsForm);
    dashboard->runMonitor();
    dashboard->updateMap();
    dashboard->updateThumbnailInfo();

    NodeT rootService = dashboard->rootNode();
    int platformSeverity = qMin(rootService.sev, static_cast<int>(ngrt4n::Unknown));
    if (platformSeverity != ngrt4n::Normal) {
      ++problemTypeCount[platformSeverity];
      if (m_notificationManager) {
        m_notificationManager->updateServiceData(rootService);
      }
    }

    std::string rootServiceName =  rootService.name.toStdString();
    ThumbnailMapT::Iterator thumbItem = m_thumbnailItems.find(rootServiceName);
    if (thumbItem != m_thumbnailItems.end()) {
      (*thumbItem)->setStyleClass(dashboard->thumbnailCssClass());
      (*thumbItem)->setToolTip(dashboard->tooltip());
      if (m_dbSession->isCompleteUserDashboard()) {
        updateViewBiCharts(rootServiceName);
      }
    }
    ++currentView;
  }

  // Display notifications only on operator console
  if (! m_dbSession->isLoggedAdmin()) {
    for(auto severityEntry: problemTypeCount) {
      m_notificationBoxes[severityEntry.first]->setText(QString::number(severityEntry.second).toStdString());
      m_notificationBoxes[severityEntry.first]->setHidden(severityEntry.second <= 0);
    }

    updateEventFeeds();
  } // notification section

  startTimer();
}

Wt::WAnchor* WebMainUI::createLogoLink(void)
{
  Wt::WAnchor* anchor = new Wt::WAnchor(Wt::WLink(PKG_URL.toStdString()),
                                        new Wt::WImage("images/built-in/logo-mini.png"));
  anchor->setTarget(Wt::TargetNewWindow);
  return anchor;
}

void WebMainUI::selectFileToOpen(void)
{
  m_fileUploadDialog->setWindowTitle(tr("Preview | %1").arg(APP_NAME).toStdString());
  Wt::WContainerWidget* container(new Wt::WContainerWidget(m_fileUploadDialog->contents()));
  container->clear();
  
  container->setMargin(10, Wt::All);
  container->addWidget(createViewSelector());
  
  // Provide a button to close the window
  Wt::WPushButton* finish(new Wt::WPushButton(tr("Preview").toStdString(), container));
  finish->clicked().connect(this, &WebMainUI::handlePreview);
  
  m_fileUploadDialog->show();
}

void WebMainUI::openFileUploadDialog(void)
{
  m_fileUploadDialog->setWindowTitle(tr("Importation | %1").arg(APP_NAME).toStdString());
  m_fileUploadDialog->setStyleClass("Wt-dialog");

  Wt::WContainerWidget* container = m_fileUploadDialog->contents();
  container->addWidget(new Wt::WText(Q_TR("<p>Select a description file</p>"), Wt::XHTMLText));

  m_uploader = new Wt::WFileUpload(container);
  m_uploader->uploaded().connect(this, &WebMainUI::handleImportation);
  m_uploader->setFileTextSize(MAX_FILE_UPLOAD);
  m_uploader->setProgressBar(new Wt::WProgressBar());
  m_uploader->setMargin(10, Wt::Right);
  

  // Provide a button to start uploading.
  Wt::WPushButton* uploadButton = new Wt::WPushButton(tr("Upload").toStdString(), container);
  uploadButton->clicked().connect(std::bind([=](){
    m_uploader->upload();
    uploadButton->disable();
  }));
  
  // Provide a button to close the upload dialog
  Wt::WPushButton* close(new Wt::WPushButton(tr("Close").toStdString(), container));

  close->clicked().connect(std::bind([=](){
    uploadButton->enable();
    m_fileUploadDialog->accept();
    m_fileUploadDialog->contents()->clear();
  }));
  
  // React to a file upload problem.
  m_uploader->fileTooLarge().connect(std::bind([=] () {
    showMessage(ngrt4n::OperationFailed, tr("File too large.").toStdString());
  }));
  m_fileUploadDialog->show();
}


void WebMainUI::loadView(const std::string& path, WebDashboard*& dashboard)
{
  try {
    dashboard = NULL;
    dashboard = new WebDashboard(path.c_str(), m_eventFeedLayout);
    if (! dashboard) {
      showMessage(ngrt4n::OperationFailed, Q_TR("Cannot allocate the dashboard widget"));
      return ;
    }
    dashboard->initialize(& m_dataSourceSettingsForm);
    if (dashboard->lastErrorState()) {
      showMessage(ngrt4n::OperationFailed, dashboard->lastErrorMsg().toStdString());
      delete dashboard;
      dashboard = NULL;
    } else {
      QString platformName = dashboard->rootNode().name;
      DashboardMapT::Iterator result = m_dashboards.find(platformName);
      if (result != m_dashboards.end()) {
        showMessage(ngrt4n::OperationFailed,
                    tr("A platfom with the same name is already loaded (%1)").arg(platformName).toStdString());
        delete dashboard;
        dashboard = NULL;
      } else {
        m_dashboards.insert(platformName, dashboard);
        m_dashboardStackedContents->addWidget(dashboard->getWidget());
        m_selectViewBreadCrumbsBox->addItem(platformName.toStdString());
      }
    }
  } catch (const std::bad_alloc&) {
    std::string errorMsg = tr("Dashboard initialization failed with bad_alloc").toStdString();
    CORE_LOG("error", errorMsg);
    delete dashboard;
    showMessage(ngrt4n::OperationFailed, errorMsg);
  }
}

void WebMainUI::scaleMap(double factor)
{
  if (m_currentDashboard) {
    m_currentDashboard->map()->scaleMap(factor);
  }
}

Wt::WWidget* WebMainUI::createSettingsPage(void)
{
  m_adminStackedContents = new Wt::WStackedWidget(m_mainWidget);

  Wt::WTemplate* settingPageTpl = new Wt::WTemplate(Wt::WString::tr("admin-home.tpl"));
  settingPageTpl->bindWidget("title", m_adminPanelTitle = new Wt::WText(m_mainWidget));
  settingPageTpl->bindWidget("contents", m_adminStackedContents);

  Wt::WAnchor* link = NULL;
  switch (m_dbSession->loggedUser().role) {
    case DboUser::AdmRole: {
      settingPageTpl->bindWidget("info-box", m_infoBox);
      m_dataSourceSettingsForm.setEnabledInputs(true);

      // Start menu
      std::string menuText = QObject::tr("Welcome").toStdString();
      std::string contentTitle = QObject::tr("Getting Started in 3 Simple Steps !").toStdString();
      link = new Wt::WAnchor("#", menuText, m_mainWidget);
      settingPageTpl->bindWidget("menu-get-started", link);
      Wt::WWidget* getStartPage = new Wt::WTemplate(Wt::WString::tr("getting-started.tpl"));
      m_adminStackedContents->addWidget(getStartPage);
      link->clicked().connect(std::bind([=](){
        m_adminStackedContents->setCurrentWidget(getStartPage);
        m_adminPanelTitle->setText(contentTitle);
      }));
      m_menuLinks.insert(MenuWelcome, link);

      // menu view
      menuText = QObject::tr("Import").toStdString();
      link = new Wt::WAnchor("#", menuText, m_mainWidget);
      link->clicked().connect(this, &WebMainUI::openFileUploadDialog);
      settingPageTpl->bindWidget("menu-import", link);
      m_menuLinks.insert(MenuImport, link);

      // menu preview

      menuText = QObject::tr("Preview").toStdString();
      link = new Wt::WAnchor("#", menuText, m_mainWidget);
      link->clicked().connect(this, &WebMainUI::selectFileToOpen);
      settingPageTpl->bindWidget("menu-preview", link);
      m_menuLinks.insert(MenuPreview, link);

      // Create view management form
      menuText = QObject::tr("Views and Access Control").toStdString();
      m_viewAccessPermissionForm = new ViewAclManagement(m_dbSession);
      m_viewAccessPermissionForm->viewDeleted().connect(this, &WebMainUI::handleDeleteView);
      m_adminStackedContents->addWidget(m_viewAccessPermissionForm);


      // link views and acl
      link = new Wt::WAnchor("#", menuText);
      link->clicked().connect(this, &WebMainUI::handleViewAclMenu);
      settingPageTpl->bindWidget("menu-all-views", link);
      m_menuLinks.insert(MenuViewAndAcl, link);

      // User menus
      m_dbUserManager = new DbUserManager(m_dbSession);
      m_adminStackedContents->addWidget(m_dbUserManager->userForm());
      m_dbUserManager->updateCompleted().connect(std::bind([=](int retCode) {
        if (retCode != 0) {
          showMessage(ngrt4n::OperationFailed, m_dbSession->lastError());
        } else {
          showMessage(ngrt4n::OperationSucceeded, Q_TR("Updated successfully"));
          m_dbUserManager->resetUserForm();
        }
      }, std::placeholders::_1));

      // link new user
      link = new Wt::WAnchor("#", Q_TR("New User"));
      link->clicked().connect(this, &WebMainUI::handleNewUserMenu);
      settingPageTpl->bindWidget("menu-new-user", link);
      m_menuLinks.insert(MenuNewUser, link);

      // built-in menu
      m_adminStackedContents->addWidget(m_dbUserManager->dbUserListWidget());
      link = new Wt::WAnchor("#", Q_TR("All Users"));
      link->clicked().connect(this, &WebMainUI::handleBuiltInUsersMenu);
      settingPageTpl->bindWidget("menu-builin-users", link);
      m_menuLinks.insert(MenuBuiltInUsers, link);


      // ldap user menu
      m_ldapUserManager = new LdapUserManager(m_dbSession);
      m_adminStackedContents->addWidget(m_ldapUserManager);
      link = new Wt::WAnchor("#", Q_TR("LDAP Users"));
      link->clicked().connect(this, &WebMainUI::handleLdapUsersMenu);
      m_ldapUserManager->userEnableStatusChanged().connect(this, &WebMainUI::handleUserEnableStatusChanged);
      settingPageTpl->bindWidget("menu-ldap-users", link);
      m_menuLinks.insert(MenuLdapUsers, link);

    }
      break;
    default: {
      settingPageTpl->bindEmpty("info-box");
      wApp->doJavaScript("$('#userMenuBlock').hide(); $('#viewMenuBlock').hide();");
      settingPageTpl->bindEmpty("menu-get-started");
      settingPageTpl->bindEmpty("menu-import");
      settingPageTpl->bindEmpty("menu-preview");
      settingPageTpl->bindEmpty("menu-all-views");
      settingPageTpl->bindEmpty("menu-new-user");
      settingPageTpl->bindEmpty("menu-all-users");
      settingPageTpl->bindEmpty("menu-notification-settings");
    }
      break;
  }

  // monitoring settings menu
  m_adminStackedContents->addWidget(&m_dataSourceSettingsForm);
  link = new Wt::WAnchor("#", Q_TR("Monitoring Data Sources"));
  settingPageTpl->bindWidget("menu-monitoring-settings", link);
  m_menuLinks.insert(MenuMonitoringSettings, link);
  link->clicked().connect(this, &WebMainUI::handleDataSourceSetup);

  // auth settings menu
  m_adminStackedContents->addWidget(&m_authSettingsForm);
  link = new Wt::WAnchor("#", Q_TR("Authentication Backend"));
  settingPageTpl->bindWidget("menu-auth-settings", link);
  m_menuLinks.insert(MenuAuthSettings, link);
  link->clicked().connect(this, &WebMainUI::handleAuthSetup);

  // notification settings menu
  m_adminStackedContents->addWidget(& m_notificationSettingsForm);
  link = new Wt::WAnchor("#", Q_TR("Notification Settings"));
  settingPageTpl->bindWidget("menu-notification-settings", link);
  m_menuLinks.insert(MenuAuthSettings, link);
  link->clicked().connect(this, &WebMainUI::handleNotificationSetup);


  // my account menu
  m_adminStackedContents->addWidget(m_userAccountForm);
  link = new Wt::WAnchor("#", Q_TR("My Account"));
  settingPageTpl->bindWidget("menu-my-account", link);
  m_menuLinks.insert(MenuMyAccount, link);
  link->clicked().connect(this, &WebMainUI::handleDisplayUserProfile);

  // change password settings
  m_adminStackedContents->addWidget(m_changePasswordPanel);
  link = new Wt::WAnchor("#", "Change Password");
  settingPageTpl->bindWidget("menu-change-password", link);
  m_menuLinks.insert(MenuChangePassword, link);
  link->clicked().connect(this, &WebMainUI::handleChangePassword);

  // license activation menu
  if (m_dbSession->isLoggedAdmin()) {
    m_adminStackedContents->addWidget(m_licenseMngtForm);
    link = new Wt::WAnchor("#", Q_TR("Activation Key"));
    settingPageTpl->bindWidget("menu-license-activation", link);
    m_menuLinks.insert(MenuAuthSettings, link);
    link->clicked().connect(this, &WebMainUI::updateLicenseMgntForm);
  } else {
    settingPageTpl->bindEmpty("menu-license-activation");
  }


  return settingPageTpl;
}


void WebMainUI::handleDataSourceSetup(void)
{
  m_adminPanelTitle->setText(Q_TR("Monitoring Data Sources"));
  m_adminStackedContents->setCurrentWidget(&m_dataSourceSettingsForm);
  m_dataSourceSettingsForm.updateContents();
}


UserFormView* WebMainUI::createAccountPanel(void)
{
  bool changedPassword(false);
  bool isUserForm(true);
  DboUserT userInfo = m_dbSession->loggedUser().data();
  UserFormView* userAccountForm = new UserFormView(&userInfo, changedPassword, isUserForm);
  userAccountForm->validated().connect(std::bind([=](DboUserT userToUpdate) {
    int ret = m_dbSession->updateUser(userToUpdate);
    if (ret != 0) {
      showMessage(ngrt4n::OperationFailed, Q_TR("Update failed, see details in log."));
    } else {
      showMessage(ngrt4n::OperationSucceeded, Q_TR("Update completed."));
    }}, std::placeholders::_1));

  return userAccountForm;
}

UserFormView* WebMainUI::createPasswordPanel(void)
{
  bool changedPassword(true);
  bool userForm(true);
  DboUserT userInfo = m_dbSession->loggedUser().data();
  UserFormView* changePasswordPanel = new UserFormView(&userInfo, changedPassword, userForm);
  changePasswordPanel->changePasswordTriggered().connect(std::bind([=](const std::string& login,
                                                                   const std::string& lastpass,
                                                                   const std::string& pass) {
    int ret = m_dbSession->updatePassword(login, lastpass, pass);
    if (ret != 0) {
      showMessage(ngrt4n::OperationFailed, Q_TR("Failed to change password"));
    } else {
      showMessage(ngrt4n::OperationSucceeded, Q_TR("Password updated successfully"));
    }
  }, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  return changePasswordPanel;
}



Wt::WComboBox* WebMainUI::createViewSelector(void)
{
  DbViewsT views = m_dbSession->viewList();
  
  Wt::WComboBox* viewSelector = new Wt::WComboBox();
  viewSelector->setMargin(10, Wt::Right);
  
  Wt::WStandardItemModel* viewSelectorModel = new Wt::WStandardItemModel(m_mainWidget);
  Wt::WStandardItem *item = new Wt::WStandardItem();
  item->setText(Q_TR("-- Select a business view --"));
  viewSelectorModel->appendRow(item);
  
  Q_FOREACH(const DboView& view, views) {
    item = new Wt::WStandardItem();
    item->setText(view.name);
    item->setData(view.path, Wt::UserRole);
    viewSelectorModel->appendRow(item);
  }
  
  viewSelector->setModel(viewSelectorModel);
  viewSelector->setCurrentIndex(0);
  
  // Set selection action
  viewSelector->changed().connect(std::bind([=]() {
    int index = viewSelector->currentIndex();
    Wt::WStandardItemModel* model = static_cast<Wt::WStandardItemModel*>(viewSelector->model());
    if (index>0) {
      m_selectedFile = boost::any_cast<std::string>(model->item(index, 0)->data());
    }
  }));
  
  return viewSelector;
}


void WebMainUI::showMessage(int status, const std::string& msg)
{
  switch (status) {
    case ngrt4n::OperationSucceeded:
      showMessageClass(msg, "alert alert-success");
      break;
    case ngrt4n::OperationFailed:
      showMessageClass(msg, "alert alert-warning");
      break;
    default:
      break;
  }
}

void WebMainUI::showMessageClass(const std::string& msg, std::string statusCssClass)
{
  std::string logLevel = "info";
  if (statusCssClass != "alert alert-success") {
    logLevel = "error";
  }
  CORE_LOG(logLevel, msg);

  m_infoBox->setText(msg);
  m_infoBox->setStyleClass(statusCssClass);
  m_infoBox->show();
}

Wt::WDialog* WebMainUI::createAboutDialog(void)
{
  Wt::WDialog* dialog = new Wt::WDialog(m_mainWidget);
  dialog->setTitleBarEnabled(false);
  dialog->setStyleClass("Wt-dialog");
  
  Wt::WPushButton* closeButton(new Wt::WPushButton(tr("Close").toStdString()));
  closeButton->clicked().connect(dialog, &Wt::WDialog::accept);
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("about-tpl"), dialog->contents());

  tpl->bindString("software", APP_NAME.toStdString());
  tpl->bindString("version", PKG_VERSION.toStdString());
  tpl->bindString("corelib-version", ngrt4n::libVersion().toStdString());
  tpl->bindString("codename", REL_NAME.toStdString());
  tpl->bindString("release-id", REL_INFO.toStdString());
  tpl->bindString("license-offer", m_licenseMngtForm->licenseOfferName());
  tpl->bindString("release-year", REL_YEAR.toStdString());
  tpl->bindString("bug-report-email", REPORT_BUG.toStdString());
  tpl->bindWidget("close-button", closeButton);

  return dialog;
}


void WebMainUI::initOperatorDashboard(void)
{
  Wt::WContainerWidget* thumbnailsContainer = new Wt::WContainerWidget(m_mainWidget);
  Wt::WGridLayout* thumbLayout = new Wt::WGridLayout(thumbnailsContainer);

  Wt::WContainerWidget* eventFeedsContainer = new Wt::WContainerWidget(m_mainWidget);
  m_eventFeedLayout = new Wt::WVBoxLayout();
  eventFeedsContainer->setLayout(m_eventFeedLayout);
  m_opsHomeTpl = createOpsHomeTpl(thumbnailsContainer, eventFeedsContainer);
  m_dashboardStackedContents->addWidget(m_opsHomeTpl);

  m_dbSession->updateViewList(m_dbSession->loggedUser().username);
  m_assignedDashboardCount = m_dbSession->viewList().size();

  // Build view thumbnails
  int thumbIndex = 0;
  int thumbnailsPerRow = m_dbSession->dashboardTilesPerRow();
  for (const auto& view: m_dbSession->viewList()) {
    WebDashboard* dashboard;
    loadView(view.path, dashboard);
    if (dashboard) {
      Wt::WTemplate* thumbItem = new Wt::WTemplate(Wt::WString::tr("dashboard-thumbnail.tpl"));
      thumbItem->setStyleClass("btn btn-unknown");
      thumbItem->bindWidget("thumb-titlebar", dashboard->thumbnailTitleBar());
      thumbItem->bindWidget("thumb-image", dashboard->thumbnail());
      thumbItem->bindWidget("thumb-problem-details", dashboard->thumbnailProblemDetailBar());

      /** signal/slot events */
      thumbItem->clicked().connect(std::bind([=](){ setDashboardAsFrontStackedWidget(dashboard);}));
      QObject::connect(dashboard, SIGNAL(dashboardSelected(Wt::WWidget*)), this, SLOT(setWidgetAsFrontStackedWidget(Wt::WWidget*)));

      /** add to layout */
      thumbLayout->addWidget(thumbItem, thumbIndex / thumbnailsPerRow, thumbIndex % thumbnailsPerRow);

      m_thumbnailItems.insert(view.name, thumbItem);
      ++thumbIndex;
    }
  }

  showConditionalUiWidgets();

  if (thumbIndex > 0) {
    startDashbaordUpdate();
  } else {
    thumbLayout->addWidget(new Wt::WText(tr("No view to display").toStdString()), 0, 0);
  }
}


void WebMainUI::showConditionalUiWidgets(void)
{
  if (m_dbSession->isCompleteUserDashboard()) {
    Wt::WContainerWidget* reportContainer = new Wt::WContainerWidget(m_mainWidget);
    Wt::WGridLayout* reportsLayout = new Wt::WGridLayout(reportContainer);
    int biIndex = 0;
    for (const auto& view: m_dbSession->viewList()) {
      WebPieChart* piechart = new WebPieChart();
      RawQosTrendsChart* rawQosChart = new RawQosTrendsChart(view.name, QosDataList());
      piechart->setDataType(ChartBase::SLAData);
      reportsLayout->addWidget(new Wt::WText(Wt::WString("<h5>{1}</h5>").arg(view.name),Wt::XHTMLText), biIndex, 0);
      reportsLayout->addWidget(createReportExportLinks(view.name), biIndex, 1, Wt::AlignRight);
      reportsLayout->addWidget(piechart->getContainerArea(), ++biIndex, 0);
      reportsLayout->addWidget(rawQosChart, biIndex, 1);
      m_qosCharts[view.name] = piechart;
      m_rawQosCharts[view.name] = rawQosChart;
      ++biIndex;
    }
    m_opsHomeTpl->bindString("bi-report-title", Q_TR("Reports"));
    m_opsHomeTpl->bindWidget("report-period-header-pane", createReportSectionHeader());
    m_opsHomeTpl->bindWidget("bigraphs", reportContainer);
  } else {
    m_opsHomeTpl->bindEmpty("bi-report-title");
    m_opsHomeTpl->bindEmpty("report-period-header-pane");
    m_opsHomeTpl->bindEmpty("bigraphs");
    if (m_dbSession->isTileUserDashboard()) {
      doJavaScript("$('#ngrt4n-side-pane').hide();");
      doJavaScript("$('#ngrt4n-content-pane').width('100%');");
    } else {
      doJavaScript("$('#ngrt4n-side-pane').show();");
      doJavaScript("$('#ngrt4n-content-pane').width('70%');");
      doJavaScript("$('#ngrt4n-side-pane').width('28%');");
    }
  }
}

Wt::WTemplate* WebMainUI::createOpsHomeTpl(Wt::WContainerWidget* thumbnailsContainer, Wt::WContainerWidget* eventFeedContainer)
{
  Wt::WTemplate* operatorHomeTpl = new Wt::WTemplate(Wt::WString::tr("operator-home.tpl"));
  operatorHomeTpl->bindWidget("info-box", m_infoBox);
  operatorHomeTpl->bindWidget("thumbnails", thumbnailsContainer);
  operatorHomeTpl->bindWidget("event-feeds", eventFeedContainer);
  return operatorHomeTpl;
}

void WebMainUI::setInternalPath(const std::string& path)
{
  wApp->setInternalPath(path);
}

Wt::WDialog* WebMainUI::createDialog(const std::string& title, Wt::WWidget* content)
{
  Wt::WDialog* dialog = new Wt::WDialog(title);
  dialog->setStyleClass("Wt-dialog");
  dialog->titleBar()->setStyleClass("titlebar");
  if (content != NULL) dialog->contents()->addWidget(content);
  return dialog;
}

bool WebMainUI::createDirectory(const std::string& path, bool cleanContent)
{
  bool ret = false;
  QDir dir(path.c_str());
  if (! dir.exists() && ! dir.mkdir(dir.absolutePath())) {
    return false;
    QString errMsg = tr("Unable to create the directory (%1)").arg(dir.absolutePath());
    CORE_LOG("error", errMsg.toStdString());
    showMessage(ngrt4n::OperationFailed, errMsg.toStdString());
  }  else {
    ret = true;
    if (cleanContent) dir.remove("*");
  }
  return ret;
}


void WebMainUI::startDashbaordUpdate(void)
{
  Wt::WTimer* tmpTimer(new Wt::WTimer);
  tmpTimer->setInterval(2000);
  tmpTimer->start();
  tmpTimer->timeout().connect(std::bind([=](){
    tmpTimer->stop();
    delete tmpTimer;
    handleRefresh();
  }));
}


void WebMainUI::updateEventFeeds(void)
{
  //FIXME: m_eventFeedLayout->addWidget(createEventFeedItem());
}


void WebMainUI::handleAuthSystemChanged(int authSystem)
{
  switch (authSystem) {
    case WebPreferencesBase::LDAP:
      m_menuLinks[MenuLdapUsers]->setDisabled(false);
      wApp->doJavaScript("$('#menu-ldap-users').prop('disabled', false);");
      break;
    default:
      m_dbSession->deleteAuthSystemUsers(WebPreferencesBase::LDAP);
      wApp->doJavaScript("$('#menu-ldap-users').prop('disabled', true);");
      break;
  }
}

void WebMainUI::handleLdapUsersMenu(void)
{
  if (m_authSettingsForm.getAuthenticationMode() != WebPreferencesBase::LDAP) {
    showMessage(ngrt4n::OperationFailed, Q_TR("Denied, please enable LDAP authentication first"));
  } else {
    m_adminStackedContents->setCurrentWidget(m_ldapUserManager);
    if (m_ldapUserManager->updateUserList() <= 0) {
      showMessage(ngrt4n::OperationFailed, m_ldapUserManager->lastError());
    }
    m_adminPanelTitle->setText(Q_TR("Manage LDAP Users"));
  }
}


void WebMainUI::handleBuiltInUsersMenu(void)
{
  m_adminStackedContents->setCurrentWidget(m_dbUserManager->dbUserListWidget());
  m_dbUserManager->updateDbUsers();
  m_adminPanelTitle->setText(Q_TR("Manage Users"));
}


void WebMainUI::handleNewUserMenu(void)
{
  m_dbUserManager->userForm()->reset();
  m_adminStackedContents->setCurrentWidget(m_dbUserManager->userForm());
  m_adminPanelTitle->setText(Q_TR("Create New User"));
}


void  WebMainUI::handleViewAclMenu(void)
{
  m_adminStackedContents->setCurrentWidget(m_viewAccessPermissionForm);
  m_viewAccessPermissionForm->resetModelData();
  m_adminPanelTitle->setText(Q_TR("Manage Views and Access Control"));
}


void WebMainUI::handleDeleteView(const std::string& viewName)
{
  DashboardMapT::Iterator dashboardIter = m_dashboards.find(viewName.c_str());
  if (dashboardIter != m_dashboards.end()) {
    WebDashboard* dashboard = *dashboardIter;
    m_dashboardStackedContents->removeWidget(dashboard->getWidget());
    delete (*dashboardIter);
    m_dashboards.remove(viewName.c_str());
  }
}


void WebMainUI::handleUserEnableStatusChanged(int status, std::string data)
{
  switch (status) {
    case LdapUserManager::EnableAuthSuccess:
      showMessage(ngrt4n::OperationSucceeded,
                  Q_TR("LDAP authentication enabled for user ") + data);
      break;
    case LdapUserManager::DisableAuthSuccess:
      showMessage(ngrt4n::OperationSucceeded,
                  Q_TR("LDAP authentication disabled for user ") + data);
      break;
    case LdapUserManager::GenericError:
      showMessage(ngrt4n::OperationFailed, data);
      break;
    default:
      break;
  }
}


void WebMainUI::updateBiCharts(void)
{
  for (const auto& view: m_dbSession->viewList()) {
    updateViewBiCharts(view.name);
  }
}

void WebMainUI::updateViewBiCharts(const std::string& viewName)
{
  QosDataByViewMapT qosData;
  if (m_dbSession->listQosData(qosData, viewName, reportStartTime(), reportEndTime()) == 0) {
    QosTrendsChartMapT::iterator qosChart = m_qosCharts.find(viewName);
    if (qosChart != m_qosCharts.end()) {
      SLADataManager slaManager(qosData[viewName]);
      qosChart->second->setSeverityData(slaManager.normalDuration(),
                                        slaManager.minorDuration(),
                                        slaManager.majorDuration(),
                                        slaManager.criticalDuration(),
                                        slaManager.totalDuration());
      qosChart->second->repaint();
    }
    RawQosTrendsChartMapT::iterator rawQosChart = m_rawQosCharts.find(viewName);
    if (rawQosChart != m_rawQosCharts.end())
      rawQosChart->second->updateData(qosData[viewName]);
  }
}


Wt::WDatePicker* WebMainUI::createReportDatePicker(long epochDatetime)
{
  Wt::WDateTime dt;
  dt.setTime_t(epochDatetime);

  Wt::WDatePicker *picker = new Wt::WDatePicker();
  picker->setFormat("dd-MM-yyyy");
  picker->setDate(dt.date().addDays(1));
  picker->setStyleClass("inline");
  return picker;
}


Wt::WContainerWidget* WebMainUI::createReportSectionHeader(void)
{
  Wt::WContainerWidget* container = new Wt::WContainerWidget();
  Wt::WHBoxLayout* layout = new Wt::WHBoxLayout(container);

  layout->addWidget(new Wt::WText(Q_TR("Select a period")), 1);
  layout->addWidget(m_reportStartDatePicker = createReportDatePicker(LAST_30_DAYS), 1);
  layout->addWidget(new Wt::WText(Q_TR("-")), 1);
  layout->addWidget(m_reportEndDatePicker = createReportDatePicker(time(NULL)), 1);
  layout->addWidget(m_reportApplyAnchor = new Wt::WAnchor(Wt::WLink("#"), Q_TR("Apply")), 1);

  // Event
  m_reportApplyAnchor->clicked().connect(this, &WebMainUI::updateBiCharts);

  return container;
}


Wt::WContainerWidget* WebMainUI::createReportExportLinks(const std::string& viewName)
{
  Wt::WContainerWidget* container = new Wt::WContainerWidget();
  Wt::WResource *csvResource = new CsvReportResource(this, viewName, container);
  Wt::WAnchor *anchor = new Wt::WAnchor(Wt::WLink(csvResource), new Wt::WImage("images/built-in/csv-file.png"));
  anchor->setToolTip(Q_TR("Download data as CSV file"));
  anchor->setTarget(Wt::TargetNewWindow);
  return anchor;
}


WebNotificationManager* WebMainUI::createNotificationManager(void)
{
  WebNotificationManager* notificationManager = new WebNotificationManager(m_dbSession, m_mainWidget);
  notificationManager->operationCompleted().connect(this, &WebMainUI::showMessage);
  return notificationManager;
}


void WebMainUI::setDashboardAsFrontStackedWidget(WebDashboard* dashboard)
{
  if (dashboard) {
    setWidgetAsFrontStackedWidget(dashboard->getWidget());
    dashboard->triggerResizeComponents();
    int index = m_selectViewBreadCrumbsBox->findText(dashboard->rootNode().name.toStdString());
    m_selectViewBreadCrumbsBox->setCurrentIndex(index);
    m_displayOnlyTroubleEventsBox->setHidden(false);
    m_currentDashboard = dashboard;
  }
}

void WebMainUI::setWidgetAsFrontStackedWidget(Wt::WWidget* widget)
{
  if (widget) {
    m_dashboardStackedContents->setCurrentWidget(widget);
  }
}



void WebMainUI::handleImportation(void)
{
  if (! m_uploader->empty()) {
    if (createDirectory(m_confdir, false)) { // false means don't clean the directory
      CORE_LOG("debug", "Parsing the input file");
      QString tmpFileName(m_uploader->spoolFileName().c_str());
      CoreDataT cdata;

      Parser parser(tmpFileName ,&cdata);
      connect(&parser, SIGNAL(errorOccurred(QString)), this, SLOT(handleLibError(QString)));

      if (! parser.process(false)) {
        std::string msg = tr("Invalid description file").toStdString();
        CORE_LOG("warn", msg);
        showMessage(ngrt4n::OperationFailed, msg);
      } else {
        std::string filename = m_uploader->clientFileName().toUTF8();
        QString dest = tr("%1/%2").arg(m_confdir.c_str(), filename.c_str());
        QFile file(tmpFileName);
        file.copy(dest);
        file.remove();

        DboView view;
        view.name = cdata.bpnodes[ngrt4n::ROOT_ID].name.toStdString();
        view.service_count = cdata.bpnodes.size() + cdata.cnodes.size();
        view.path = dest.toStdString();
        if (! m_licenseMngtForm->canHandleNewView(m_dbSession->viewCount(), cdata.bpnodes.size() + cdata.cnodes.size()) ) {
          showMessage(ngrt4n::OperationFailed, m_licenseMngtForm->lastError());
        } else {
          if (m_dbSession->addView(view) != 0){
            showMessage(ngrt4n::OperationFailed, m_dbSession->lastError());
          } else {
            QString msg = tr("Added: %1 (%2 Services) - Path: %3")
                .arg(view.name.c_str(), QString::number(view.service_count), view.path.c_str());
            showMessage(ngrt4n::OperationSucceeded, msg.toStdString());
          }
        }
      }
    }
  }
}


void WebMainUI::handlePreview(void)
{
  m_fileUploadDialog->accept();
  m_fileUploadDialog->contents()->clear();
  if (! m_selectedFile.empty()) {
    WebDashboard* dashbord;
    loadView(m_selectedFile, dashbord);
    setDashboardAsFrontStackedWidget(dashbord);
    m_selectedFile.clear();
  } else {
    showMessage(ngrt4n::OperationFailed, tr("No item selected for preview").toStdString());
  }
}


void WebMainUI::handleAuthSetup(void)
{
  m_adminPanelTitle->setText(Q_TR("Authentication Settings"));
  m_adminStackedContents->setCurrentWidget(&m_authSettingsForm);
  m_authSettingsForm.updateContents();
}


void WebMainUI::handleNotificationSetup(void)
{
  m_adminPanelTitle->setText(Q_TR("Notification Settings"));
  m_adminStackedContents->setCurrentWidget(& m_notificationSettingsForm);
  m_notificationSettingsForm.updateContents();
}


void WebMainUI::handleDisplayUserProfile(void)
{
  m_userAccountForm->resetValidationState(false);
  m_adminStackedContents->setCurrentWidget(m_userAccountForm);
  m_adminPanelTitle->setText(Q_TR("My Account"));
}

void WebMainUI::handleChangePassword(void)
{
  m_adminStackedContents->setCurrentWidget(m_changePasswordPanel);
  m_changePasswordPanel->reset();
  m_adminPanelTitle->setText("Change password");
}



void WebMainUI::updateLicenseMgntForm()
{
  m_adminPanelTitle->setText(Q_TR("License Activation"));
  m_adminStackedContents->setCurrentWidget(m_licenseMngtForm);
  m_licenseMngtForm->updateContent();
}



void WebMainUI::unbindWidgets(void)
{
  m_adminStackedContents->removeWidget(&m_dataSourceSettingsForm);
  m_adminStackedContents->removeWidget(&m_notificationSettingsForm);
  m_adminStackedContents->removeWidget(&m_authSettingsForm);
}
