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

#include "dbo/src/ViewAccessControl.hpp"
#include "AuthManager.hpp"
#include "WebMainUI.hpp"
#include "utilsCore.hpp"
#include "WebUtils.hpp"
#include "WebInputField.hpp"
#include <functional>
#include <Wt/WApplication.h>
#include <Wt/WToolBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WMessageBox.h>
#include <Wt/WLineEdit.h>
#include <Wt/WImage.h>
#include <Wt/WProgressBar.h>
#include <Wt/WDialog.h>
#include <Wt/WSelectionBox.h>
#include <Wt/WTemplate.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WEvent.h>

#define RESIZE_PANELS \
  "var top = $(\"#ngrt4n-content-pane\").offset().top;" \
  "var windowHeight = $(window).height() - 40;" \
  "$(\"#ngrt4n-content-pane\").height(windowHeight - top);" \
  "$(\"#ngrt4n-side-pane\").height(windowHeight - top);"



WebMainUI::WebMainUI(AuthManager* authManager)
  : Wt::WContainerWidget(),
    m_authManager(authManager),
    m_dbSession(m_authManager->session()),
    m_currentAppBoard(nullptr),
    m_notificationManager(nullptr),
    m_eventFeedLayoutRef(nullptr),
    m_executiveViewPageRef(nullptr),
    m_platformStatusAnalyticsChartsRef(nullptr),
    m_platformStatusAnalyticsPageRef(nullptr)
{
  m_menuLabels[MenuExecutiveView] = Q_TR("Executive View");
  m_menuLabels[MenuPlatformStatusAnalytics] = Q_TR("Platform Availability Analytics");
  m_mainWidgetRef = addNew<Wt::WContainerWidget>();
  m_mainWidgetRef->setId("maincontainer");

  m_navbarRef = m_mainWidgetRef->addNew<Wt::WNavigationBar>();
  m_navbarRef->setResponsive(true);
  m_navbarRef->addWidget(createLogoLink(), Wt::AlignmentFlag::Left);
  m_navbarRef->addWidget(std::make_unique<Wt::WTemplate>(Wt::WString::tr("beta-message")));

  m_mainStackRef = m_mainWidgetRef->addNew<Wt::WStackedWidget>();
  m_mainStackRef->setId("stackcontentarea");

  m_previewInput.setStyleClass("Wt-dialog");
  m_previewInput.titleBar()->setStyleClass("titlebar");
  m_previewInput.dataTriggered().connect(this, &WebMainUI::handlePreviewFile);

  m_opsStackRef = m_mainStackRef->addNew<Wt::WStackedWidget>();
  m_opsStackRef->addStyleClass("wrapper-container");

  // display options
  auto displayOptionsToolbar= createDisplayOptionsToolbar();
  m_navbarRef->addWidget(std::move(displayOptionsToolbar));

  // logged user menus
  auto loggedUserPopupMenu = std::make_unique<Wt::WPopupMenu>();
  loggedUserPopupMenu->addItem(tr("Settings").toStdString())->clicked().connect(this, &WebMainUI::handleShowSettingsView);

  Wt::WMenuItem* menuHelp = loggedUserPopupMenu->addItem(tr("Help").toStdString());
  Wt::WLink link(Wt::LinkType::Url, REALOPINSIGHT_GET_HELP_URL);
  link.setTarget(Wt::LinkTarget::NewWindow);
  menuHelp->setLink(link);

  m_aboutDialog = createAboutDialog();
  loggedUserPopupMenu->addItem("About")->triggered().connect(m_aboutDialog.get(), &Wt::WDialog::show);
  loggedUserPopupMenu->addItem(tr("Sign out").toStdString())->clicked().connect(this, &WebMainUI::handleSignout);

  auto loggedUserMenuItem = std::make_unique<Wt::WMenuItem>(tr("Signed in as %1").arg(m_dbSession->loggedUserName()).toStdString());
  loggedUserMenuItem->setMenu(std::move(loggedUserPopupMenu));

  auto loggedUserMenu = std::make_unique<Wt::WMenu>();
  loggedUserMenu->addItem(std::move(loggedUserMenuItem));
  m_navbarRef->addMenu(std::move(loggedUserMenu), Wt::AlignmentFlag::Right);

  auto toobarActionIcon = ngrt4n::createFontAwesomeTextButton("fa fa-refresh", "Refresh the console map");
  toobarActionIcon->clicked().connect(this, &WebMainUI::handleRefresh);
  m_navbarRef->addWidget(std::move(toobarActionIcon));

  toobarActionIcon = ngrt4n::createFontAwesomeTextButton("fa fa-search-plus", "Zoom the console map in");
  toobarActionIcon->clicked().connect(std::bind(&WebMainUI::scaleMap, this, ngrt4n::SCALIN_FACTOR));
  m_navbarRef->addWidget(std::move(toobarActionIcon));

  toobarActionIcon = ngrt4n::createFontAwesomeTextButton("fa fa-search-minus","Zoom the console map out");
  toobarActionIcon->clicked().connect(std::bind(&WebMainUI::scaleMap, this, ngrt4n::SCALOUT_FACTOR));
  m_navbarRef->addWidget(std::move(toobarActionIcon));

  // problems icons
  if (! m_dbSession->isLoggedAdmin()) {
    m_notificationManager = new WebMsgDialog(m_dbSession);
    m_notificationManager->operationCompleted().connect(this, &WebMainUI::showMessage);
    auto notifToolbar = createNotificationToolbar();
    m_navbarRef->addWidget(std::move(notifToolbar));
  }

  auto pageTitle = (m_dbSession->isLoggedAdmin()) ? tr("%1 - Administration").arg(APP_NAME).toStdString():
                                                    tr("%1 - Operations Console").arg(APP_NAME).toStdString();
  wApp->setTitle(pageTitle.append(" - ").append(m_dbSession->loggedUser().username));

  m_settingsPageRef = m_opsStackRef->addNew<Wt::WTemplate>(Wt::WString::tr("admin-home.tpl"));
  m_adminPageTitleRef = m_settingsPageRef->bindNew<Wt::WText>("title");
  m_adminStackRef = m_settingsPageRef->bindNew<Wt::WStackedWidget>("contents");

  if (! m_dbSession->isLoggedAdmin()) {
    wApp->doJavaScript("$('#userMenuSection').hide(); $('#viewMenuBlock').hide();");
    m_settingsPageRef->bindEmpty("info-box");
    m_settingsPageRef->bindEmpty("menu-get-started");
    m_settingsPageRef->bindEmpty("menu-database-settings");
    m_settingsPageRef->bindEmpty("menu-editor");
    m_settingsPageRef->bindEmpty("menu-preview");
    m_settingsPageRef->bindEmpty("menu-all-views");
    m_settingsPageRef->bindEmpty("menu-new-user");
    m_settingsPageRef->bindEmpty("menu-all-users");
    m_settingsPageRef->bindEmpty("menu-notification-settings");
    m_executiveViewPageRef = buildExecutiveViewPage();
  } else {
    auto infoBox = std::make_unique<Wt::WText>();
    m_infoBoxRef = infoBox.get();
    infoBox->setId("info-box");
    infoBox->hide();
    infoBox->clicked().connect(this, &WebMainUI::handleHideInfoBox);
    m_settingsPageRef->bindWidget("info-box", std::move(infoBox));

    // Start menu
    auto adminHomePage = std::make_unique<Wt::WTemplate>(Wt::WString::tr("getting-started.tpl"));
    m_adminHomePageRef = adminHomePage.get();
    m_adminStackRef->addWidget(std::move(adminHomePage));

    auto linkPtr = std::make_unique<Wt::WAnchor>("#", QObject::tr(" >> Home << ").toStdString());
    linkPtr->clicked().connect(this, &WebMainUI::handleShowAdminHome);
    m_menuLinks.insert(MenuWelcome, linkPtr.get());
    m_settingsPageRef->bindWidget("menu-get-started", std::move(linkPtr));

    m_webEditorRef = m_adminStackRef->addNew<WebEditor>();
    m_webEditorRef->setConfigDir(SettingFactory::coreAppDir());
    m_webEditorRef->setDbSession(m_dbSession);
    m_webEditorRef->refreshDynamicContents();
    m_webEditorRef->operationCompleted().connect(this, &WebMainUI::showMessage);

    linkPtr = std::make_unique<Wt::WAnchor>("#", QObject::tr("Service Editor").toStdString());
    linkPtr->clicked().connect(this, &WebMainUI::handleLaunchEditor);
    m_menuLinks.insert(MenuEditor, linkPtr.get());
    m_settingsPageRef->bindWidget("menu-editor", std::move(linkPtr));

    // menu preview
    linkPtr = std::make_unique<Wt::WAnchor>("#", QObject::tr("Preview").toStdString());
    m_menuLinks.insert(MenuPreview, linkPtr.get());
    linkPtr->clicked().connect(this, &WebMainUI::selectItem4Preview);
    m_settingsPageRef->bindWidget("menu-preview", std::move(linkPtr));

    // Create view management form
    auto aclPage = std::make_unique<ViewAccessControl>(m_dbSession);
    m_aclPageRef = aclPage.get();
    aclPage->viewDeleted().connect(this, &WebMainUI::handleDeleteView);
    m_adminStackRef->addWidget(std::move(aclPage));

    // link views and acl
    linkPtr = std::make_unique<Wt::WAnchor>("#", QObject::tr("Profiles").toStdString());
    linkPtr->clicked().connect(this, &WebMainUI::handleViewAclMenu);
    m_menuLinks.insert(MenuViewAndAcl, linkPtr.get());
    m_settingsPageRef->bindWidget("menu-all-views", std::move(linkPtr));

    // link new user
    m_dbUserManager = std::make_unique<DbUserManager>(m_dbSession);
    m_adminStackRef->addWidget(m_dbUserManager->userForm());
    m_dbUserManager->updateCompleted().connect(this, &WebMainUI::handleUserUpdatedCompleted);

    linkPtr = std::make_unique<Wt::WAnchor>("#", Q_TR("New User"));
    linkPtr->clicked().connect(this, &WebMainUI::handleNewUser);
    m_menuLinks.insert(MenuNewUser, linkPtr.get());
    m_settingsPageRef->bindWidget("menu-new-user", std::move(linkPtr));

    // built-in menu
    m_adminStackRef->addWidget(m_dbUserManager->dbUserListWidget());
    linkPtr = std::make_unique<Wt::WAnchor>("#", Q_TR("All Users"));
    linkPtr->clicked().connect(this, &WebMainUI::handleManageBuiltinUsers);
    m_menuLinks.insert(MenuBuiltInUsers, linkPtr.get());
    m_settingsPageRef->bindWidget("menu-builtin-users", std::move(linkPtr));

    // ldap user menu
    auto ldapUserManager = std::make_unique<LdapUserManager>(m_dbSession);
    m_ldapUserManagerRef = ldapUserManager.get();
    m_adminStackRef->addWidget(std::move(ldapUserManager));
    m_ldapUserManagerRef->userEnableStatusChanged().connect(this, &WebMainUI::handleUserEnableStatusChanged);
    linkPtr = std::make_unique<Wt::WAnchor>("#", Q_TR("LDAP Users"));
    linkPtr->clicked().connect(this, &WebMainUI::handleLdapUsersMenu);
    m_menuLinks.insert(MenuLdapUsers, linkPtr.get());
    m_settingsPageRef->bindWidget("menu-ldap-users", std::move(linkPtr));
  }

  m_sourceConfigRef = m_adminStackRef->addNew<WebDataSourceSettings>();
  m_sourceConfigRef->setDbSession(m_dbSession);
  m_sourceConfigRef->setEnabledInputs(true);
  m_sourceConfigRef->operationCompleted().connect(this, &WebMainUI::showMessage);

  auto linkPtr = std::make_unique<Wt::WAnchor>("#", Q_TR("Data Sources"));
  linkPtr->clicked().connect(this, &WebMainUI::handleDataSourceSettings);
  m_menuLinks.insert(MenuMonitoringSettings, linkPtr.get());
  m_settingsPageRef->bindWidget("menu-monitoring-settings", std::move(linkPtr));

  m_authPageRef = m_adminStackRef->addNew<WebAuthSettings>();
  m_authPageRef->operationCompleted().connect(this, &WebMainUI::showMessage);
  m_authPageRef->authSystemChanged().connect(this, &WebMainUI::handleAuthSystemChanged);

  linkPtr = std::make_unique<Wt::WAnchor>("#", Q_TR("Authentication"));
  linkPtr->clicked().connect(this, &WebMainUI::handleAuthSettings);
  m_menuLinks.insert(MenuAuthSettings, linkPtr.get());
  m_settingsPageRef->bindWidget("menu-auth-settings", std::move(linkPtr));

  auto notifPage = std::make_unique<WebNotificationSettings>();
  m_notifPageRef = notifPage.get();
  m_notifPageRef->operationCompleted().connect(this, &WebMainUI::showMessage);
  m_adminStackRef->addWidget(std::move(notifPage));

  linkPtr = std::make_unique<Wt::WAnchor>("#", Q_TR("Notification"));
  linkPtr->clicked().connect(this, &WebMainUI::handleNotificationSettings);
  m_menuLinks.insert(MenuNotificationSettings, linkPtr.get());
  m_settingsPageRef->bindWidget("menu-notification-settings", std::move(linkPtr));

  m_dbPageRef = m_adminStackRef->addNew<WebDatabaseSettings>();
  m_dbPageRef->operationCompleted().connect(this, &WebMainUI::showMessage);

  linkPtr = std::make_unique<Wt::WAnchor>("#", Q_TR("Database Settings"));
  linkPtr->clicked().connect(this, &WebMainUI::handleDatabaseSettings);
  m_menuLinks.insert(MenuDatabaseSettings, linkPtr.get());
  m_settingsPageRef->bindWidget("menu-database-settings", std::move(linkPtr));

  auto userData = m_dbSession->loggedUser().data();
  m_accountPageRef = m_adminStackRef->addNew<UserFormView>(&userData, false, true);
  m_accountPageRef->validated().connect(this, &WebMainUI::handleUpdateUserAccount);

  linkPtr = std::make_unique<Wt::WAnchor>("#", Q_TR("My Account"));
  linkPtr->clicked().connect(this, &WebMainUI::handleUserProfileSettings);
  m_menuLinks.insert(MenuMyAccount, linkPtr.get());
  m_settingsPageRef->bindWidget("menu-my-account", std::move(linkPtr));

  m_changePasswdPanelRef = m_adminStackRef->addNew<UserFormView>(&userData, true, true);
  m_changePasswdPanelRef->changePasswordTriggered().connect(this, &WebMainUI::handleChangePassword);

  linkPtr = std::make_unique<Wt::WAnchor>("#", "Change Password");
  linkPtr->clicked().connect(this, &WebMainUI::handleDisplayChangePassword);
  m_menuLinks.insert(MenuChangePassword, linkPtr.get());
  m_settingsPageRef->bindWidget("menu-change-password", std::move(linkPtr));

  if (! m_dbSession->isLoggedAdmin()) {
    disableAdminFeatures();
    switchFeaturePanel(m_executiveViewPageRef);
    m_boardSelectorRef->setCurrentIndex(0);
    m_showOnlyProblemsSelectorRef->setHidden(true);
  }
  doJavaScript(RESIZE_PANELS);
}

WebMainUI::~WebMainUI()
{
  wApp->doJavaScript("document.location.reload(true);");
}


Wt::WTemplate* WebMainUI::buildExecutiveViewPage(void)
{
  auto executiveViewPageRef = m_opsStackRef->addNew<Wt::WTemplate>(Wt::WString::tr("ops-home.tpl"));

  auto thumbnailsLayout = std::make_unique<Wt::WGridLayout>();
  auto eventFeedLayout = std::make_unique<Wt::WVBoxLayout>();
  m_eventFeedLayoutRef = eventFeedLayout.get();
  auto eventFeedItem = std::make_unique<Wt::WContainerWidget>();
  eventFeedItem->setLayout(std::move(eventFeedLayout));
  executiveViewPageRef->bindWidget("event-feeds", std::move(eventFeedItem));

  auto infoBox = std::make_unique<Wt::WText>();
  m_infoBoxRef = infoBox.get();
  executiveViewPageRef->bindWidget("info-box", std::move(infoBox));

  auto listOfUserViews = m_dbSession->listAssignedViewsByUser(m_dbSession->loggedUser().username);
  if (listOfUserViews.empty()) {
    executiveViewPageRef->bindEmpty("thumbnails");
    showMessage(ngrt4n::OperationFailed, Q_TR("No views to visualize"));
    return executiveViewPageRef;
  }

  // Generate view cards
  int currentThumbailIndex = 0;
  int cardPerRow = m_dbSession->boardCardsPerRow();
  std::string failuresCount = "";
  for (const auto& sv : listOfUserViews) {
    auto loadResult = loadView(sv.path);
    if (! loadResult.first) {
      CORE_LOG("error", loadResult.second.toStdString());
      failuresCount.append(failuresCount.empty()? "": ", ").append(sv.name);
      continue;
    }
    auto board = loadResult.first;
    //TODO want to replace Qt signal handling by the Wt's one
    QObject::connect(board, SIGNAL(dashboardSelected(std::string)), this, SLOT(handleDashboardSelected(std::string)));

    auto thumbnail = std::make_unique<Wt::WTemplate>(Wt::WString::tr("dashboard-thumbnail.tpl"));
    auto thumbnailTitle = board->thumbTitle();
    thumbnail->setStyleClass("btn btn-unknown");
    thumbnail->bindWidget("thumb-titlebar", std::make_unique<Wt::WLabel>(thumbnailTitle));
    thumbnail->bindWidget("thumb-image", std::make_unique<Wt::WImage>(Wt::WLink(board->thumbURL())));

    thumbnail->clicked().connect(std::bind(&WebMainUI::handleDashboardSelected, this, thumbnailTitle));
    m_thumbnailComments[thumbnailTitle] = thumbnail->bindNew<Wt::WLabel>("thumb-problem-details", "");
    m_thumbnails.insert(thumbnailTitle, thumbnail.get());
    thumbnailsLayout->addWidget(std::move(thumbnail), currentThumbailIndex / cardPerRow, currentThumbailIndex % cardPerRow);
    ++currentThumbailIndex;
  }

  if (m_dbSession->displayOnlyTiles()) {
    doJavaScript("$('#ngrt4n-side-pane').hide();");
    doJavaScript("$('#ngrt4n-content-pane').removeClass().addClass('col-sm-12');");
  } else {
    doJavaScript("$('#ngrt4n-side-pane').show();");
    doJavaScript("$('#ngrt4n-content-pane').removeClass().addClass('col-sm-8');");
    doJavaScript("$('#ngrt4n-side-pane').removeClass().addClass('col-sm-4');");
  }

  if (currentThumbailIndex > 0) {
    startDashboardUpdate();
  }

  if (currentThumbailIndex != static_cast<int>(listOfUserViews.size())) {
    showMessage(ngrt4n::OperationFailed, QObject::tr("Failed to load views => %1. Check details in logs").arg(failuresCount.c_str()).toStdString());
  }
  auto thumbnails = std::make_unique<Wt::WContainerWidget>();
  thumbnails->setLayout(std::move(thumbnailsLayout));
  executiveViewPageRef->bindWidget("thumbnails", std::move(thumbnails));

  return executiveViewPageRef;
}

Wt::WTemplate* WebMainUI::buildSlaAnalyticsPage(void)
{
  auto statusAnalyticsPage = m_opsStackRef->addNew<Wt::WTemplate>(Wt::WString::tr("platform-status-analytics.tpl"));
  auto assignedViews = m_dbSession->listAssignedViewsByUser(m_dbSession->loggedUser().username);
  if (assignedViews.empty()) {
    statusAnalyticsPage->bindEmpty("platform-analytics-board-charts");
    showMessage(ngrt4n::OperationFailed, Q_TR("No views to visualize"));
    return statusAnalyticsPage;
  }

  auto statusAnalyticsBoard = std::make_unique<WebPlatformStatusAnalyticsCharts>(assignedViews, m_dbSession->listSources(MonitorT::Any));
  m_platformStatusAnalyticsChartsRef = statusAnalyticsBoard.get();
  m_platformStatusAnalyticsChartsRef->reportPeriodChanged().connect(this, &WebMainUI::handleReportPeriodChanged);
  statusAnalyticsPage->bindWidget("platform-analytics-board-charts", std::move(statusAnalyticsBoard));

  for (const auto& view: assignedViews) {
    PlatformMappedStatusHistoryT statusHistory;
    m_dbSession->listStatusHistory(statusHistory, view.name, m_platformStatusAnalyticsChartsRef->startTime(), m_platformStatusAnalyticsChartsRef->endTime());
    m_platformStatusAnalyticsChartsRef->updateByView(view.name, statusHistory);
  }

  return statusAnalyticsPage;
}


std::unique_ptr<Wt::WWidget> WebMainUI::createDisplayOptionsToolbar(void)
{
  auto panel = std::make_unique<Wt::WTemplate>(Wt::WString::tr("breadcrumbs-bar.tpl"));
  auto boardSelector = std::make_unique<Wt::WComboBox>();
  m_boardSelectorRef = boardSelector.get();
  m_boardSelectorRef->changed().connect(this, &WebMainUI::handleBoardSelectionChanged);
  if (! m_dbSession->isLoggedAdmin()) {
    m_boardSelectorRef->addItem(m_menuLabels[MenuExecutiveView]) ;
    m_boardSelectorRef->addItem(m_menuLabels[MenuPlatformStatusAnalytics]) ;
  } else {
    m_boardSelectorRef->addItem(Q_TR("Admin Home"));
  }
  panel->bindWidget("display-view-selection-box", std::move(boardSelector));

  auto showOnlyProblemsSelector = std::make_unique<Wt::WCheckBox>(Q_TR("Show only problems"));
  m_showOnlyProblemsSelectorRef = showOnlyProblemsSelector.get();
  m_showOnlyProblemsSelectorRef->setHidden(true);
  showOnlyProblemsSelector->changed().connect(this, &WebMainUI::handleShowOnlyProblems);
  showOnlyProblemsSelector->setHidden(true);
  panel->bindWidget("display-only-trouble-event-box", std::move(showOnlyProblemsSelector));

  return panel;
}

void WebMainUI::handleShowSettingsView(void)
{
  if (! m_dbSession->isLoggedAdmin()) {
    disableAdminFeatures();
  }
  switchFeaturePanel(m_settingsPageRef);
}

std::unique_ptr<Wt::WWidget> WebMainUI::createNotificationToolbar(void)
{
  auto tpl = std::make_unique<Wt::WTemplate>(Wt::WString::tr("notification.block.tpl"));
  tpl->setToolTip(Q_TR("Manage notification"));

  auto initialMsg = std::make_unique<Wt::WText>("0");
  m_notificationBoxes[ngrt4n::Normal] = initialMsg.get();
  m_notificationBoxes[ngrt4n::Normal]->setStyleClass("btn btn-normal");
  m_notificationBoxes[ngrt4n::Normal]->setHidden(true);
  tpl->bindWidget("normal-count", std::move(initialMsg));

  initialMsg.reset(new Wt::WText("0"));
  m_notificationBoxes[ngrt4n::Minor] = initialMsg.get();
  m_notificationBoxes[ngrt4n::Minor]->setStyleClass("btn btn-minor");
  m_notificationBoxes[ngrt4n::Minor]->setHidden(true);
  tpl->bindWidget("minor-count", std::move(initialMsg));

  initialMsg.reset(new Wt::WText("0"));
  m_notificationBoxes[ngrt4n::Major] = initialMsg.get();
  m_notificationBoxes[ngrt4n::Major]->setStyleClass("btn btn-major");
  m_notificationBoxes[ngrt4n::Major]->setHidden(true);
  tpl->bindWidget("major-count", std::move(initialMsg));

  initialMsg.reset(new Wt::WText("0"));
  m_notificationBoxes[ngrt4n::Critical] = initialMsg.get();
  m_notificationBoxes[ngrt4n::Critical]->setStyleClass("btn btn-critical");
  m_notificationBoxes[ngrt4n::Critical]->setHidden(true);
  tpl->bindWidget("critical-count", std::move(initialMsg));

  initialMsg.reset(new Wt::WText("0"));
  m_notificationBoxes[ngrt4n::Unknown] = initialMsg.get();
  m_notificationBoxes[ngrt4n::Unknown]->setStyleClass("btn btn-unknown");
  m_notificationBoxes[ngrt4n::Unknown]->setHidden(true);
  tpl->bindWidget("unknown-count", std::move(initialMsg));

  tpl->clicked().connect(this, &WebMainUI::handleShowNotificationManager);

  return tpl;
}



void WebMainUI::disableAdminFeatures(void)
{
  wApp->doJavaScript("$('#userMenuSection').hide();"
                     "$('#viewMenuBlock').hide();"
                     "$('#menu-database-settings').hide();"
                     "$('#menu-auth-settings').hide();"
                     "$('#menu-notification-settings').hide();"
                     "$('#menu-license-activation').hide();");
  m_sourceConfigRef->setEnabledInputs(false);
  m_dbPageRef->setDisabled(true);
  m_notifPageRef->setHidden(true);
  m_authPageRef->setHidden(true);
}

void WebMainUI::handleRefresh(void)
{
  CORE_LOG("info", QObject::tr("updating console (operator: %1, session: %2)").arg(m_dbSession->loggedUserName(), wApp->sessionId().c_str()).toStdString());

  m_globalTimer.stop();

  std::map<int, int> appStates = {
    {ngrt4n::Normal, 0},
    {ngrt4n::Minor, 0},
    {ngrt4n::Major, 0},
    {ngrt4n::Critical, 0},
    {ngrt4n::Unknown, 0},
  };

  // clear the notification manager when applicable
  if (m_notificationManager) {
    m_notificationManager->clearAllServicesData();
  }

  int currentView = 1;
  for (auto& currentBoard : m_appBoards) {
    currentBoard->setDbSession(m_dbSession);
    auto loadDsOut = currentBoard->loadDataSources();
    if (loadDsOut.first != ngrt4n::RcSuccess) {
      CORE_LOG("error", loadDsOut.second.toStdString());
      continue;
    }
    currentBoard->updateAllNodesStatus();
    currentBoard->updateMap();
    NodeT currentRootNode = currentBoard->rootNode();
    int overallSeverity = qMin(currentRootNode.sev, static_cast<int>(ngrt4n::Unknown));
    if (overallSeverity != ngrt4n::Normal) {
      ++appStates[overallSeverity];
      if (m_notificationManager) {
        m_notificationManager->updateServiceData(currentRootNode);
      }
    }
    std::string vname = currentRootNode.name.toStdString();
    auto thumb = m_thumbnails.find(vname);
    if (thumb != m_thumbnails.end()) {
      (*thumb)->setStyleClass(currentBoard->thumbCss());
      (*thumb)->setToolTip(currentBoard->tooltip());
      if (! m_dbSession->isLoggedAdmin() && m_platformStatusAnalyticsChartsRef) {
        std::string currentView = m_boardSelectorRef->currentText().toUTF8();
        if (currentView == m_menuLabels[MenuPlatformStatusAnalytics]) {
          PlatformMappedStatusHistoryT platformStatusData;
          m_dbSession->listStatusHistory(platformStatusData, vname, m_platformStatusAnalyticsChartsRef->startTime(), m_platformStatusAnalyticsChartsRef->endTime());
          m_platformStatusAnalyticsChartsRef->updateByView(vname, platformStatusData);
        }
      }
    }
    auto thumbComment = m_thumbnailComments.find(vname);
    if (thumbComment != m_thumbnailComments.end()) {
      (*thumbComment)->setText(currentBoard->thumbMsg());
    }


    ++currentView;
  }

  // Display notifications only on operator console
  if (! m_dbSession->isLoggedAdmin()) {
    for (auto appState: appStates) {
      auto notif = m_notificationBoxes[appState.first];
      notif->setText(QString::number(appState.second).toStdString());
      notif->setHidden(appState.second <= 0);
      notif->setToolTip(QObject::tr("%1 application(s) in %2 state")
                        .arg(QString::number(appState.second), Severity(appState.first).toString())
                        .toStdString());
    }
  }

  CORE_LOG("info",
           QObject::tr("console update completed (operator: %1, session: %2)")
           .arg(m_dbSession->loggedUserName(), wApp->sessionId().c_str())
           .toStdString());

  m_globalTimer.setInterval(std::chrono::milliseconds(1000 * m_settings.updateInterval()));
  m_globalTimer.start();
}



void WebMainUI::handleLaunchEditor(void)
{
  m_adminPageTitleRef->setText(Q_TR("Service Editor"));
  m_adminStackRef->setCurrentWidget(m_webEditorRef);
  m_webEditorRef->refreshDynamicContents();
}


void WebMainUI::handlePreviewFile(const std::string& path, const std::string&)
{
  auto loadResult = loadView(path);
  if (! loadResult.first) {
    showMessage(ngrt4n::OperationFailed, loadResult.second.toStdString());
  } else {
    loadResult.first->setShowOnlyProblemMsgsState(m_showOnlyProblemsSelectorRef->checkState()==Wt::CheckState::Checked);
  }
}


void WebMainUI::handleAuthSettings(void)
{
  m_adminPageTitleRef->setText(Q_TR("Authentication Settings"));
  m_adminStackRef->setCurrentWidget(m_authPageRef);
  m_authPageRef->updateContents();
}


void WebMainUI::handleNotificationSettings(void)
{
  m_adminPageTitleRef->setText(Q_TR("Notification Settings"));
  m_adminStackRef->setCurrentWidget(m_notifPageRef);
  m_notifPageRef->updateContents();
}


void WebMainUI::handleDatabaseSettings(void)
{
  m_adminPageTitleRef->setText(Q_TR("Database Settings"));
  m_adminStackRef->setCurrentWidget(m_dbPageRef);
  m_dbPageRef->updateContents();
}


void WebMainUI::handleUserProfileSettings(void)
{
  m_accountPageRef->resetValidationState(false);
  m_adminStackRef->setCurrentWidget(m_accountPageRef);
  m_adminPageTitleRef->setText(Q_TR("My Account"));
}


void WebMainUI::handleUpdateUserAccount(const DboUserT& userToUpdate)
{
  handleErrcode(m_dbSession->updateUser(userToUpdate).first);
}


void WebMainUI::handleDisplayChangePassword(void)
{
  m_adminStackRef->setCurrentWidget(m_changePasswdPanelRef);
  m_changePasswdPanelRef->reset();
  m_adminPageTitleRef->setText("Change password");
}


void WebMainUI::handleChangePassword(const std::string& login, const std::string& lastpass, const std::string& pass)
{
  handleErrcode(m_dbSession->updatePassword(login, lastpass, pass).first);
}


void WebMainUI::handleReportPeriodChanged(long start, long end)
{
  PlatformMappedStatusHistoryT qosData;
  m_dbSession->listStatusHistory(qosData, "ALL", start, end);
  for (const auto& vname : qosData.keys()) {
    m_platformStatusAnalyticsChartsRef->updateByView(vname, qosData);
  }
}


void WebMainUI::handleDataSourceSettings(void)
{
  m_adminPageTitleRef->setText(Q_TR("Monitoring Sources"));
  m_adminStackRef->setCurrentWidget(m_sourceConfigRef);
  m_sourceConfigRef->updateContents();
}


void WebMainUI::handleBoardSelectionChanged(void)
{
  std::string currentView = m_boardSelectorRef->currentText().toUTF8();
  auto appBoard = m_appBoards.find(currentView.c_str());
  if (appBoard != m_appBoards.end()) {
    showAppBoard(*appBoard);
    m_showOnlyProblemsSelectorRef->setHidden(false);
  } else {
    m_currentAppBoard = nullptr;
    m_showOnlyProblemsSelectorRef->setHidden(true);
    if (m_dbSession->isLoggedAdmin()) {
      switchFeaturePanel(m_settingsPageRef);
    } else {
      if (currentView == m_menuLabels[MenuPlatformStatusAnalytics]) {
        if (! m_platformStatusAnalyticsPageRef) {
          m_platformStatusAnalyticsPageRef = buildSlaAnalyticsPage();
        }
        switchFeaturePanel(m_platformStatusAnalyticsPageRef);
      } else {
        switchFeaturePanel(m_executiveViewPageRef);
      }
    }
  }
}


void WebMainUI::handleShowOnlyProblems(void)
{
  if (m_showOnlyProblemsSelectorRef && m_currentAppBoard) {
    m_currentAppBoard->setShowOnlyProblemMsgsState(m_showOnlyProblemsSelectorRef->checkState()==Wt::CheckState::Checked);
    m_currentAppBoard->refreshMsgConsoleOnProblemStates();
  }
}


std::unique_ptr<Wt::WAnchor> WebMainUI::createLogoLink(void)
{
  auto link = Wt::WLink(PKG_URL.toStdString());
  link.setTarget(Wt::LinkTarget::NewWindow);
  auto image = std::make_unique<Wt::WImage>("images/built-in/logo-mini.png");
  auto result = std::make_unique<Wt::WAnchor>(link, std::move(image));
  return result;
}


void WebMainUI::selectItem4Preview(void)
{
  m_previewInput.updateContentWithViewList(m_dbSession->listViews());
  m_previewInput.show(); // accept action triggers the method handlePreviewFile
}


std::pair<WebDashboard*, QString> WebMainUI::loadView(const std::string& path)
{
  if (path.empty()) {
    return {nullptr, QObject::tr("cannot open empty path")};
  }

  WebDashboard* myboard = nullptr;
  try {
    myboard = new WebDashboard(m_dbSession);
    if (! myboard) {
      return {nullptr, QObject::tr("cannot allocate the dashboard widget")};
    }

    auto initResult = myboard->initialize(path.c_str());
    if (initResult.first != ngrt4n::RcSuccess) {
      CORE_LOG("error", tr("failed initializing %1").arg(path.c_str()).toStdString());
      return {nullptr, initResult.second};
    }

    myboard->buildTree();
    myboard->buildMap();

    QString appName = myboard->rootNode().name;
    auto bfound = m_appBoards.find(appName);
    if (bfound != m_appBoards.end()) {
      m_opsStackRef->removeWidget(*bfound);
      m_appBoards.remove(appName);
    }

    m_appBoards.insert(appName, myboard);
    m_boardSelectorRef->addItem(appName.toStdString());
    if (m_eventFeedLayoutRef) {
      myboard->setEventFeedLayout(m_eventFeedLayoutRef);
    }
    std::unique_ptr<WebDashboard> myBoardUptr;
    myBoardUptr.reset(myboard);
    m_opsStackRef->addWidget(std::move(myBoardUptr));
  } catch (const std::bad_alloc& ex) {
    CORE_LOG("error", tr("dashboard initialization failed with bad_alloc: %1").arg(ex.what()).toStdString());
    showMessage(ngrt4n::OperationFailed, Q_TR("memory overflow"));
    return {nullptr, QObject::tr("memory overflow")};
  }

  showAppBoard(myboard);

  return {myboard, ""};
}

void WebMainUI::scaleMap(double factor)
{
  if (m_currentAppBoard) {
    m_currentAppBoard->mapRef()->scaleMap(factor);
  }
}


void WebMainUI::handleErrcode(int errcode)
{
  if (errcode != ngrt4n::RcSuccess) {
    showMessage(ngrt4n::OperationFailed, Q_TR("Updated failed"));
  } else {
    showMessage(ngrt4n::OperationSucceeded, Q_TR("Updated successfully"));
  }
}



void WebMainUI::handleUserUpdatedCompleted(int errcode)
{
  if (errcode != 0) {
    showMessage(ngrt4n::OperationFailed, "update failed: check log for details");
  } else {
    showMessage(ngrt4n::OperationSucceeded, Q_TR("Updated successfully"));
    m_dbUserManager->resetUserForm();
  }
}

void WebMainUI::handleShowAdminHome(void)
{
  m_adminStackRef->setCurrentWidget(m_adminHomePageRef);
  m_adminPageTitleRef->setText("");
}


void WebMainUI::handleHideInfoBox(Wt::WMouseEvent)
{
  m_infoBoxRef->hide();
}

void WebMainUI::showMessage(int status, const std::string& msg)
{
  std::string logLevel = "debug";
  std::string cssClass = "alert alert-success";
  bool hidden = false;

  switch (status) {
    case ngrt4n::OperationSucceeded:
      cssClass = "alert alert-success";
      logLevel = "info";
      break;
    case ngrt4n::OperationFailed:
      cssClass = "alert alert-warning";
      logLevel = "warn";
      break;
    case ngrt4n::OperationInProgress:
      cssClass = "alert alert-primary";
      logLevel = "error";
      break;
    default:
      m_infoBoxRef->hide();
      hidden = true;
      break;
  }

  CORE_LOG(logLevel, msg);

  m_infoBoxRef->setText(msg);
  m_infoBoxRef->setStyleClass(cssClass);
  m_infoBoxRef->setHidden(hidden);
}


std::shared_ptr<Wt::WDialog> WebMainUI::createAboutDialog(void)
{
  auto dialog = std::make_shared<Wt::WDialog>();
  dialog->setTitleBarEnabled(false);
  dialog->setStyleClass("Wt-dialog");

  auto closeButton = std::make_unique<Wt::WPushButton>(tr("Close").toStdString());
  closeButton->clicked().connect(dialog.get(), &Wt::WDialog::accept);

  auto page =  std::make_unique<Wt::WTemplate>(Wt::WString::tr("about-tpl"));
  page->bindString("version", PKG_VERSION.toStdString());
  page->bindString("codename", REL_NAME.toStdString());
  page->bindString("release-date", QString::number(BUILD_TIME).toStdString());
  page->bindWidget("close-button", std::move(closeButton));
  page->bindWidget("footer", ngrt4n::footer());
  dialog->contents()->addWidget(std::move(page));

  return dialog;
}



std::unique_ptr<Wt::WTemplate> WebMainUI::createThumb(std::string title, std::string msg, std::string imgURL, std::string imgLink)
{
  auto page = std::make_unique<Wt::WTemplate>(Wt::WString::tr("dashboard-thumbnail.tpl"));
  page->setStyleClass("btn btn-unknown");
  page->bindWidget("thumb-titlebar", std::make_unique<Wt::WLabel>(title));
  auto img = std::make_unique<Wt::WImage>(imgURL);
  img->setImageLink (imgLink);
  page->bindWidget("thumb-image", std::move(img));
  return page;
}

void WebMainUI::setInternalPath(const std::string& path)
{
  wApp->setInternalPath(path);
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


void WebMainUI::startDashboardUpdate(void)
{
  auto newTimer(new Wt::WTimer());
  newTimer->setInterval(std::chrono::milliseconds(2000));
  newTimer->start();
  newTimer->timeout().connect(std::bind([=]() {
    newTimer->stop();
    delete newTimer;
    handleRefresh();
  }));
}


void WebMainUI::handleAuthSystemChanged(int authSystem)
{
  switch (authSystem) {
    case WebBaseSettings::LDAP:
      m_menuLinks[MenuLdapUsers]->setDisabled(false);
      wApp->doJavaScript("$('#menu-ldap-users').prop('disabled', false);");
      break;
    default:
      m_dbSession->deleteAuthSystemUsers(WebBaseSettings::LDAP);
      wApp->doJavaScript("$('#menu-ldap-users').prop('disabled', true);");
      break;
  }
}

void WebMainUI::handleLdapUsersMenu(void)
{
  if (m_authPageRef->getAuthenticationMode() != WebBaseSettings::LDAP) {
    showMessage(ngrt4n::OperationFailed, Q_TR("Denied, please enable LDAP authentication first"));
  } else {
    m_adminStackRef->setCurrentWidget(m_ldapUserManagerRef);
    if (m_ldapUserManagerRef->updateUserList() <= 0) {
      showMessage(ngrt4n::OperationFailed, m_ldapUserManagerRef->lastError());
    }
    m_adminPageTitleRef->setText(Q_TR("LDAP Users"));
  }
}


void WebMainUI::handleManageBuiltinUsers(void)
{
  m_adminStackRef->setCurrentWidget(m_dbUserManager->dbUserListWidgetRef());
  m_dbUserManager->updateDbUsers();
  m_adminPageTitleRef->setText(Q_TR("Manage Users"));
}


void WebMainUI::handleNewUser(void)
{
  m_dbUserManager->userFormRef()->reset();
  m_adminStackRef->setCurrentWidget(m_dbUserManager->userFormRef());
  m_adminPageTitleRef->setText(Q_TR("Create New User"));
}


void  WebMainUI::handleViewAclMenu(void)
{
  m_adminStackRef->setCurrentWidget(m_aclPageRef);
  m_aclPageRef->updateFieldStates();
  m_adminPageTitleRef->setText(Q_TR("Manage Operations Profiles"));
}


void WebMainUI::handleDeleteView(const std::string& viewName)
{
  auto loadedDashboardItem = m_appBoards.find(viewName.c_str());
  if (loadedDashboardItem != m_appBoards.end()) {
    WebDashboard* dashboard = *loadedDashboardItem;
    m_opsStackRef->removeWidget(dashboard);
    delete (*loadedDashboardItem);
    m_appBoards.remove(viewName.c_str());
  }
}


void WebMainUI::handleUserEnableStatusChanged(int status, std::string data)
{
  switch (status) {
    case LdapUserManager::EnableAuthSuccess:
      showMessage(ngrt4n::OperationSucceeded, Q_TR("LDAP authentication enabled for user ") + data);
      break;
    case LdapUserManager::DisableAuthSuccess:
      showMessage(ngrt4n::OperationSucceeded, Q_TR("LDAP authentication disabled for user ") + data);
      break;
    case LdapUserManager::GenericError:
      showMessage(ngrt4n::OperationFailed, data);
      break;
    default:
      break;
  }
}


void WebMainUI::showAppBoard(WebDashboard* appBoard)
{
  if (appBoard) {
    switchFeaturePanel(appBoard);
    // appBoard->doJavascriptAutoResize();
    m_boardSelectorRef->setCurrentIndex(m_boardSelectorRef->findText(appBoard->rootNode().name.toStdString()));
    m_showOnlyProblemsSelectorRef->setHidden(false);
    m_currentAppBoard = appBoard;
  }
}

void WebMainUI::switchFeaturePanel(Wt::WWidget* item)
{
  if (item) {
    m_opsStackRef->setCurrentWidget(item);
  }
}

void WebMainUI::handleDashboardSelected(std::string viewName)
{
  auto loadedDashboardItem = m_appBoards.find(viewName.c_str());
  if (loadedDashboardItem != m_appBoards.end()) {
    showAppBoard(*loadedDashboardItem);
  }
}



void WebMainUI::saveViewInfoIntoDatabase(const CoreDataT& cdata, const QString& path)
{
  DboView view;
  view.name = cdata.bpnodes[ngrt4n::ROOT_ID].name.toStdString();
  view.service_count = cdata.bpnodes.size() + cdata.cnodes.size();
  view.path = path.toStdString();

  auto addViewOut = m_dbSession->addView(view);
  if (addViewOut.first != ngrt4n::RcSuccess){
    showMessage(ngrt4n::OperationFailed, addViewOut.second.toStdString());
  } else {
    QString appName(view.name.c_str());
    QString appOpsViewConfig(view.path.c_str());
    QString appCountLabel = QString::number(view.service_count);
    std::string msg = QObject::tr("Added: %1 (%2 Services) - Path: %3"
                                  ).arg(appName, appCountLabel, appOpsViewConfig).toStdString();
    showMessage(ngrt4n::OperationSucceeded, msg);
    CORE_LOG("info", msg);
  }
}
