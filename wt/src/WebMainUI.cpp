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

#include "WebMainUI.hpp"
#include "utilsClient.hpp"
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
#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/Login>
#include <Wt/WHBoxLayout>

typedef Wt::Auth::AuthWidget AuthWidget;
namespace {
  //  Wt::Auth::Login login; /* slot conflict if decleared in the DbSession class */
  const std::string LINK_LOAD ="/load-platform";
  const std::string LINK_IMPORT ="/import-platform";
  const std::string LINK_LOGIN_PAGE ="/login";
  const std::string LINK_ADMIN_HOME ="/adm-dashboard";
}

WebMainUI::WebMainUI(const Wt::WEnvironment& env)
  : Wt::WApplication(env),
    m_userRole(Auth::OpUserRole), //FIXME: consider user role
    m_settings (new Settings()),
    m_timer(new Wt::WTimer(this)),
    m_mainWidget(new Wt::WContainerWidget()),
    m_dashtabs(new Wt::WTabWidget()),
    m_infoBox(new Wt::WText("", m_mainWidget)),
    m_dbSession(new DbSession())
{
  root()->setId("wrapper");
  m_mainWidget->setId("maincontainer");
  m_infoBox->addStyleClass("alert alert-warning alert-dismissable");
  m_infoBox->hide();
  addEvents();
}

WebMainUI::~WebMainUI()
{
  delete m_timer;
  delete m_infoBox;
  delete m_fileUploadDialog;
  delete m_dashtabs;
  delete m_mainWidget;
  delete m_dbSession;
}


void WebMainUI::addEvents(void)
{
  m_login.changed().connect(this, &WebMainUI::handleAuthentification);
  internalPathChanged().connect(this, &WebMainUI::handleInternalPath);
  connect(m_settings, SIGNAL(timerIntervalChanged(qint32)), this, SLOT(resetTimer(qint32)));
}

void WebMainUI::showLoginHome(void)
{
  root()->addWidget(createLoginHome());
}

Wt::WWidget* WebMainUI::createLoginHome(void)
{
  setInternalPath(LINK_LOGIN_PAGE);
  setTitle(QObject::tr("Authentication - %1 Operations Console").arg(APP_NAME).toStdString());
  AuthWidget* authWidget = new AuthWidget( DbSession::auth(),
                                           m_dbSession->users(),
                                           m_login);
  authWidget->addStyleClass("login-container");
  authWidget->model()->addPasswordAuth(&m_dbSession->passwordAuthentificator());
  authWidget->setRegistrationEnabled(false);
  authWidget->processEnvironment();
  return authWidget;
}

void WebMainUI::showAdminHome(void)
{
  checkUserLogin();
  setInternalPath(LINK_ADMIN_HOME);
  setTitle(QObject::tr("%1 Operations Console").arg(APP_NAME).toStdString());
  Wt::WVBoxLayout* mainLayout(new Wt::WVBoxLayout(m_mainWidget));
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(createNavBar());
  // mainLayout->addWidget(createMenuBarWidget());
  createAdminHome();
  resetTimer();
  root()->addWidget(m_mainWidget);
}

Wt::WWidget* WebMainUI::createNavBar(void)
{
  checkUserLogin();
  Wt::WNavigationBar* bar(new Wt::WNavigationBar());
  bar->addWidget(createLogoLink(), Wt::AlignLeft);

  // Setup the main menu
  Wt::WStackedWidget* contentsStack = new Wt::WStackedWidget();
  contentsStack->setId("stackcontentarea");
  Wt::WMenu* mainMenu = new Wt::WMenu(contentsStack);
  bar->addMenu(mainMenu, Wt::AlignLeft);
  bar->addWidget(createToolBar());
  mainMenu->addItem("home", m_dashtabs, Wt::WMenuItem::LazyLoading);

  Wt::WMenu* profileMenu(new Wt::WMenu());
  bar->addMenu(profileMenu, Wt::AlignRight);
  profileMenu->addItem("Documentation")
      ->setLink(Wt::WLink(Wt::WLink::Url,"http://realopinsight.com/en/index.php/page/documentation"));

  Wt::WPopupMenu* popup = new Wt::WPopupMenu();
  Wt::WMenuItem* item = new Wt::WMenuItem(QObject::tr("You are %1").arg(m_dbSession->loggedUser().username.c_str()).toStdString());
  item->setMenu(popup);
  profileMenu->addItem(item);

  popup->addItem(QObject::tr("Edit profile").toStdString().c_str())
      ->triggered().connect(std::bind([=](){ m_login.logout();}));
  popup->addItem(QObject::tr("Change password").toStdString().c_str())
      ->triggered().connect(std::bind([=](){ m_login.logout();}));
  popup->addItem("Logout")
      ->triggered().connect(std::bind([=](){ m_login.logout();}));
  return bar;
}

Wt::WWidget* WebMainUI::createMenuBarWidget(void)
{
  checkUserLogin();
  Wt::WContainerWidget* menuBar(new Wt::WContainerWidget());
  Wt::WNavigationBar* navBar = new Wt::WNavigationBar(menuBar);
  navBar->setResponsive(true);
  //  // Add a Search control.
  //  Wt::WLineEdit* edit = new Wt::WLineEdit();
  //  edit->setEmptyText("Enter a search item");
  //  edit->enterPressed().connect(std::bind([=] () {
  //    mainMenu->select(0); // FIXME: is the index of the "Home"
  //    m_infoBox->setText(Wt::WString("Nothing found for {1}.").arg(edit->text()));
  //    m_infoBox->setHidden(false);
  //  }));

  //  navBar->addSearch(edit, Wt::AlignRight);
  return menuBar;
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
  b->setStyleClass("button");
  b->clicked().connect(this, &WebMainUI::handleRefresh);
  toolBar->addButton(b);

  b = createTooBarButton("/images/built-in/menu_zoomin.png");
  b->setStyleClass("button");
  b->clicked().connect(std::bind(&WebMainUI::scaleMap, this, utils::SCALIN_FACTOR));
  toolBar->addButton(b);

  b = createTooBarButton("/images/built-in/menu_zoomout.png");
  b->setStyleClass("button");
  b->clicked().connect(std::bind(&WebMainUI::scaleMap, this, utils::SCALOUT_FACTOR));
  toolBar->addButton(b);

  b = createTooBarButton("/images/built-in/menu_disket.png");
  b->setStyleClass("button");
  b->clicked().connect(this, &WebMainUI::handleRefresh);
  toolBar->addButton(createTooBarButton("/images/built-in/menu_disket.png"));

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
  m_fileUploadDialog = new Wt::WDialog(QObject::tr("Select a file").toStdString());
  Wt::WContainerWidget* container(new Wt::WContainerWidget(m_fileUploadDialog->contents()));

  container->setMargin(10, Wt::All);

  Wt::WSelectionBox* flist = new Wt::WSelectionBox(container);
  flist->setMargin(10, Wt::Right);

  // Create a text box to display info
  Wt::WText* infoBox = new Wt::WText("", container);

  // List the configuration avaialable
  QDir cdir(CONFIG_DIR);
  flist->addItem("");
  flist->setCurrentIndex(1);
  QFileInfoList files = cdir.entryInfoList(QStringList("*.ngrt4n.xml"), QDir::Files);
  if (! files.empty()) {
    Q_FOREACH(const QFileInfo& f, files) {
      flist->addItem(f.fileName().toStdString());
    }
  } else {
    infoBox->setText(QObject::tr("No configuration available").toStdString());
  }

  // Set the selected file dynamically
  flist->activated().connect(std::bind([=]() {
    m_selectFile = flist->currentText().toUTF8();
  }));

  // Provide a button to close the window
  Wt::WPushButton* finish(new Wt::WPushButton(QObject::tr("Finish").toStdString(), container));
  finish->clicked().connect(std::bind(&WebMainUI::finishFileDialog, this, OPEN));

  m_fileUploadDialog->show();
}

void WebMainUI::openFileUploadDialog(void)
{
  checkUserLogin();
  m_fileUploadDialog = new Wt::WDialog(QObject::tr("Import a file").toStdString());
  Wt::WContainerWidget* container(new Wt::WContainerWidget(m_fileUploadDialog->contents()));

  container->setMargin(10, Wt::All);

  m_uploader = new Wt::WFileUpload(container);
  m_uploader->uploaded().connect(std::bind(&WebMainUI::finishFileDialog, this, IMPORT));
  m_uploader->setFileTextSize(1024); //max=1MB
  m_uploader->setProgressBar(new Wt::WProgressBar());
  m_uploader->setMargin(10, Wt::Right);

  // Create a text zone to display message
  Wt::WText* infoBox = new Wt::WText(container);

  // Provide a button to start uploading.
  Wt::WPushButton* uploadButton = new Wt::WPushButton(QObject::tr("Upload").toStdString(), container);
  uploadButton->clicked().connect(std::bind([=](){
    m_uploader->upload();
    uploadButton->disable();
  }));

  // Provide a button to close the upload dialog
  Wt::WPushButton* close(new Wt::WPushButton(QObject::tr("Finish").toStdString(), container));
  close->clicked().connect(std::bind([=](){
    uploadButton->enable();
    m_fileUploadDialog->accept();
    m_fileUploadDialog->contents()->clear();
  }));

  // React to a succesfull upload.
  m_uploader->uploaded().connect(std::bind([=] () {
    infoBox->setText(QObject::tr("File upload is finished.").toStdString());
  }));

  // React to a file upload problem.
  m_uploader->fileTooLarge().connect(std::bind([=] () {
    infoBox->setText(QObject::tr("File is too large.").toStdString());
  }));
  m_fileUploadDialog->show();
}

void WebMainUI::finishFileDialog(int action)
{
  checkUserLogin();
  switch(action) {
  case IMPORT:
    if (! m_uploader->empty()) {
      QDir cdir(CONFIG_DIR);
      if (! cdir.exists() && ! cdir.mkdir(cdir.absolutePath())) {
        //FIXME: display in console
        utils::alert(QObject::tr("Unable to use the configuration directory (%1)").arg(cdir.absolutePath()));
      }
      QFile file(QString::fromStdString(m_uploader->spoolFileName()));
      file.copy(QString("%1/%2").arg(cdir.absolutePath(),
                                     QString::fromStdString(m_uploader->clientFileName().toUTF8())));
    }
    break;
  case OPEN:
    m_fileUploadDialog->accept();
    m_fileUploadDialog->contents()->clear();
    if (! m_selectFile.empty()) {
      openFile(m_selectFile);
      m_selectFile.clear();
    } else {
      m_infoBox->setText(QObject::tr("No file selected").toStdString());
      m_infoBox->setHidden(false);
    }
    break;
  default:
    break;
  }
}

void WebMainUI::openFile(const std::string& path)
{
  checkUserLogin();
  std::string realPath = CONFIG_DIR.toStdString()+"/"+path;
  WebDashboard* dashboard = new WebDashboard(m_userRole,
                                             QString::fromStdString(realPath));
  if (! dashboard->errorState()) {
    std::string platform = dashboard->rootService()->name.toStdString();
    std::pair<DashboardListT::iterator, bool> result;
    result = m_dashboards.insert(std::pair<std::string, WebDashboard*>(platform, dashboard));
    if (result.second) {
      m_dashtabs->addTab(dashboard->get(), platform, Wt::WTabWidget::LazyLoading)
          ->triggered().connect(std::bind([=](){
        m_currentDashboard = dashboard;
        setInternalPath("/"+platform);
      }));
      handleRefresh();
    } else {
      delete dashboard;
      m_infoBox->setText(QObject::tr("This platform or a platfom "
                                     "with the same name is already loaded").toStdString());
      m_infoBox->setHidden(false);
    }
  } else {
    m_infoBox->setText(dashboard->lastError().toStdString()); //FIXME: set it somewhere
    m_infoBox->setHidden(false);
  }
}

void WebMainUI::scaleMap(double factor)
{
  checkUserLogin();
  m_currentDashboard->map()->scaleMap(factor);
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
    showAdminHome();
  } else if (path == LINK_LOGIN_PAGE) {
    showLoginHome();
  } else {
    m_infoBox->setText(QObject::tr("Sorry, the request resource "
                                   "is not available or has been removed").toStdString());
    m_infoBox->setHidden(false);
  }
}

void WebMainUI::createAdminHome(void)
{
  checkUserLogin();
  Wt::WTemplate *tpl = new Wt::WTemplate(Wt::WString::tr("template.home"));
  tpl->bindWidget("info-box", m_infoBox);
  tpl->bindWidget("andhor-load-file",
                  createAnchorForHomeLink(QObject::tr("Open").toStdString(),
                                          QObject::tr("An existing platform").toStdString(),
                                          LINK_LOAD));
  tpl->bindWidget("andhor-import-file",
                  createAnchorForHomeLink(QObject::tr("Import").toStdString(),
                                          QObject::tr("A platform description").toStdString(),
                                          LINK_IMPORT));
  m_dashtabs->addTab(tpl, "Home", Wt::WTabWidget::LazyLoading)
      ->triggered().connect(std::bind([=](){setInternalPath("/home");}));
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


void WebMainUI::handleAuthentification(void)
{
  root()->clear();
  if (m_login.loggedIn()) {
    m_dbSession->setLoggedUser(m_login.user().id());
    Wt::log("notice")<<"[realopinsight] "<< m_dbSession->loggedUser().username<<" logged in.";
    showAdminHome();
  } else {
    Wt::log("notice") << "[realopinsight] "<<"Not connected. Redirecting to login page.";
    showLoginHome();
  }
}


void WebMainUI::checkUserLogin(void)
{
  if (! m_login.loggedIn()) {
    redirect(LINK_LOGIN_PAGE);
  }
}
