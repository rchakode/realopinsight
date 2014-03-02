/*
 * MainWebWindow.cpp
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

#include "WebPreferences.hpp"
#include "AuthManager.hpp"
#include "ViewMgnt.hpp"
#include "WebMainUI.hpp"
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
  "var windowHeight = $(window).height();" \
  "$(\"#ngrt4n-content-pane\").height(windowHeight - top);" \
  "$(\"#ngrt4n-side-pane\").height(windowHeight - top);"

WebMainUI::WebMainUI(AuthManager* authManager)
  : Wt::WContainerWidget(),
    m_mainWidget(new Wt::WContainerWidget(this)),
    m_settings (new Settings()),
    m_authManager(authManager),
    m_dbSession(m_authManager->session()),
    m_preferenceDialog(new WebPreferences(m_dbSession->loggedUser().role)),
    m_dashtabs(new Wt::WTabWidget()),
    m_fileUploadDialog(createDialog(tr("Select a file").toStdString())),
    m_confdir("/var/lib/realopinsight/config"),
    m_terminateSession(this),
    m_showSettingTab(true)
{
  createDirectory(wApp->docRoot().append("/tmp"), true); //true means clean the directory
  createMainUI();
  createViewAssignmentDialog();
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
  delete m_preferenceDialog;
  delete m_fileUploadDialog;
  delete m_navbar;
  delete m_contents;
  delete m_mainWidget;
  LOG("notice", "Session closed");
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
    m_dashtabs->setTabHidden(0, true);
    initOperatorDashboard();
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


void WebMainUI::setupManagementMenus(void)
{
  m_mgntContents = new Wt::WStackedWidget(m_mainWidget);
  m_mgntTopMenu = new Wt::WMenu(m_mgntContents, m_mainWidget);
  
  Wt::WMenuItem* menuItem = NULL;
  if (m_dbSession->loggedUser().role == User::AdmRole) {
    // Start menu
    menuItem = m_mgntTopMenu->addItem("Get Started",
                                      new Wt::WTemplate(Wt::WString::tr("getting-started.tpl")));
    menuItem->triggered().connect(std::bind([=](){
      m_adminPanelTitle->setText("Getting Started in 3 Simple Steps !");
    }));
    
    // view menus
    m_mgntTopMenu->addSectionHeader("Views");
    m_mgntTopMenu->addItem("Import")
        ->setLink(Wt::WLink(Wt::WLink::InternalPath, ngrt4n::LINK_IMPORT));
    m_mgntTopMenu->addItem("Preview")
        ->setLink(Wt::WLink(Wt::WLink::InternalPath, ngrt4n::LINK_LOAD));
    m_mgntTopMenu->addItem("Manage Views", m_viewAssignmentDialog->contents())
        ->triggered().connect(std::bind([=](){
      m_viewAssignmentDialog->resetModelData();
      m_adminPanelTitle->setText("Manage Views");
    }));
    
    // User menus
    m_userMgntUI = new UserMngtUI(m_dbSession);
    m_mgntTopMenu->addSectionHeader("Users");
    menuItem = m_mgntTopMenu->addItem("Add user", m_userMgntUI->userForm());
    menuItem->triggered().connect(std::bind([=](){
      m_adminPanelTitle->setText("Create new user");
    }));
    
    menuItem = m_mgntTopMenu->addItem("Manage Users", m_userMgntUI->userListWidget());
    menuItem->triggered().connect(std::bind([=](){
      m_userMgntUI->updateUserList();
      m_adminPanelTitle->setText("Manage Users");
    }));
  }
  
  // setting menus
  m_mgntTopMenu->addSectionHeader("Account & Settings");
  menuItem = m_mgntTopMenu->addItem("Monitoring Settings", m_preferenceDialog->getWidget());
  menuItem->triggered().connect(std::bind([=](){
    m_adminPanelTitle->setText("Update Monitoring Settings");
  }));
  
  menuItem = m_mgntTopMenu->addItem("My Account", m_accountPanel->contents());
  menuItem->triggered().connect(std::bind([=](){
    m_adminPanelTitle->setText("My Account");
  }));
  
  menuItem = m_mgntTopMenu->addItem("Change Password", m_changePasswordPanel->contents());
  menuItem->triggered().connect(std::bind([=](){
    m_adminPanelTitle->setText("Change password");
  }));
}

void WebMainUI::setupProfileMenus(void)
{
  m_profileMenu = new Wt::WMenu();
  m_navbar->addMenu(m_profileMenu, Wt::AlignRight);
  
  if (m_dbSession->loggedUser().role == User::OpRole) {
    Wt::WTemplate* notificationIcon = new Wt::WTemplate(Wt::WString::tr("notification.block.tpl"));
    notificationIcon->bindString("problem-count", "1");
    m_navbar->addWidget(notificationIcon, Wt::AlignRight);
  }
  
  m_mainProfileMenuItem = new Wt::WMenuItem("Profile");
  m_mainProfileMenuItem->setText(tr("Signed in as %1")
                                 .arg(m_dbSession->loggedUser().username.c_str())
                                 .toStdString());
  
  Wt::WPopupMenu* profilePopupMenu = new Wt::WPopupMenu();
  m_mainProfileMenuItem->setMenu(profilePopupMenu);
  m_profileMenu->addItem(m_mainProfileMenuItem);
  
  Wt::WMenuItem* curItem = NULL;
  curItem = profilePopupMenu->addItem(tr("Show Account & Settings").toStdString());
  curItem->triggered().connect(std::bind([=]() {
    if (m_showSettingTab) {
      if (m_dashtabs->count() > 1) {
        m_dashtabs->setTabHidden(0, false);
        m_dashtabs->setCurrentIndex(0);
        curItem->setText(tr("Hide Account & Settings").toStdString());
      }
    } else {
      if (m_dashtabs->count()>1) {
        m_dashtabs->setTabHidden(0, true);
        m_dashtabs->setCurrentIndex(1);
        curItem->setText(tr("Show Account & Settings").toStdString());
      }
    }
    m_showSettingTab = ! m_showSettingTab;
  }));

  curItem = profilePopupMenu->addItem(tr("Help").toStdString());
  curItem->setLink(Wt::WLink(Wt::WLink::Url, GET_HELP_URL));
  curItem->setLinkTarget(Wt::TargetNewWindow);

  profilePopupMenu->addItem("About")
      ->triggered().connect(std::bind([=](){m_aboutDialog->show();}));
}

void WebMainUI::setupMenus(void)
{
  setupManagementMenus();
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
  
  for(auto& dash : m_dashboards) {
    dash.second->runMonitor();
    dash.second->updateMap();
    dash.second->updateThumbnail();
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
  m_fileUploadDialog->setWindowTitle(tr("Select a file").toStdString());
  Wt::WContainerWidget* container(new Wt::WContainerWidget(m_fileUploadDialog->contents()));
  container->clear();
  
  container->setMargin(10, Wt::All);
  container->addWidget(createViewSelector());
  
  // Provide a button to close the window
  Wt::WPushButton* finish(new Wt::WPushButton(tr("Finish").toStdString(), container));
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
    showMessage(tr("File is too large.").toStdString(), "alert alert-warning");
  }));
  m_fileUploadDialog->show();
}

void WebMainUI::finishFileDialog(int action)
{
  switch(action) {
    case IMPORT:
      if (! m_uploader->empty()) {
        if (createDirectory(m_confdir, false)) { // false means don't clean the directory
          LOG("notice", "Parsing the input file");
          QString tmpFileName(m_uploader->spoolFileName().c_str());
          CoreDataT cdata;

          Parser parser(tmpFileName ,&cdata);
          connect(&parser, SIGNAL(errorOccurred(QString)), this, SLOT(handleLibError(QString)));

          if (! parser.process(false)) {
            std::string msg = tr("Invalid configuration file").toStdString();
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
                               " Number of services: %2 -"
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
      if (! m_selectFile.empty()) {
        int tabIndex;
        WebDashboard* dashbord;
        loadView(m_selectFile, dashbord, tabIndex);
        m_selectFile.clear();
      } else {
        showMessage(tr("No file selected").toStdString(), "alert alert-warning");
      }
      break;
    default:
      break;
  }
}

void WebMainUI::loadView(const std::string& path, WebDashboard*& dashboard, int& tabIndex)
{
  
  tabIndex = -1;
  
  dashboard = new WebDashboard(path.c_str());
  dashboard->initialize(m_preferenceDialog);
  connect(dashboard, SIGNAL(errorOccurred(QString)), this, SLOT(handleLibError(QString)));
  
  if (! dashboard->errorState()) {
    std::string platform = dashboard->rootNode().name.toStdString();
    std::pair<DashboardListT::iterator, bool> result;
    result = m_dashboards.insert(std::pair<std::string, WebDashboard*>(platform, dashboard));
    if (result.second) {
      Wt::WMenuItem* tab = m_dashtabs->addTab(dashboard->get(), platform, Wt::WTabWidget::LazyLoading);
      tab->triggered().connect(std::bind([=]() {
        m_currentDashboard = dashboard;
      }));
      tabIndex = m_dashtabs->count() - 1;
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

  Wt::WTemplate* settingPageTpl = new Wt::WTemplate(Wt::WString::tr("admin-home.tpl"));
  settingPageTpl->bindWidget("title", m_adminPanelTitle = new Wt::WText(m_mainWidget));
  settingPageTpl->bindWidget("contents", m_mgntContents);
  settingPageTpl->bindWidget("menu", m_mgntTopMenu);
  settingPageTpl->bindWidget("info-box", m_infoBox);
  return settingPageTpl;
}


void WebMainUI::createAccountPanel(void)
{
  bool changedPassword(false);
  bool userForm(true);
  UserFormView* form = new UserFormView(&(m_dbSession->loggedUser()),
                                        changedPassword,
                                        userForm);
  form->closeTriggered().connect(std::bind([=](){m_accountPanel->accept();}));
  form->validated().connect(std::bind([=](User userToUpdate) {
    int ret = m_dbSession->updateUser(userToUpdate);
    form->showMessage(ret,
                      "Update failed. More details in log.",
                      "Update completed.");}, std::placeholders::_1));
  
  m_accountPanel = createDialog(tr("Manage user account").toStdString(), form);
}

void WebMainUI::createPasswordPanel(void)
{
  bool changedPassword(true);
  bool userForm(true);
  UserFormView* form = new UserFormView(&(m_dbSession->loggedUser()),
                                        changedPassword,
                                        userForm);
  form->closeTriggered().connect(std::bind([=](){m_changePasswordPanel->accept();}));
  form->changePasswordTriggered().connect(std::bind([=](const std::string& login,
                                                    const std::string& lastpass,
                                                    const std::string& pass) {
    int ret = m_dbSession->updatePassword(login, lastpass, pass);
    form->showMessage(ret,
                      "Change password failed. More details in log.",
                      "Password changed.");
  }, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  m_changePasswordPanel = createDialog(tr("Change password").toStdString(), form);
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
    //FIXME: url not found wApp->redirect(LINK_LOGIN_PAGE);
  }
}


Wt::WComboBox* WebMainUI::createViewSelector(void)
{
  ViewListT views = m_dbSession->viewList();
  
  Wt::WComboBox* viewSelector = new Wt::WComboBox();
  viewSelector->setMargin(10, Wt::Right);
  
  Wt::WStandardItemModel* viewSelectorModel = new Wt::WStandardItemModel(m_mainWidget);
  Wt::WStandardItem *item = new Wt::WStandardItem();
  item->setText("Select a description file");
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
      m_selectFile = boost::any_cast<std::string>(model->item(index, 0)->data());
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

void WebMainUI::createViewAssignmentDialog(void)
{
  m_viewAssignmentDialog = new ViewAssignmentUI(m_dbSession, m_mainWidget);
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
  Wt::WHBoxLayout* layout = new  Wt::WHBoxLayout(thumbs);
  
  
  Wt::WContainerWidget* eventFeeds = new Wt::WContainerWidget(m_mainWidget);
  m_eventFeedLayout = new Wt::WVBoxLayout(eventFeeds);
  
  Wt::WTemplate* m_operatorHomeTpl = new Wt::WTemplate(Wt::WString::tr("operator-home.tpl"));
  m_operatorHomeTpl->bindWidget("info-box", m_infoBox);
  m_operatorHomeTpl->bindWidget("contents", thumbs);
  m_operatorHomeTpl->bindWidget("event-feeds", eventFeeds);
  m_dashtabs->addTab(m_operatorHomeTpl, tr("Operations Console").toStdString());

  m_dbSession->updateViewList(m_dbSession->loggedUser().username);
  m_assignedDashboardCount = m_dbSession->viewList().size();
  for (const auto& view: m_dbSession->viewList()) {
    int tabIndex;
    WebDashboard* dashboard;
    loadView(view.path, dashboard, tabIndex);
    if (dashboard) {
      dashboard->setEventFeedLayout(m_eventFeedLayout);
      layout->addWidget(thumbnail(dashboard));
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
