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

namespace {
  const std::string LINK_HOME ="/realopinsight";
  const std::string LINK_LOAD ="/load-platform";
  const std::string LINK_IMPORT ="/import-platform";
  const std::string LINK_LOGIN_PAGE ="/login";
  const std::string LINK_ADMIN_HOME ="/adm";
  const std::string LINK_OP_HOME ="/op";
}

WebMainUI::WebMainUI(AuthManager* authManager)
  : Wt::WContainerWidget(),
    m_settings (new Settings()),
    m_timer(new Wt::WTimer()),
    m_mainWidget(new Wt::WContainerWidget(this)),
    m_dashtabs(new Wt::WTabWidget()),
    m_fileUploadDialog(createDialog(utils::tr("Select a file"))),
    m_authManager(authManager),
    m_dbSession(m_authManager->session()),
    m_confdir(Wt::WApplication::instance()->docRoot()+"/config"),
    m_terminateSession(this)
{
  m_mainWidget->setId("maincontainer");
  m_dashtabs->addStyleClass("wrapper-container");

  createMainUI();
  createInfoMsgBox();
  createViewAssignmentDialog();
  createAccountPanel();
  createPasswordPanel();
  createAboutDialog();
  setupUserMenus();
  showUserHome();

  addEvents();
}

WebMainUI::~WebMainUI()
{
  // Delete all
  //  m_contents(NULL),
  //  m_navbar(NULL),
  //  m_mgntMenu(NULL),
  //  m_profileMenu(NULL),
  delete m_timer;
  std::cout << "m_timer deleted\n";
  delete m_fileUploadDialog;
  std::cout << "m_fileUploadDialog deleted\n";
  //delete m_dashtabs;
  std::cout << "m_dashtabs deleted\n";
  //FIXME: delete if not null => set them null at startup
  delete m_navbar;
  std::cout << "m_navbar deleted\n";
  delete m_contents;
  std::cout << "m_contents deleted\n";
  delete m_mainWidget;
  std::cout << "m_mainWidget deleted\n";
}


void WebMainUI::addEvents(void)
{
  wApp->internalPathChanged().connect(this, &WebMainUI::handleInternalPath);
  connect(m_settings, SIGNAL(timerIntervalChanged(qint32)), this, SLOT(resetTimer(qint32)));
}

void WebMainUI::showLoginHome(void)
{
  //FIXME: ebMainUI::showLoginHome(void)
  //  m_mainWidget->hide();
  //  refresh();
}

void WebMainUI::showUserHome(void)
{
  wApp->setTitle(tr("%1 Operations Console").arg(APP_NAME).toStdString());
  checkUserLogin();

  std::string homeTabTitle = m_dbSession->loggedUser().role == User::AdmRole?
        utils::tr("Quick Start"): utils::tr("Tactical Overview");

  m_dashtabs->addTab(createUserHome(),
                     homeTabTitle,
                     Wt::WTabWidget::LazyLoading)
      ->triggered().connect(std::bind([=](){setInternalPath("/home");}));

  if (m_dbSession->loggedUser().role == User::OpRole) {
    loadUserDashboard();
  }
  resetTimer();
}

void WebMainUI::createMainUI(void)
{;
  Wt::WContainerWidget* container = new Wt::WContainerWidget();
  m_navbar = new Wt::WNavigationBar(container);
  m_navbar->addWidget(createLogoLink(), Wt::AlignLeft);

  // Create a container for stacked contents
  m_contents = new Wt::WStackedWidget(container);
  m_contents->setId("stackcontentarea");
  // Setup the main menu
  Wt::WMenu* mainMenu (new Wt::WMenu(m_contents));
  m_navbar->addMenu(mainMenu, Wt::AlignLeft);
  mainMenu->addItem(utils::tr("Home"), m_dashtabs); //Fixme: use home icon
  container->addWidget(m_contents);
  setupAdminMenus();
  setupProfileMenus();
  m_mainWidget->addWidget(container);
}


void WebMainUI::setupAdminMenus(void)
{
  Wt::WMenuItem* curItem = NULL;
  m_mgntMenu = new Wt::WMenu(m_contents);
  m_navbar->addMenu(m_mgntMenu, Wt::AlignLeft);
  Wt::WPopupMenu* mgntPopupMenu = new Wt::WPopupMenu();
  curItem = new Wt::WMenuItem(utils::tr("Management"));
  curItem->setMenu(mgntPopupMenu);
  m_mgntMenu->addItem(curItem);


  // Menus for view management
  mgntPopupMenu->addSectionHeader("View");
  mgntPopupMenu->addItem("Import")
      ->setLink(Wt::WLink(Wt::WLink::InternalPath, LINK_IMPORT));
  mgntPopupMenu->addItem("Load")
      ->setLink(Wt::WLink(Wt::WLink::InternalPath, LINK_LOAD));
  mgntPopupMenu->addItem("Assign/revoke/delete")
      ->triggered().connect(std::bind([=](){
    m_viewAssignmentDialog->resetModelData();
    m_viewAssignmentDialog->show();
  }));

  // Menus for user management
  m_userMgntUI = new UserMngtUI(m_dbSession);
  m_contents->addWidget(m_userMgntUI);
  mgntPopupMenu->addSectionHeader("User");
  mgntPopupMenu->addItem("Add")
      ->triggered().connect(std::bind([=](){
    showUserMngtPage(m_contents, UserMngtUI::AddUserAction);
  }));
  mgntPopupMenu->addItem("List")
      ->triggered().connect(std::bind([=](){
    showUserMngtPage(m_contents, UserMngtUI::ListUserAction);
  }));

}

void WebMainUI::setupProfileMenus(void)
{
  m_profileMenu = new Wt::WMenu();
  m_navbar->addMenu(m_profileMenu, Wt::AlignRight);
  Wt::WPopupMenu* profilePopupMenu = new Wt::WPopupMenu();
  m_mainProfileMenuItem = new Wt::WMenuItem("Profile");
  m_mainProfileMenuItem->setMenu(profilePopupMenu);
  m_profileMenu->addItem(m_mainProfileMenuItem);

  Wt::WMenuItem* curItem = NULL;
  profilePopupMenu->addItem(utils::tr("Account").c_str())
      ->triggered().connect(std::bind([=](){m_accountPanel->show();}));
  profilePopupMenu->addItem(utils::tr("Change password").c_str())
      ->triggered().connect(std::bind([=](){m_changePasswordPanel->show();}));
  profilePopupMenu->addSeparator();
  curItem = profilePopupMenu->addItem(utils::tr("Documentation"));
  curItem->setLink(Wt::WLink(Wt::WLink::Url, "http://realopinsight.com/en/index.php/page/documentation"));
  curItem->setLinkTarget(Wt::TargetNewWindow);

  profilePopupMenu->addItem("About")
      ->triggered().connect(std::bind([=](){m_aboutDialog->show();}));
}

void WebMainUI::setupUserMenus(void)
{
  checkUserLogin();
  User loggedUser = m_dbSession->loggedUser();

  m_mainProfileMenuItem->setText(tr("You're %1").arg(loggedUser.username.c_str()).toStdString());
  if(loggedUser.role == User::AdmRole) {
    m_mgntMenu->show();
    setInternalPath(LINK_ADMIN_HOME);
  } else {
    m_mgntMenu->hide();
    setInternalPath(LINK_OP_HOME);
  }

  //FIXME: add this after the first view loaded
  m_navbar->addWidget(createToolBar());
}

Wt::WWidget* WebMainUI::createToolBar(void)
{
  checkUserLogin();
  Wt::WContainerWidget* container(new Wt::WContainerWidget());
  Wt::WHBoxLayout* layout(new Wt::WHBoxLayout(container));
  Wt::WToolBar* toolBar(new Wt::WToolBar());
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(toolBar, Wt::AlignLeft);

  Wt::WPushButton* b(NULL);

  b = createTooBarButton("/images/built-in/menu_refresh.png");
  b->setStyleClass("btn-small");
  b->clicked().connect(this, &WebMainUI::handleRefresh);
  toolBar->addButton(b);

  b = createTooBarButton("/images/built-in/menu_zoomin.png");
  b->setStyleClass("btn-small");
  b->clicked().connect(std::bind(&WebMainUI::scaleMap, this, utils::SCALIN_FACTOR));
  toolBar->addButton(b);

  b = createTooBarButton("/images/built-in/menu_zoomout.png");
  b->setStyleClass("btn-small");
  b->clicked().connect(std::bind(&WebMainUI::scaleMap, this, utils::SCALOUT_FACTOR));
  toolBar->addButton(b);

  return container;
}

Wt::WPushButton* WebMainUI::createTooBarButton(const std::string& icon)
{
  checkUserLogin();
  Wt::WPushButton* button = new Wt::WPushButton();
  button->setIcon(icon);
  return button;
}

void WebMainUI::resetTimer(void)
{
  checkUserLogin();
  m_timer->setInterval(1000*m_settings->updateInterval());
  m_timer->timeout().connect(this, &WebMainUI::handleRefresh);
  m_timer->start();
}

void WebMainUI::resetTimer(qint32 interval)
{
  checkUserLogin();
  m_timer->stop();
  m_timer->setInterval(interval);
  m_timer->start();
}

void WebMainUI::handleRefresh(void)
{
  checkUserLogin();
  m_timer->stop();
  m_mainWidget->disable();
  for(auto&dash: m_dashboards) {
    dash.second->runMonitor();
    dash.second->updateMap();
  }
  m_mainWidget->enable();
  m_timer->start();
}

Wt::WAnchor* WebMainUI::createLogoLink(void)
{
  checkUserLogin();
  Wt::WAnchor* anchor = new Wt::WAnchor(Wt::WLink("http://realopinsight.com/"),
                                        new Wt::WImage("/images/built-in/logo-mini.png"));
  anchor->setTarget(Wt::TargetNewWindow);
  anchor->setMargin(10, Wt::Right);
  return anchor;
}

void WebMainUI::selectFileToOpen(void)
{
  checkUserLogin();
  m_fileUploadDialog->setWindowTitle(utils::tr("Select a file"));
  Wt::WContainerWidget* container(new Wt::WContainerWidget(m_fileUploadDialog->contents()));
  container->clear();

  container->setMargin(10, Wt::All);
  container->addWidget(createViewSelector());

  // Provide a button to close the window
  Wt::WPushButton* finish(new Wt::WPushButton(utils::tr("Finish"), container));
  finish->clicked().connect(std::bind(&WebMainUI::finishFileDialog, this, OPEN));

  m_fileUploadDialog->show();
}

void WebMainUI::openFileUploadDialog(void)
{
  checkUserLogin();
  m_fileUploadDialog->setWindowTitle(utils::tr("Import a file"));
  m_fileUploadDialog->setStyleClass("ngrt4n-gradient Wt-dialog");
  Wt::WContainerWidget* container(new Wt::WContainerWidget(m_fileUploadDialog->contents()));
  container->clear();
  container->setMargin(10, Wt::All);

  m_uploader = new Wt::WFileUpload(container);
  m_uploader->uploaded().connect(std::bind(&WebMainUI::finishFileDialog, this, IMPORT));
  m_uploader->setFileTextSize(1024); //max=1MB
  m_uploader->setProgressBar(new Wt::WProgressBar());
  m_uploader->setMargin(10, Wt::Right);

  // Provide a button to start uploading.
  Wt::WPushButton* uploadButton = new Wt::WPushButton(utils::tr("Upload"), container);
  uploadButton->clicked().connect(std::bind([=](){
    m_uploader->upload();
    uploadButton->disable();
  }));

  // Provide a button to close the upload dialog
  Wt::WPushButton* close(new Wt::WPushButton(utils::tr("Close"), container));
  close->clicked().connect(std::bind([=](){
    uploadButton->enable();
    m_fileUploadDialog->accept();
    m_fileUploadDialog->contents()->clear();
  }));

  // React to a file upload problem.
  m_uploader->fileTooLarge().connect(std::bind([=] () {
    showMessage(utils::tr("File is too large."), "alert alert-warning");
  }));
  m_fileUploadDialog->show();
}

void WebMainUI::finishFileDialog(int action)
{
  checkUserLogin();
  switch(action) {
    case IMPORT:
      if (! m_uploader->empty()) {
        QDir cdir(m_confdir.c_str());
        if (! cdir.exists() && ! cdir.mkdir(cdir.absolutePath())) {
          QString errrMsg = tr("Unable to use the "
                               "configuration directory (%1)").arg(cdir.absolutePath());
          Wt::log("error")<<"[realopinsight]"<<errrMsg.toStdString();
          showMessage(errrMsg.toStdString(), "alert alert-warning");
        } else {
          Wt::log("notice")<<"[realopinsight]"<< " Parsing the input file";
          QString fileName(m_uploader->spoolFileName().c_str());
          CoreDataT cdata;
          Parser parser(fileName ,&cdata);
          connect(&parser, SIGNAL(errorOccurred(QString)), this, SLOT(handleLibError(QString)));
          if (parser.process(false)) {
            std::string tmpPath = m_uploader->clientFileName().toUTF8();
            QString dest = tr("%1/%2").arg(cdir.absolutePath(), tmpPath.c_str());
            QFile file(fileName);
            file.copy(dest);
            file.remove();

            View view;
            view.name = cdata.bpnodes[utils::ROOT_ID].name.toStdString();
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
          } else {
            std::string msg = "Invalid configuration file";
            Wt::log("warn")<<"[realopinsight] "<< msg;
            showMessage(msg, "alert alert-warning");
          }
        }
      }
      break;
    case OPEN:
      m_fileUploadDialog->accept();
      m_fileUploadDialog->contents()->clear();
      if (! m_selectFile.empty()) {
        openFile(m_selectFile);
        m_selectFile.clear();
      } else {
        showMessage(utils::tr("No file selected"), "alert alert-warning");
      }
      break;
    default:
      break;
  }
}

void WebMainUI::openFile(const std::string& path)
{
  checkUserLogin();
  WebDashboard* dashboard = new WebDashboard(m_dbSession->loggedUser().role, path.c_str());
  connect(dashboard, SIGNAL(errorOccurred(QString)), this, SLOT(handleLibError(QString)));

  if (! dashboard->errorState()) {
    std::string platform = dashboard->rootService()->name.toStdString();
    std::pair<DashboardListT::iterator, bool> result;
    result = m_dashboards.insert(std::pair<std::string, WebDashboard*>(platform, dashboard));
    if (result.second) {
      Wt::WMenuItem* tab = m_dashtabs->addTab(dashboard->get(), platform, Wt::WTabWidget::LazyLoading);
      tab->triggered().connect(std::bind([=](){
        m_currentDashboard = dashboard;
        setInternalPath("/"+platform);
      }));
      tab->setCloseable(true);
      m_dashtabs->setCurrentWidget(dashboard->get());
      handleRefresh();
    } else {
      delete dashboard;
      showMessage(utils::tr("This platform or a platfom "
                            "with the same name is already loaded"),"alert alert-warning");
    }
  } else {
    showMessage(dashboard->lastError().toStdString(),"alert alert-warning");
  }
}

void WebMainUI::scaleMap(double factor)
{
  checkUserLogin();
  if (m_currentDashboard) {
    m_currentDashboard->map()->scaleMap(factor);
  }
}

Wt::WWidget* WebMainUI::createUserHome(void)
{
  checkUserLogin();
  Wt::WTemplate *tpl = new Wt::WTemplate(Wt::WString::tr("template.home"));
  tpl->bindWidget("andhor-load-file",
                  createAnchorForHomeLink(utils::tr("Load"),
                                          utils::tr("An existing platform"),
                                          LINK_LOAD));
  tpl->bindWidget("andhor-import-file",
                  createAnchorForHomeLink(utils::tr("Import"),
                                          utils::tr("A platform description"),
                                          LINK_IMPORT));
  return tpl;
}


Wt::WAnchor* WebMainUI::createAnchorForHomeLink(const std::string& title,
                                                const std::string& desc,
                                                const std::string& internalPath)
{
  Wt::WAnchor* anchor(new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath,internalPath),
                                      "<h4 class='list-group-item-heading'>"+title+"</h4>"
                                      "<p class='list-group-item-text'>"+desc+"</p>"));
  anchor->addStyleClass("list-group-item active");
  return anchor;
}


void WebMainUI::checkUserLogin(void)
{
  if (! m_authManager->isLogged()) {
    wApp->redirect(LINK_HOME);
  }
}


void WebMainUI::showUserMngtPage(Wt::WStackedWidget* contents, int destination)
{
  contents->setCurrentWidget(m_userMgntUI);
  setInternalPath("/users");
  m_userMgntUI->showDestinationView(destination);
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
                      "Update completed.");
  }, std::placeholders::_1));

  m_accountPanel = createDialog(utils::tr("Account information"), form);
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
  m_changePasswordPanel = createDialog(utils::tr("Change password"), form);
}


void WebMainUI::handleInternalPath(void)
{
  checkUserLogin();
  std::string path = Wt::WApplication::instance()->internalPath();
  if (path == LINK_LOAD) {
    selectFileToOpen();
    setInternalPath("");
  } else if (path == LINK_IMPORT) {
    openFileUploadDialog();
    setInternalPath("");
  } else if (path == LINK_ADMIN_HOME) {
    showUserHome();
  } else if (path == LINK_LOGIN_PAGE) {
    showLoginHome();
  } else {
    showLoginHome();
    showMessage(utils::tr("Sorry, the request resource "
                          "is not available or has been removed"),
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
  item->setText("Select the view to load");
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

void WebMainUI::createInfoMsgBox(void)
{
  m_infoMsgBox = new Wt::WDialog(m_mainWidget);
  m_infoMsgBox->setStyleClass("ngrt4n-transparent Wt-dialog");
  m_infoMsgBox->setModal(false);
  m_infoMsgBox->setTitleBarEnabled(false);
  m_infoMsgBox->positionAt(m_profileMenu);
}


void WebMainUI::showMessage(const std::string& msg, std::string status)
{
  m_infoMsgBox->contents()->clear();
  Wt::WText* textArea = new Wt::WText(msg, m_infoMsgBox->contents());
  textArea->setStyleClass(status);
  textArea->clicked().connect(std::bind([=](){
    m_infoMsgBox->accept();
  }));
  m_infoMsgBox->positionAt(m_profileMenu);
  m_infoMsgBox->show();
}

void WebMainUI::createViewAssignmentDialog(void)
{
  m_viewAssignmentDialog = new ViewAssignmentUI(m_dbSession, m_mainWidget);
}

void WebMainUI::createAboutDialog(void)
{
  m_aboutDialog = new Wt::WDialog(m_mainWidget);
  m_aboutDialog->setTitleBarEnabled(false);
  m_aboutDialog->setStyleClass("ngrt4n-gradient Wt-dialog");
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("about-tpl"), m_aboutDialog->contents());
  tpl->bindString("software", APP_NAME.toStdString());
  tpl->bindString("version", PKG_VERSION.toStdString());
  tpl->bindString("codename", REL_NAME.toStdString());
  tpl->bindString("release-id", REL_INFO.toStdString());
  tpl->bindString("release-year", REL_YEAR.toStdString());
  tpl->bindString("package-url", PKG_URL.toStdString());
  tpl->bindString("bug-report-email", REPORT_BUG.toStdString());

  Wt::WPushButton* closeButton(new Wt::WPushButton(utils::tr("Close")));
  closeButton->clicked().connect(std::bind([=](){m_aboutDialog->accept();}));
  tpl->bindWidget("close-button", closeButton);
}



void WebMainUI::loadUserDashboard(void)
{
  m_dbSession->updateViewList(m_dbSession->loggedUser().username);
  for (const auto& view: m_dbSession->viewList()) {
    openFile(view.path);
  }
}


void WebMainUI::setInternalPath(const std::string& path)
{
  wApp->setInternalPath(path);
}

Wt::WDialog* WebMainUI::createDialog(const std::string& title, Wt::WWidget* content)
{
  Wt::WDialog* dialog = new Wt::WDialog(utils::tr("Account information"));
  dialog->setStyleClass("ngrt4n-gradient Wt-dialog");
  dialog->titleBar()->setStyleClass("titlebar");
  if (content != NULL) dialog->contents()->addWidget(content);
  return dialog;
}
