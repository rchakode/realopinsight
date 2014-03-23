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

#include "WebPreferences.hpp"
#include "AuthManager.hpp"
#include "WebMainUI.hpp"
#include "ViewMgnt.hpp"
#include "utilsClient.hpp"
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

WebMainUI::WebMainUI(AuthManager* authManager)
  : Wt::WContainerWidget(),
    m_mainWidget(new Wt::WContainerWidget(this)),
    m_settings (new Settings()),
    m_authManager(authManager),
    m_dbSession(m_authManager->session()),
    m_preferences(new WebPreferences()),
    m_dashtabs(new Wt::WTabWidget()),
    m_fileUploadDialog(createDialog(tr("Select file to preview | %1").arg(APP_NAME).toStdString())),
    m_confdir("/var/lib/realopinsight/config"),
    m_showSettingTab(true),
    m_currentDashboard(NULL),
    m_terminateSession(this)
{
  m_preferences->setEnabledInputs(false);
  m_preferences->errorOccurred().connect(std::bind(&WebMainUI::showMessage, this,
                                                   std::placeholders::_1, "alert alert-warning"));
  // create directory for view thumbnails and misc runtime data
  // true means clean the directory
  createDirectory(wApp->docRoot().append("/run"), true);

  // Now start creating the view
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
  // Delete all
  //  m_contents(NULL),
  //  m_navbar(NULL),
  //  m_mgntMenu(NULL),
  //  m_profileMenu(NULL),
  delete m_preferences;
  delete m_fileUploadDialog;
  delete m_navbar;
  delete m_contents;
  delete m_mainWidget;
  LOG("debug", "Session closed");
}


void WebMainUI::addEvents(void)
{
  wApp->globalKeyPressed().connect(std::bind([=](const Wt::WKeyEvent& event){}, std::placeholders::_1));
  wApp->internalPathChanged().connect(this, &WebMainUI::handleInternalPath);
  connect(m_settings, SIGNAL(timerIntervalChanged(qint32)), this, SLOT(resetTimer(qint32)));
}

void WebMainUI::showUserHome(void)
{
  std::string homeTabTitle = "Home";
  if (m_dbSession->loggedUser().role == User::AdmRole) {
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
  
  if (m_dbSession->loggedUser().role == User::OpRole) {
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
  
  // Create a container for stacked contents
  m_contents = new Wt::WStackedWidget(m_mainWidget);
  m_contents->setId("stackcontentarea");
  m_contents->addWidget(m_dashtabs);
}


void WebMainUI::setupProfileMenus(void)
{
  Wt::WMenu* profileMenu = new Wt::WMenu();
  m_navbar->addMenu(profileMenu, Wt::AlignRight);
  
  if (m_dbSession->loggedUser().role == User::OpRole) {

    Wt::WTemplate* notificationBlock = new Wt::WTemplate(Wt::WString::tr("notification.block.tpl"));

    m_notificationBoxes[ngrt4n::Minor] = new Wt::WText("0");
    m_notificationBoxes[ngrt4n::Minor]->setStyleClass("badge severity-minor");
    m_notificationBoxes[ngrt4n::Minor]->setHidden(true);
    notificationBlock->bindWidget("minor-count", m_notificationBoxes[ngrt4n::Minor]);

    m_notificationBoxes[ngrt4n::Major] = new Wt::WText("0");
    m_notificationBoxes[ngrt4n::Major]->setStyleClass("badge severity-major");
    m_notificationBoxes[ngrt4n::Major]->setHidden(true);
    notificationBlock->bindWidget("major-count", m_notificationBoxes[ngrt4n::Major]);

    m_notificationBoxes[ngrt4n::Critical] = new Wt::WText("0");
    m_notificationBoxes[ngrt4n::Critical]->setStyleClass("badge severity-critical");
    m_notificationBoxes[ngrt4n::Critical]->setHidden(true);
    notificationBlock->bindWidget("critical-count", m_notificationBoxes[ngrt4n::Critical]);

    m_notificationBoxes[ngrt4n::Unknown] = new Wt::WText("0");
    m_notificationBoxes[ngrt4n::Unknown]->setStyleClass("badge severity-unknown");
    m_notificationBoxes[ngrt4n::Unknown]->setHidden(true);
    notificationBlock->bindWidget("unknown-count", m_notificationBoxes[ngrt4n::Unknown]);

    m_navbar->addWidget(notificationBlock, Wt::AlignRight);
  }
  
  Wt::WMenuItem* profileMenuItem
      = new Wt::WMenuItem(tr("Signed in as %1").arg(m_dbSession->loggedUser().username.c_str()).toStdString());
  Wt::WPopupMenu* profilePopupMenu = new Wt::WPopupMenu();
  profileMenuItem->setMenu(profilePopupMenu);
  profileMenu->addItem(profileMenuItem);
  
  Wt::WMenuItem* curItem = NULL;
  if (m_dbSession->loggedUser().role == User::OpRole) {
    curItem = profilePopupMenu->addItem(tr("Show Account & Settings").toStdString());
    curItem->triggered().connect(std::bind([=]() {
      if (m_showSettingTab) {
        if (m_dashtabs->count() > 1) {
          m_dashtabs->setTabHidden(0, false);
          m_dashtabs->setCurrentIndex(0);
          curItem->setText(tr("Hide Account & Settings").toStdString());
          wApp->doJavaScript("$('#userMenuBlock').hide(); $('#viewMenuBlock').hide();");
        }
      } else {
        if (m_dashtabs->count() > 1) {
          m_dashtabs->setTabHidden(0, true);
          m_dashtabs->setCurrentIndex(1);
          curItem->setText(tr("Show Account & Settings").toStdString());
        }
      }
      m_showSettingTab = ! m_showSettingTab;
    }));
  }

  curItem = profilePopupMenu->addItem(tr("Help").toStdString());
  curItem->setLink(Wt::WLink(Wt::WLink::Url, GET_HELP_URL));
  curItem->setLinkTarget(Wt::TargetNewWindow);

  profilePopupMenu->addItem("About")
      ->triggered().connect(std::bind([=](){m_aboutDialog->show();}));
}

void WebMainUI::setupMenus(void)
{
  setupProfileMenus();
  
  //FIXME: add this after the first view loaded
  Wt::WText* text = ngrt4n::createFontAwesomeTextButton("fa fa-refresh",
                                                        "Refresh the console map",
                                                        m_mainWidget);
  text->clicked().connect(this, &WebMainUI::handleRefresh);
  m_navbar->addWidget(text);
  
  text = ngrt4n::createFontAwesomeTextButton("icon-zoom-in",
                                             "Zoom the console map in",
                                             m_mainWidget);
  text->clicked().connect(std::bind(&WebMainUI::scaleMap, this, ngrt4n::SCALIN_FACTOR));
  m_navbar->addWidget(text);
  
  text = ngrt4n::createFontAwesomeTextButton("icon-zoom-out",
                                             "Zoom the console map out",
                                             m_mainWidget);
  text->clicked().connect(std::bind(&WebMainUI::scaleMap, this, ngrt4n::SCALOUT_FACTOR));
  m_navbar->addWidget(text);
}

void WebMainUI::resetTimer(void)
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
  m_mainWidget->disable();
  
  std::map<int, int> problemTypeCount;
  problemTypeCount[ngrt4n::Minor] = 0;
  problemTypeCount[ngrt4n::Major] = 0;
  problemTypeCount[ngrt4n::Critical] = 0;
  problemTypeCount[ngrt4n::Unknown] = 0;

  for (auto& dash : m_dashboards) {
    dash.second->runMonitor();
    dash.second->updateMap();
    dash.second->updateThumbnail();
    int platformSeverity = dash.second->rootNode().severity;
    if (platformSeverity != ngrt4n::Normal) {
      ++problemTypeCount[platformSeverity];
    }
    m_dashTabIndexes[dash.second->rootNode().name]->setStyleClass( ngrt4n::severityCssClass(platformSeverity) );
  }

  for(auto ptype: problemTypeCount) {
    m_notificationBoxes[ptype.first]->setText(QString::number(ptype.second).toStdString());
    if (ptype.second > 0) {
      m_notificationBoxes[ptype.first]->setHidden(false);
    } else {
      m_notificationBoxes[ptype.first]->setHidden(true);
    }
  }

  updateEventFeeds();

  m_timer.start();
  m_mainWidget->enable();
}

Wt::WAnchor* WebMainUI::createLogoLink(void)
{
  Wt::WAnchor* anchor = new Wt::WAnchor(Wt::WLink("http://realopinsight.com/"),
                                        new Wt::WImage("/images/built-in/logo-mini.png"));
  anchor->setTarget(Wt::TargetNewWindow);
  anchor->setMargin(10, Wt::Right);
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
  m_uploader->setFileTextSize(ngrt4n::MAX_FILE_UPLOAD);
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
    showMessage(tr("File too large.").toStdString(), "alert alert-warning");
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
            showMessage(msg, "alert alert-warning");
          } else {

            std::string filename = m_uploader->clientFileName().toUTF8();
            QString dest = tr("%1/%2").arg(m_confdir.c_str(), filename.c_str());
            QFile file(tmpFileName);
            file.copy(dest);
            file.remove();

            View view;
            view.name = cdata.bpnodes[ngrt4n::ROOT_ID].name.toStdString();
            view.service_count = cdata.bpnodes.size() + cdata.cnodes.size();
            view.path = dest.toStdString();
            if (m_dbSession->addView(view) != 0){
              showMessage(m_dbSession->lastError(), "alert alert-warning");
            } else {
              QString msg = tr("View added. "
                               " Name: %1\n - "
                               " Services: %2 -"
                               " Path: %3").arg(view.name.c_str(),
                                                QString::number(view.service_count),
                                                view.path.c_str());
              showMessage(msg.toStdString(), "alert alert-success");
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
        showMessage(tr("No file selected").toStdString(), "alert alert-warning");
      }
      break;
    default:
      break;
  }
}

void WebMainUI::loadView(const std::string& path, WebDashboard*& dashboard)
{
  dashboard = new WebDashboard(path.c_str(), m_eventFeedLayout);
  dashboard->initialize(m_preferences);
  if (! dashboard->errorState()) {
    QString platformName = dashboard->rootNode().name;
    std::pair<DashboardListT::iterator, bool> result;
    result = m_dashboards.insert(std::pair<QString, WebDashboard*>(platformName, dashboard));
    if (result.second) {
      Wt::WMenuItem* tab = m_dashtabs->addTab(dashboard->get(), platformName.toStdString());
      tab->triggered().connect(std::bind([=]() {
        m_currentDashboard = dashboard;
      }));
      m_dashTabIndexes.insert(std::pair<QString, Wt::WMenuItem*>(platformName, tab));
    } else {
      delete dashboard;
      dashboard = NULL;
      showMessage(tr("This platform or a platfom "
                     "with the same name is already loaded").toStdString(),"alert alert-warning");
    }
  } else {
    showMessage(dashboard->lastError().toStdString(),"alert alert-warning");
  }
}

void WebMainUI::scaleMap(double factor)
{
  if (m_currentDashboard) {
    m_currentDashboard->map()->scaleMap(factor);
  }
}

Wt::WWidget* WebMainUI::createSettingPage(void)
{
  m_infoBox = new Wt::WText(m_mainWidget);
  m_infoBox->hide();
  m_infoBox->clicked().connect(std::bind([=](){m_infoBox->hide();}));

  m_mgntContents = new Wt::WStackedWidget(m_mainWidget);

  Wt::WTemplate* settingPageTpl = new Wt::WTemplate(Wt::WString::tr("admin-home.tpl"));
  settingPageTpl->bindWidget("title", m_adminPanelTitle = new Wt::WText(m_mainWidget));
  settingPageTpl->bindWidget("contents", m_mgntContents);
  settingPageTpl->bindWidget("info-box", m_infoBox);

  Wt::WAnchor* link = NULL;
  if (m_dbSession->loggedUser().role == User::AdmRole) {
    // Start menu
    link = new Wt::WAnchor("#", "Welcome", m_mainWidget);
    Wt::WWidget* getStartPage = new Wt::WTemplate(Wt::WString::tr("getting-started.tpl"));
    m_mgntContents->addWidget(getStartPage);
    link->clicked().connect(std::bind([=](){
      m_mgntContents->setCurrentWidget(getStartPage);
      m_adminPanelTitle->setText("Getting Started in 3 Simple Steps !");
    }));
    settingPageTpl->bindWidget("menu-get-started", link);

    // view menus
    link = new Wt::WAnchor("#", "Import", m_mainWidget);
    link->clicked().connect(this, &WebMainUI::openFileUploadDialog);
    settingPageTpl->bindWidget("menu-import", link);

    link = new Wt::WAnchor("#", "Preview", m_mainWidget);
    link->clicked().connect(this, &WebMainUI::selectFileToOpen);
    settingPageTpl->bindWidget("menu-preview", link);

    // Create view management form
    m_viewAccessPermissionForm = new ViewAssignmentUI(m_dbSession, m_mainWidget);
    m_viewAccessPermissionForm->updateCompleted().connect(std::bind([=](int retCode, std::string msg) {
      if (retCode != 0) {
        showMessage(msg, "alert alert-warning");
      } else {
        showMessage(msg, "alert alert-success");
      }
    }, std::placeholders::_1, std::placeholders::_2));
    m_mgntContents->addWidget(m_viewAccessPermissionForm);
    link = new Wt::WAnchor("#", "All Views and Access Permissions", m_mainWidget);
    link->clicked().connect(std::bind([=](){
      m_mgntContents->setCurrentWidget(m_viewAccessPermissionForm);
      m_viewAccessPermissionForm->resetModelData();
      m_adminPanelTitle->setText("Manage Views and Access Permissions");
    }));
    settingPageTpl->bindWidget("menu-all-views", link);

    // User menus
    m_userMgntUI = new UserMngtUI(m_dbSession);
    m_userMgntUI->updateCompleted().connect(std::bind([=](int retCode) {
      if (retCode != 0) {
        showMessage(m_dbSession->lastError(), "alert alert-warning");
      } else {
        showMessage("Successul updated", "alert alert-success");
        m_userMgntUI->resetUserForm();
      }
    }, std::placeholders::_1));
    m_mgntContents->addWidget(m_userMgntUI->userForm());
    link = new Wt::WAnchor("#", "New User", m_mainWidget);
    link->clicked().connect(std::bind([=](){
      m_userMgntUI->userForm()->reset();
      m_mgntContents->setCurrentWidget(m_userMgntUI->userForm());
      m_adminPanelTitle->setText("Create New User");
    }));
    settingPageTpl->bindWidget("menu-new-user", link);

    link = new Wt::WAnchor("#", "All Users", m_mainWidget);
    m_mgntContents->addWidget(m_userMgntUI->userListWidget());
    link->clicked().connect(std::bind([=]() {
      m_mgntContents->setCurrentWidget(m_userMgntUI->userListWidget());
      m_userMgntUI->updateUserList();
      m_adminPanelTitle->setText("Manage Users");
    }));
    settingPageTpl->bindWidget("menu-all-users", link);
    m_preferences->setEnabledInputs(true);
  } else {
    wApp->doJavaScript("$('#userMenuBlock').hide(); $('#viewMenuBlock').hide();");
    settingPageTpl->bindEmpty("menu-get-started");
    settingPageTpl->bindEmpty("menu-import");
    settingPageTpl->bindEmpty("menu-preview");
    settingPageTpl->bindEmpty("menu-all-views");
    settingPageTpl->bindEmpty("menu-new-user");
    settingPageTpl->bindEmpty("menu-all-users");
  }

  // setting menus
  m_mgntContents->addWidget(m_preferences);
  link = new Wt::WAnchor("#", "Monitoring Settings", m_mainWidget);
  link->clicked().connect(std::bind([=](){
    m_adminPanelTitle->setText("Monitoring Settings");
    m_mgntContents->setCurrentWidget(m_preferences);
  }));
  settingPageTpl->bindWidget("menu-monitoring-setting", link);

  m_mgntContents->addWidget(m_userAccountForm);
  link = new Wt::WAnchor("#", "My Account", m_mainWidget);
  link->clicked().connect(std::bind([=](){
    m_mgntContents->setCurrentWidget(m_userAccountForm);
    m_adminPanelTitle->setText("My Account");
  }));
  settingPageTpl->bindWidget("menu-my-account", link);

  m_mgntContents->addWidget(m_changePasswordPanel);
  link = new Wt::WAnchor("#", "Change Password", m_mainWidget);
  link->clicked().connect(std::bind([=](){
    m_mgntContents->setCurrentWidget(m_changePasswordPanel);
    m_adminPanelTitle->setText("Change password");
  }));
  settingPageTpl->bindWidget("menu-change-password", link);

  return settingPageTpl;
}


void WebMainUI::createAccountPanel(void)
{
  bool changedPassword(false);
  bool isUserForm(true);
  m_userAccountForm = new UserFormView(&(m_dbSession->loggedUser()), changedPassword, isUserForm);
  m_userAccountForm->validated().connect(std::bind([=](User userToUpdate) {
    int ret = m_dbSession->updateUser(userToUpdate);
    if (ret != 0) {
      showMessage("Update failed, see details in log.", "alert alert-warning");
    } else {
      showMessage("Update completed.", "alert alert-success");
    }}, std::placeholders::_1));
}

void WebMainUI::createPasswordPanel(void)
{
  bool changedPassword(true);
  bool userForm(true);
  m_changePasswordPanel = new UserFormView(&(m_dbSession->loggedUser()),
                                           changedPassword,
                                           userForm);
  //m_changePasswordPanel->closeTriggered().connect(std::bind([=](){m_changePasswordPanel->accept();}));
  m_changePasswordPanel->changePasswordTriggered().connect(std::bind([=](const std::string& login,
                                                                     const std::string& lastpass,
                                                                     const std::string& pass) {
    int ret = m_dbSession->updatePassword(login, lastpass, pass);
    if (ret != 0) {
      showMessage("Change password failed, see details in log.", "alert alert-warning");
    } else {
      showMessage("Successul password updated.", "alert alert-success");
    }
  }, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}


void WebMainUI::handleInternalPath(void)
{
  std::string path = Wt::WApplication::instance()->internalPath();
  if (path == ngrt4n::LINK_LOAD) {
    selectFileToOpen();
    setInternalPath("");
  } else if (path == ngrt4n::LINK_IMPORT) {
    openFileUploadDialog();
    setInternalPath("");
  } else if (path == ngrt4n::LINK_ADMIN_HOME) {
    showUserHome();
  } else if (path == ngrt4n::LINK_LOGIN) {
    wApp->redirect(ngrt4n::LINK_LOGIN);
  } else {
    showMessage(tr("Sorry, the request resource "
                   "is not available or has been removed").toStdString(),
                "alert alert-warning");
  }
}


Wt::WComboBox* WebMainUI::createViewSelector(void)
{
  ViewListT views = m_dbSession->viewList();
  
  Wt::WComboBox* viewSelector = new Wt::WComboBox();
  viewSelector->setMargin(10, Wt::Right);
  
  Wt::WStandardItemModel* viewSelectorModel = new Wt::WStandardItemModel(m_mainWidget);
  Wt::WStandardItem *item = new Wt::WStandardItem();
  item->setText("-- Select a description file --");
  viewSelectorModel->appendRow(item);
  
  Q_FOREACH(const View& view, views) {
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


void WebMainUI::showMessage(const std::string& msg, std::string status)
{
  m_infoBox->setText(msg);
  m_infoBox->setStyleClass(status);
  m_infoBox->show();
}

void WebMainUI::createAboutDialog(void)
{
  m_aboutDialog = new Wt::WDialog(m_mainWidget);
  m_aboutDialog->setTitleBarEnabled(false);
  m_aboutDialog->setStyleClass("Wt-dialog");
  
  Wt::WPushButton* closeButton(new Wt::WPushButton(tr("Close").toStdString()));
  closeButton->clicked().connect(std::bind([=](){m_aboutDialog->accept();}));
  
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("about-tpl"), m_aboutDialog->contents());
  
  tpl->bindString("software", APP_NAME.toStdString());
  tpl->bindString("version", PKG_VERSION.toStdString());
  tpl->bindString("corelib-version", ngrt4n::libVersion().toStdString());
  tpl->bindString("codename", REL_NAME.toStdString());
  tpl->bindString("release-id", REL_INFO.toStdString());
  tpl->bindString("release-year", REL_YEAR.toStdString());
  tpl->bindString("package-url", PKG_URL.toStdString());
  tpl->bindString("bug-report-email", REPORT_BUG.toStdString());
  tpl->bindWidget("close-button", closeButton);
}


void WebMainUI::initOperatorDashboard(void)
{
  Wt::WContainerWidget* thumbs = new Wt::WContainerWidget(m_mainWidget);
  Wt::WGridLayout* thumbLayout = new Wt::WGridLayout(thumbs);
  
  
  Wt::WContainerWidget* eventFeeds = new Wt::WContainerWidget(m_mainWidget);
  m_eventFeedLayout = std::make_shared<Wt::WVBoxLayout>(eventFeeds);
  
  Wt::WTemplate* m_operatorHomeTpl = new Wt::WTemplate(Wt::WString::tr("operator-home.tpl"));
  m_operatorHomeTpl->bindWidget("info-box", m_infoBox);
  m_operatorHomeTpl->bindWidget("contents", thumbs);
  m_operatorHomeTpl->bindWidget("event-feeds", eventFeeds);
  m_dashtabs->addTab(m_operatorHomeTpl, tr("Operations Console").toStdString());

  m_dbSession->updateViewList(m_dbSession->loggedUser().username);
  m_assignedDashboardCount = m_dbSession->viewList().size();
  int thumbIndex = 0;
  for (const auto& view: m_dbSession->viewList()) {
    WebDashboard* dashboard;
    loadView(view.path, dashboard);
    if (dashboard) {
      thumbLayout->addWidget(thumbnail(dashboard), thumbIndex / 4, thumbIndex % 4);
      ++thumbIndex;
    }
  }
  startDashbaordUpdate();
}


Wt::WTemplate* WebMainUI::thumbnail(WebDashboard* dashboard)
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
    showMessage(errMsg.toStdString(), "alert alert-warning");
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
