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
#include "WebPreferences.hpp"
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
  if (m_mainUiClass->dbSession()->fetchQosData(qosData,
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
    m_settings (new Settings()),
    m_authManager(authManager),
    m_dbSession(m_authManager->session()),
    m_preferences(new WebPreferences()),
    m_dashtabs(new Wt::WTabWidget()),
    m_fileUploadDialog(createDialog(tr("Select file to preview | %1").arg(APP_NAME).toStdString())),
    m_showSettingTab(true),
    m_currentDashboardPtr(NULL),
    m_eventFeedLayout(NULL),
    m_reportStartDatePicker(NULL),
    m_reportEndDatePicker(NULL),
    m_reportApplyAnchor(NULL)
{
  m_preferences->setEnabledInputs(false);
  createMainUI();
  createAccountPanel();
  createPasswordPanel();
  createAboutDialog();
  setupMenus();
  showUserHome();
  addEvents();
  doJavaScript(RESIZE_PANES);
}

WebMainUI::~WebMainUI()
{
  delete m_preferences;
  delete m_fileUploadDialog;
  delete m_navbar;
  delete m_contents;
  delete m_mainWidget;
  LOG("debug", "Session closed");
}


void WebMainUI::addEvents(void)
{
  m_preferences->errorOccurred().connect(std::bind(&WebMainUI::showMessage, this, std::placeholders::_1, OperationError));
  m_preferences->authSystemChanged().connect(this, &WebMainUI::handleAuthSystemChanged);
  wApp->globalKeyPressed().connect(std::bind([=](const Wt::WKeyEvent& event){}, std::placeholders::_1));
  wApp->internalPathChanged().connect(this, &WebMainUI::handleInternalPath);
  connect(m_settings, SIGNAL(timerIntervalChanged(qint32)), this, SLOT(resetTimer(qint32)));
  m_timer.timeout().connect(this, &WebMainUI::handleRefresh);

  if (m_dbSession->loggedUser().role != DboUser::AdmRole) {
    m_reportApplyAnchor->clicked().connect(this, &WebMainUI::updateBiCharts);
    m_mainNotificationIcon->clicked().connect(m_notificationManager, &WebNotificationManager::show);
  }
}

void WebMainUI::showUserHome(void)
{
  std::string homeTabTitle = "Home";
  if (m_dbSession->loggedUser().role == DboUser::AdmRole) {
    homeTabTitle = tr("Account & Settings").toStdString();
  } else {
    homeTabTitle =  tr("Operations Console").toStdString();
  }
  
  std::string pageTitle = homeTabTitle;
  pageTitle.append(" - ")
      .append(m_dbSession->loggedUser().username)
      .append(" - ")
      .append(APP_NAME.toStdString());
  wApp->setTitle(pageTitle);
  
  // data for CSS styling
  m_mainWidget->setId("maincontainer");
  m_dashtabs->addStyleClass("wrapper-container");
  m_dashtabs->addTab(createSettingPage(), tr("Account & Settings").toStdString());
  
  if (m_dbSession->loggedUser().role != DboUser::AdmRole) {
    initOperatorDashboard();
    m_dashtabs->setTabHidden(0, true);
    m_dashtabs->setCurrentIndex(1);
  }
}

void WebMainUI::createMainUI(void)
{
  m_navbar = new Wt::WNavigationBar(m_mainWidget);
  m_navbar->setResponsive(true);
  m_navbar->addWidget(createLogoLink(), Wt::AlignLeft);
  m_navbar->addWidget(new Wt::WTemplate(Wt::WString::tr("beta-message")));
  
  // Create a container for stacked contents
  m_contents = new Wt::WStackedWidget(m_mainWidget);
  m_contents->setId("stackcontentarea");
  m_contents->addWidget(m_dashtabs);
}


void WebMainUI::setupProfileMenus(void)
{
  Wt::WMenu* profileMenu = new Wt::WMenu();
  m_navbar->addMenu(profileMenu, Wt::AlignRight);
  
  if (m_dbSession->loggedUser().role != DboUser::AdmRole) {

    m_notificationManager = new WebNotificationManager(m_dbSession, m_mainWidget);

    Wt::WTemplate* notificationSectionTpl = new Wt::WTemplate(Wt::WString::tr("notification.block.tpl"));

    m_mainNotificationIcon = new Wt::WText(" ");
    m_mainNotificationIcon->setStyleClass("fa fa-bell");
    m_mainNotificationIcon->setHidden(false);
    notificationSectionTpl->bindWidget("manage-all-notification-icon", m_mainNotificationIcon);

    m_notificationBoxes[ngrt4n::Normal] = new Wt::WText("0");
    m_notificationBoxes[ngrt4n::Normal]->setStyleClass("badge severity-normal");
    m_notificationBoxes[ngrt4n::Normal]->setHidden(true);
    notificationSectionTpl->bindWidget("normal-count", m_notificationBoxes[ngrt4n::Normal]);

    m_notificationBoxes[ngrt4n::Minor] = new Wt::WText("0");
    m_notificationBoxes[ngrt4n::Minor]->setStyleClass("badge severity-minor");
    m_notificationBoxes[ngrt4n::Minor]->setHidden(true);
    notificationSectionTpl->bindWidget("minor-count", m_notificationBoxes[ngrt4n::Minor]);

    m_notificationBoxes[ngrt4n::Major] = new Wt::WText("0");
    m_notificationBoxes[ngrt4n::Major]->setStyleClass("badge severity-major");
    m_notificationBoxes[ngrt4n::Major]->setHidden(true);
    notificationSectionTpl->bindWidget("major-count", m_notificationBoxes[ngrt4n::Major]);

    m_notificationBoxes[ngrt4n::Critical] = new Wt::WText("0");
    m_notificationBoxes[ngrt4n::Critical]->setStyleClass("badge severity-critical");
    m_notificationBoxes[ngrt4n::Critical]->setHidden(true);
    notificationSectionTpl->bindWidget("critical-count", m_notificationBoxes[ngrt4n::Critical]);

    m_notificationBoxes[ngrt4n::Unknown] = new Wt::WText("0");
    m_notificationBoxes[ngrt4n::Unknown]->setStyleClass("badge severity-unknown");
    m_notificationBoxes[ngrt4n::Unknown]->setHidden(true);
    notificationSectionTpl->bindWidget("unknown-count", m_notificationBoxes[ngrt4n::Unknown]);

    m_navbar->addWidget(notificationSectionTpl, Wt::AlignRight);
  }
  
  Wt::WMenuItem* profileMenuItem
      = new Wt::WMenuItem(tr("Signed in as %1").arg(m_dbSession->loggedUser().username.c_str()).toStdString());
  Wt::WPopupMenu* profilePopupMenu = new Wt::WPopupMenu();
  profileMenuItem->setMenu(profilePopupMenu);
  profileMenu->addItem(profileMenuItem);
  
  Wt::WMenuItem* currentMenuItem = NULL;
  if (m_dbSession->loggedUser().role != DboUser::AdmRole) {
    currentMenuItem = profilePopupMenu->addItem(tr("Show Settings").toStdString());
    currentMenuItem->triggered().connect(std::bind(&WebMainUI::handleShowHideSettingsMenus, this, currentMenuItem));
  }

  currentMenuItem = profilePopupMenu->addItem(tr("Help").toStdString());
  currentMenuItem->setLink(Wt::WLink(Wt::WLink::Url, REALOPINSIGHT_GET_HELP_URL));
  currentMenuItem->setLinkTarget(Wt::TargetNewWindow);

  profilePopupMenu->addItem("About")->triggered().connect(m_aboutDialog, &Wt::WDialog::show);
}


void WebMainUI::handleShowHideSettingsMenus(Wt::WMenuItem* menuItem)
{
  if (m_showSettingTab) {
    if (m_dashtabs->count() > 1) {
      m_dashtabs->setTabHidden(0, false);
      m_dashtabs->setCurrentIndex(0);
      menuItem->setText(tr("Hide Settings").toStdString());
      wApp->doJavaScript("$('#userMenuBlock').hide(); "
                         "$('#viewMenuBlock').hide();"
                         "$('#menu-auth-settings').hide();");
      m_preferences->showAuthSettingsWidgets(false);
    }
  } else {
    if (m_dashtabs->count() > 1) {
      m_dashtabs->setTabHidden(0, true);
      m_dashtabs->setCurrentIndex(1);
      menuItem->setText(tr("Show Account & Settings").toStdString());
    }
  }
  m_showSettingTab = ! m_showSettingTab;
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
  m_reportApplyAnchor->setDisabled(true);

  std::map<int, int> problemTypeCount;
  problemTypeCount[ngrt4n::Normal]   = 0;
  problemTypeCount[ngrt4n::Minor]    = 0;
  problemTypeCount[ngrt4n::Major]    = 0;
  problemTypeCount[ngrt4n::Critical] = 0;
  problemTypeCount[ngrt4n::Unknown]  = 0;


  int currentView = 1;
  for (auto& dash : m_dashboards) {
    dash.second->initSettings(m_preferences);
    dash.second->runMonitor();
    dash.second->updateMap();
    dash.second->updateThumbnail();
    int platformSeverity = qMin(dash.second->rootNode().sev, (int)ngrt4n::Unknown);
    if (platformSeverity != ngrt4n::Normal) {
      ++problemTypeCount[platformSeverity];
    }

    QString viewName = dash.second->rootNode().name;
    m_dashTabWidgets[viewName]->setStyleClass( ngrt4n::severityCssClass(platformSeverity) );
    updateViewBiCharts(viewName.toStdString());
    ++currentView;
  }


  // Set notification only for operator console
  if (m_dbSession->loggedUser().role != DboUser::AdmRole) {
    for(auto ptype: problemTypeCount) {
      m_notificationBoxes[ptype.first]->setText(QString::number(ptype.second).toStdString());
      if (ptype.second > 0) {
        m_notificationBoxes[ptype.first]->setHidden(false);
      } else {
        m_notificationBoxes[ptype.first]->setHidden(true);
      }
    }
    updateEventFeeds();
  } // notification section

  m_reportApplyAnchor->setDisabled(false);
  startTimer();
}

Wt::WAnchor* WebMainUI::createLogoLink(void)
{
  Wt::WAnchor* anchor = new Wt::WAnchor(Wt::WLink("http://realopinsight.com/"),
                                        new Wt::WImage("images/built-in/logo-mini.png"));
  anchor->setTarget(Wt::TargetNewWindow);
  return anchor;
}

void WebMainUI::selectFileToOpen(void)
{
  m_fileUploadDialog->setWindowTitle(tr("Select the platform to preview").toStdString());
  Wt::WContainerWidget* container(new Wt::WContainerWidget(m_fileUploadDialog->contents()));
  container->clear();
  
  container->setMargin(10, Wt::All);
  container->addWidget(createViewSelector());
  
  // Provide a button to close the window
  Wt::WPushButton* finish(new Wt::WPushButton(tr("Preview").toStdString(), container));
  finish->clicked().connect(std::bind(&WebMainUI::finishFileDialog, this, OPEN));
  
  m_fileUploadDialog->show();
}

void WebMainUI::openFileUploadDialog(void)
{
  m_fileUploadDialog->setWindowTitle(tr("Import a description file").toStdString());
  m_fileUploadDialog->setStyleClass("Wt-dialog");
  Wt::WContainerWidget* container(new Wt::WContainerWidget(m_fileUploadDialog->contents()));
  container->clear();
  container->setMargin(10, Wt::All);
  
  m_uploader = new Wt::WFileUpload(container);
  m_uploader->uploaded().connect(std::bind(&WebMainUI::finishFileDialog, this, IMPORT));
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
    showMessage(tr("File too large.").toStdString(), OperationError);
  }));
  m_fileUploadDialog->show();
}

void WebMainUI::finishFileDialog(int action)
{
  switch(action) {
    case IMPORT:
      if (! m_uploader->empty()) {
        if (createDirectory(m_confdir, false)) { // false means don't clean the directory
          LOG("debug", "Parsing the input file");
          QString tmpFileName(m_uploader->spoolFileName().c_str());
          CoreDataT cdata;

          Parser parser(tmpFileName ,&cdata);
          connect(&parser, SIGNAL(errorOccurred(QString)), this, SLOT(handleLibError(QString)));

          if (! parser.process(false)) {
            std::string msg = tr("Invalid description file").toStdString();
            LOG("warn", msg);
            showMessage(msg, OperationError);
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
            if (m_dbSession->addView(view) != 0){
              showMessage(m_dbSession->lastError(), OperationError);
            } else {
              QString msg = tr("View added. "
                               " Name: %1\n - "
                               " Services: %2 -"
                               " Path: %3").arg(view.name.c_str(),
                                                QString::number(view.service_count),
                                                view.path.c_str());
              showMessage(msg.toStdString(), OperationSuccess);
            }
          }
        }
      }
      break;
    case OPEN:
      m_fileUploadDialog->accept();
      m_fileUploadDialog->contents()->clear();
      if (! m_selectedFile.empty()) {
        WebDashboard* dashbord;
        loadView(m_selectedFile, dashbord);
        m_selectedFile.clear();
      } else {
        showMessage(tr("No file selected").toStdString(), OperationError);
      }
      break;
    default:
      break;
  }
}

void WebMainUI::loadView(const std::string& path, WebDashboard*& dashboardWidget)
{
  try {
    dashboardWidget = NULL;
    dashboardWidget = new WebDashboard(path.c_str(), m_eventFeedLayout);
    if (! dashboardWidget) {
      showMessage("Cannot allocate the dashboard widget", OperationError);
      return ;
    }
    dashboardWidget->initialize(m_preferences);
    if (dashboardWidget->lastErrorState()) {
      showMessage(dashboardWidget->lastErrorMsg().toStdString(), OperationError);
      delete dashboardWidget;
      dashboardWidget = NULL;
    } else {
      QString platformName = dashboardWidget->rootNode().name;
      std::pair<DashboardListT::iterator, bool> result;
      result = m_dashboards.insert(std::pair<QString, WebDashboard*>(platformName, dashboardWidget));
      if (result.second) {
        Wt::WMenuItem* tab = m_dashtabs->addTab(dashboardWidget->getWidget(), platformName.toStdString());
        tab->triggered().connect(std::bind([=]() { m_currentDashboardPtr = dashboardWidget; }));
        m_dashTabWidgets.insert(std::pair<QString, Wt::WMenuItem*>(platformName, tab));
      } else {
        showMessage(tr("A platfom with the same name is already loaded (%1)").arg(platformName).toStdString(),
                    OperationError);
        delete dashboardWidget;
        dashboardWidget = NULL;
      }
    }
  } catch (const std::bad_alloc&) {
    std::string errorMsg = tr("Dashboard initialization failed with bad_alloc").toStdString();
    LOG("error", errorMsg);
    delete dashboardWidget;
    showMessage(errorMsg, OperationError);
  }
}

void WebMainUI::scaleMap(double factor)
{
  if (m_currentDashboardPtr) {
    m_currentDashboardPtr->map()->scaleMap(factor);
  }
}

Wt::WWidget* WebMainUI::createSettingPage(void)
{
  m_infoBox = new Wt::WText();
  m_infoBox->hide();
  m_infoBox->clicked().connect(std::bind([=](){m_infoBox->hide();}));

  m_mgntContentWidgets = new Wt::WStackedWidget(m_mainWidget);

  Wt::WTemplate* settingPageTpl = new Wt::WTemplate(Wt::WString::tr("admin-home.tpl"));
  settingPageTpl->bindWidget("title", m_adminPanelTitle = new Wt::WText(m_mainWidget));
  settingPageTpl->bindWidget("contents", m_mgntContentWidgets);
  settingPageTpl->bindWidget("info-box", m_infoBox);

  Wt::WAnchor* link = NULL;
  switch (m_dbSession->loggedUser().role) {
    case DboUser::AdmRole: {
      m_preferences->setEnabledInputs(true);
      // Start menu
      std::string menuText = QObject::tr("Welcome").toStdString();
      std::string contentTitle = QObject::tr("Getting Started in 3 Simple Steps !").toStdString();
      link = new Wt::WAnchor("#", menuText, m_mainWidget);
      settingPageTpl->bindWidget("menu-get-started", link);
      Wt::WWidget* getStartPage = new Wt::WTemplate(Wt::WString::tr("getting-started.tpl"));
      m_mgntContentWidgets->addWidget(getStartPage);
      link->clicked().connect(std::bind([=](){
        m_mgntContentWidgets->setCurrentWidget(getStartPage);
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
      m_viewAccessPermissionForm->viewDeleted().connect(std::bind([=](std::string viewName) {
        DashTabWidgetsT::iterator tabItem = m_dashTabWidgets.find(viewName.c_str());
        if (tabItem != m_dashTabWidgets.end()) {
          m_dashtabs->removeTab(tabItem->second);
          delete tabItem->second;
          m_dashTabWidgets.erase(tabItem->first);
        }
      }, std::placeholders::_1));
      m_mgntContentWidgets->addWidget(m_viewAccessPermissionForm);


      // link views and acl
      link = new Wt::WAnchor("#", menuText);
      link->clicked().connect(this, &WebMainUI::handleViewAclMenu);
      settingPageTpl->bindWidget("menu-all-views", link);
      m_menuLinks.insert(MenuViewAndAcl, link);

      // User menus
      m_dbUserManager = new DbUserManager(m_dbSession);
      m_mgntContentWidgets->addWidget(m_dbUserManager->userForm());
      m_dbUserManager->updateCompleted().connect(std::bind([=](int retCode) {
        if (retCode != 0) {
          showMessage(m_dbSession->lastError(), OperationError);
        } else {
          showMessage("Updated successfully", OperationSuccess);
          m_dbUserManager->resetUserForm();
        }
      }, std::placeholders::_1));

      // link new user
      link = new Wt::WAnchor("#", Q_TR("New User"));
      link->clicked().connect(this, &WebMainUI::handleNewUserMenu);
      settingPageTpl->bindWidget("menu-new-user", link);
      m_menuLinks.insert(MenuNewUser, link);

      // built-in menu
      m_mgntContentWidgets->addWidget(m_dbUserManager->dbUserListWidget());
      link = new Wt::WAnchor("#", Q_TR("All Users"));
      link->clicked().connect(this, &WebMainUI::handleBuiltInUsersMenu);
      settingPageTpl->bindWidget("menu-builin-users", link);
      m_menuLinks.insert(MenuBuiltInUsers, link);


      // ldap user menu
      m_ldapUserManager = new LdapUserManager(m_dbSession);
      m_mgntContentWidgets->addWidget(m_ldapUserManager);
      link = new Wt::WAnchor("#", Q_TR("LDAP Users"));
      link->clicked().connect(this, &WebMainUI::handleLdapUsersMenu);
      m_ldapUserManager->userEnableStatusChanged().connect(this, &WebMainUI::handleUserEnableStatusChanged);
      settingPageTpl->bindWidget("menu-ldap-users", link);
      m_menuLinks.insert(MenuLdapUsers, link);

    }
      break;
    default: {
      wApp->doJavaScript("$('#userMenuBlock').hide(); $('#viewMenuBlock').hide();");
      settingPageTpl->bindEmpty("menu-get-started");
      settingPageTpl->bindEmpty("menu-import");
      settingPageTpl->bindEmpty("menu-preview");
      settingPageTpl->bindEmpty("menu-all-views");
      settingPageTpl->bindEmpty("menu-new-user");
      settingPageTpl->bindEmpty("menu-all-users");
    }
      break;
  }

  // monitoring settings menu
  m_mgntContentWidgets->addWidget(m_preferences);
  link = new Wt::WAnchor("#", Q_TR("Monitoring Sources"));
  settingPageTpl->bindWidget("menu-monitoring-settings", link);
  m_menuLinks.insert(MenuMonitoringSettings, link);
  link->clicked().connect(std::bind([=](){
    m_adminPanelTitle->setText(Q_TR("Setting up Monitoring Sources"));
    m_mgntContentWidgets->setCurrentWidget(m_preferences);
    m_preferences->showMonitoringSettings();
  }));

  // auth settings menu
  m_mgntContentWidgets->addWidget(m_preferences);
  link = new Wt::WAnchor("#", Q_TR("User Authentication"));
  settingPageTpl->bindWidget("menu-auth-settings", link);
  m_menuLinks.insert(MenuAuthSettings, link);
  link->clicked().connect(std::bind([=](){
    m_adminPanelTitle->setText(Q_TR("Authentication Settings"));
    m_mgntContentWidgets->setCurrentWidget(m_preferences);
    m_preferences->showAuthSettings();
  }));

  // notification settings menu
  m_mgntContentWidgets->addWidget(m_preferences);
  link = new Wt::WAnchor("#", Q_TR("Notification Options"));
  settingPageTpl->bindWidget("menu-notification-settings", link);
  m_menuLinks.insert(MenuAuthSettings, link);
  link->clicked().connect(std::bind([=](){
    m_adminPanelTitle->setText(Q_TR("Notification Settings"));
    m_mgntContentWidgets->setCurrentWidget(m_preferences);
    m_preferences->showNotificationSettings();
  }));

  // my account menu
  m_mgntContentWidgets->addWidget(m_userAccountForm);
  link = new Wt::WAnchor("#", Q_TR("My Account"));
  settingPageTpl->bindWidget("menu-my-account", link);
  m_menuLinks.insert(MenuMyAccount, link);
  link->clicked().connect(std::bind([=](){
    m_userAccountForm->resetValidationState(false);
    m_mgntContentWidgets->setCurrentWidget(m_userAccountForm);
    m_adminPanelTitle->setText(Q_TR("My Account"));
  }));

  // change password settings
  m_mgntContentWidgets->addWidget(m_changePasswordPanel);
  link = new Wt::WAnchor("#", "Change Password");
  settingPageTpl->bindWidget("menu-change-password", link);
  m_menuLinks.insert(MenuChangePassword, link);
  link->clicked().connect(std::bind([=](){
    m_mgntContentWidgets->setCurrentWidget(m_changePasswordPanel);
    m_changePasswordPanel->reset();
    m_adminPanelTitle->setText("Change password");
  }));

  return settingPageTpl;
}


void WebMainUI::createAccountPanel(void)
{
  bool changedPassword(false);
  bool isUserForm(true);
  m_userAccountForm = new UserFormView(&(m_dbSession->loggedUser()), changedPassword, isUserForm);
  m_userAccountForm->validated().connect(std::bind([=](DboUser userToUpdate) {
    int ret = m_dbSession->updateUser(userToUpdate);
    if (ret != 0) {
      showMessage(Q_TR("Update failed, see details in log."), OperationError);
    } else {
      showMessage(Q_TR("Update completed."), OperationSuccess);
    }}, std::placeholders::_1));
}

void WebMainUI::createPasswordPanel(void)
{
  bool changedPassword(true);
  bool userForm(true);
  m_changePasswordPanel = new UserFormView(&(m_dbSession->loggedUser()),
                                           changedPassword,
                                           userForm);
  m_changePasswordPanel->changePasswordTriggered().connect(std::bind([=](const std::string& login,
                                                                     const std::string& lastpass,
                                                                     const std::string& pass) {
    int ret = m_dbSession->updatePassword(login, lastpass, pass);
    if (ret != 0) {
      showMessage(Q_TR("Change password failed, see details in log."), OperationError);
    } else {
      showMessage(Q_TR("Password updated successfully"), OperationSuccess);
    }
  }, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}



Wt::WComboBox* WebMainUI::createViewSelector(void)
{
  DbViewsT views = m_dbSession->viewList();
  
  Wt::WComboBox* viewSelector = new Wt::WComboBox();
  viewSelector->setMargin(10, Wt::Right);
  
  Wt::WStandardItemModel* viewSelectorModel = new Wt::WStandardItemModel(m_mainWidget);
  Wt::WStandardItem *item = new Wt::WStandardItem();
  item->setText(Q_TR("-- Select a description file --"));
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
  viewSelector->activated().connect(std::bind([=]() {
    int index = viewSelector->currentIndex();
    Wt::WStandardItemModel* model = static_cast<Wt::WStandardItemModel*>(viewSelector->model());
    if (index>0) {
      m_selectedFile = boost::any_cast<std::string>(model->item(index, 0)->data());
    }
  }));
  
  return viewSelector;
}


void WebMainUI::showMessage(const std::string& msg, int status)
{
  switch (status) {
    case OperationSuccess:
      showMessageClass(msg, "alert alert-success");
      break;
    case OperationError:
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
  LOG(logLevel, msg);

  m_infoBox->setText(msg);
  m_infoBox->setStyleClass(statusCssClass);
  m_infoBox->show();
}

void WebMainUI::createAboutDialog(void)
{
  m_aboutDialog = new Wt::WDialog(m_mainWidget);
  m_aboutDialog->setTitleBarEnabled(false);
  m_aboutDialog->setStyleClass("Wt-dialog");
  
  Wt::WPushButton* closeButton(new Wt::WPushButton(tr("Close").toStdString()));
  closeButton->clicked().connect(m_aboutDialog, &Wt::WDialog::accept);
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("about-tpl"), m_aboutDialog->contents());

  tpl->bindString("software", APP_NAME.toStdString());
  tpl->bindString("version", PKG_VERSION.toStdString());
  tpl->bindString("corelib-version", ngrt4n::libVersion().toStdString());
  tpl->bindString("codename", REL_NAME.toStdString());
  tpl->bindString("release-id", REL_INFO.toStdString());
  tpl->bindString("release-year", REL_YEAR.toStdString());
  tpl->bindString("bug-report-email", REPORT_BUG.toStdString());
  tpl->bindWidget("close-button", closeButton);
}


void WebMainUI::initOperatorDashboard(void)
{
  Wt::WContainerWidget* thumbs = new Wt::WContainerWidget(m_mainWidget);
  Wt::WGridLayout* thumbLayout = new Wt::WGridLayout(thumbs);
  
  Wt::WContainerWidget* bigraphs = new Wt::WContainerWidget(m_mainWidget);
  Wt::WGridLayout* bigraphsLayout = new Wt::WGridLayout(bigraphs);

  Wt::WContainerWidget* eventFeeds = new Wt::WContainerWidget(m_mainWidget);
  m_eventFeedLayout = new Wt::WVBoxLayout(eventFeeds);
  
  Wt::WTemplate* m_operatorHomeTpl = new Wt::WTemplate(Wt::WString::tr("operator-home.tpl"));
  m_operatorHomeTpl->bindWidget("info-box", m_infoBox);
  m_operatorHomeTpl->bindWidget("thumbnails", thumbs);
  m_operatorHomeTpl->bindWidget("report-period-header-pane", createReportSectionHeader());
  m_operatorHomeTpl->bindWidget("bigraphs", bigraphs);
  m_operatorHomeTpl->bindWidget("event-feeds", eventFeeds);
  m_dashtabs->addTab(m_operatorHomeTpl, Q_TR("Operations Console"));

  m_dbSession->updateViewList(m_dbSession->loggedUser().username);
  m_assignedDashboardCount = m_dbSession->viewList().size();

  // Build view thumbnails
  int thumbIndex = 0;
  for (const auto& view: m_dbSession->viewList()) {
    WebDashboard* dashboard;
    loadView(view.path, dashboard);
    if (dashboard) {
      Wt::WTemplate* thumb = getDashboardThumbnail(dashboard);
      thumb->setStyleClass("view-thumbnail");
      thumb->clicked().connect(std::bind([=](){ openViewTab(dashboard->getWidget());}));
      QObject::connect(dashboard, SIGNAL(dashboardSelected(Wt::WWidget*)), this, SLOT(openViewTab(Wt::WWidget*)));
      thumbLayout->addWidget(thumb, thumbIndex / 4, thumbIndex % 4);

      ++thumbIndex;
    }
  }


  int biIndex = 0;
  for (const auto& view: m_dbSession->viewList()) {
    m_qosCharts[view.name] = new QosTrendsChart(view.name, QosDataList());
    m_rawQosCharts[view.name] = new RawQosTrendsChart(view.name, QosDataList());
    bigraphsLayout->addWidget(new Wt::WText(Wt::WString("<h5>{1}</h5>").arg(view.name),Wt::XHTMLText), biIndex, 0);
    bigraphsLayout->addWidget(createReportExportLinks(view.name), biIndex, 1, Wt::AlignRight);

    bigraphsLayout->addWidget(m_qosCharts[view.name], ++biIndex, 0);
    bigraphsLayout->addWidget(m_rawQosCharts[view.name], biIndex, 1);
    ++biIndex;
  }

  if (thumbIndex > 0) {
    startDashbaordUpdate();
  } else {
    thumbLayout->addWidget(new Wt::WText(tr("No view to display").toStdString()), 0, 0);
  }
}


Wt::WTemplate* WebMainUI::getDashboardThumbnail(WebDashboard* dashboard)
{
  Wt::WTemplate * tpl = new Wt::WTemplate(Wt::WString::tr("dashboard-thumbnail.tpl"));
  tpl->bindWidget("thumb-titlebar", dashboard->thumbnailTitleBar());
  tpl->bindWidget("thumb-image", dashboard->thumbnail());
  return tpl;
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
    LOG("error", errMsg.toStdString());
    showMessage(errMsg.toStdString(), OperationError);
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


void WebMainUI::handleInternalPath(void)
{
  std::string path = Wt::WApplication::instance()->internalPath();
  if (path == LINK_LOAD) {
    selectFileToOpen();
    setInternalPath("");
  } else if (path == LINK_IMPORT) {
    openFileUploadDialog();
    setInternalPath("");
  } else if (path == LINK_ADMIN_HOME) {
    showUserHome();
  } else if (path == LINK_LOGIN) {
    wApp->redirect(LINK_LOGIN);
  } else {
    showMessage(Q_TR("Sorry, the request resource is not available or has been removed"),OperationError);
  }
}


void WebMainUI::handleAuthSystemChanged(int authSystem)
{
  switch (authSystem) {
    case WebPreferences::LDAP:
      m_menuLinks[MenuLdapUsers]->setDisabled(false);
      wApp->doJavaScript("$('#menu-ldap-users').prop('disabled', false);");
      break;
    default:
      m_dbSession->deleteAuthSystemUsers(WebPreferences::LDAP);
      wApp->doJavaScript("$('#menu-ldap-users').prop('disabled', true);");
      break;
  }
}

void WebMainUI::handleLdapUsersMenu(void)
{
  if (m_preferences->getAuthenticationMode() != WebPreferences::LDAP) {
    showMessage(Q_TR("Denied. LDAP authentication is disabled"), OperationError);
  } else {
    m_mgntContentWidgets->setCurrentWidget(m_ldapUserManager);
    if (m_ldapUserManager->updateUserList() <= 0) {
      showMessage(m_ldapUserManager->lastError(), OperationError);
    }
    m_adminPanelTitle->setText(Q_TR("Manage LDAP Users"));
  }
}


void WebMainUI::handleBuiltInUsersMenu(void)
{
  m_mgntContentWidgets->setCurrentWidget(m_dbUserManager->dbUserListWidget());
  m_dbUserManager->updateDbUsers();
  m_adminPanelTitle->setText(Q_TR("Manage Users"));
}


void WebMainUI::handleNewUserMenu(void)
{
  m_dbUserManager->userForm()->reset();
  m_mgntContentWidgets->setCurrentWidget(m_dbUserManager->userForm());
  m_adminPanelTitle->setText(Q_TR("Create New User"));
}


void  WebMainUI::handleViewAclMenu(void)
{
  m_mgntContentWidgets->setCurrentWidget(m_viewAccessPermissionForm);
  m_viewAccessPermissionForm->resetModelData();
  m_adminPanelTitle->setText(Q_TR("Manage Views and Access Control"));
}


void WebMainUI::handleUserEnableStatusChanged(int status, std::string data)
{
  switch (status) {
    case LdapUserManager::EnableAuthSuccess:
      showMessage(Q_TR("LDAP authentication enabled for user ") + data, OperationSuccess);
      break;
    case LdapUserManager::DisableAuthSuccess:
      showMessage(Q_TR("LDAP authentication disabled for user ") + data, OperationSuccess);
      break;
    case LdapUserManager::GenericError:
      showMessage(data, OperationError);
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
  if (m_dbSession->fetchQosData(qosData, viewName, reportStartTime(), reportEndTime()) == 0) {
    QosTrendsChartList::iterator qosChart = m_qosCharts.find(viewName);
    if (qosChart != m_qosCharts.end()) {
      qosChart->second->updateData(qosData[viewName]);
    }
    RawQosTrendsChartList::iterator rawQosChart = m_rawQosCharts.find(viewName);
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
  picker->setDate(dt.date());
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

