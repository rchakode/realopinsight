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

const std::string LINK_LOAD ="/load-platform";
const std::string LINK_IMPORT ="/import-platform";

WebMainUI::WebMainUI(const Wt::WEnvironment& env)
  : Wt::WApplication(env),
    m_userRole(Auth::OpUserRole), //FIXME: consider user role
    m_settings (new Settings()),
    m_timer(new Wt::WTimer(this)),
    m_mainWidget(new Wt::WContainerWidget()),
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
  delete m_dashboardMenu;
  delete m_mainWidget;
  delete m_dbSession;
}


void WebMainUI::addEvents(void)
{
  internalPathChanged().connect(this, &WebMainUI::handleInternalPath);
  //FIXME: use right signal
  connect(m_settings, SIGNAL(timerIntervalChanged(qint32)), this, SLOT(resetTimer(qint32)));
}


void WebMainUI::showAdminHome(void)
{
  root()->addWidget(m_mainWidget);
  setTitle(QObject::tr("%1 Operations Console").arg(APP_NAME).toStdString());
  Wt::WVBoxLayout* mainLayout(new Wt::WVBoxLayout(m_mainWidget));
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(createMenuBarWidget());
  createAdminHome();
  resetTimer();
}

void WebMainUI::showLoginHome(void)
{
  setTitle(QObject::tr("Authentication - %1 Operations Console").arg(APP_NAME).toStdString());
  root()->addWidget(createLoginHome());
  root()->doJavaScript("$(document).ready(function(){"
                       "$('.social-login-box').height( $('.login-box').height() - 160 );"
                       "});");
}

Wt::WContainerWidget* WebMainUI::createMenuBarWidget(void)
{
  Wt::WContainerWidget* menuBar(new Wt::WContainerWidget());
  Wt::WNavigationBar* navigation = new Wt::WNavigationBar(menuBar);
  navigation->setResponsive(true);
  navigation->addWidget(createLogoLink());

  Wt::WStackedWidget* contentsStack = new Wt::WStackedWidget(menuBar);
  contentsStack->setId("stackcontentarea");

  // Setup a Left-aligned menu.
  m_dashboardMenu = new Wt::WMenu(contentsStack);
  navigation->addMenu(m_dashboardMenu);
  navigation->addWidget(createToolBar());

  // Setup a Right-aligned menu.
  Wt::WMenu* rightMenu = new Wt::WMenu();
  navigation->addMenu(rightMenu, Wt::AlignRight);

  // Create a popup submenu for the Help menu.
  Wt::WPopupMenu* popup = new Wt::WPopupMenu();
  Wt::WMenuItem* item = popup->addItem("Open...");
  item->setLink(Wt::WLink(Wt::WLink::InternalPath, LINK_LOAD));
  item = popup->addItem("Import");
  item->setLink(Wt::WLink(Wt::WLink::InternalPath, LINK_IMPORT));
  popup->addSeparator();
  popup->addItem("Documentation")
      ->setLink(Wt::WLink(Wt::WLink::Url,"http://realopinsight.com/en/index.php/page/documentation"));
  popup->addItem("About");
  popup->addSeparator();
  popup->addItem("Sign out");

  item = new Wt::WMenuItem("Menu");
  item->setMenu(popup);
  rightMenu->addItem(item);

  // Add a Search control.
  Wt::WLineEdit* edit = new Wt::WLineEdit();
  edit->setEmptyText("Enter a search item");
  edit->enterPressed().connect(std::bind([=] () {
    m_dashboardMenu->select(0); // FIXME: is the index of the "Home"
    m_infoBox->setText(Wt::WString("Nothing found for {1}.").arg(edit->text()));
    m_infoBox->setHidden(false);
  }));

  navigation->addSearch(edit, Wt::AlignRight);
  return menuBar;
}


Wt::WContainerWidget* WebMainUI::createToolBar(void)
{
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
  Wt::WPushButton* button = new Wt::WPushButton();
  button->setIcon(icon);
  return button;
}

void WebMainUI::resetTimer(void)
{
  m_timer->setInterval(1000*m_settings->updateInterval());
  m_timer->timeout().connect(this, &WebMainUI::handleRefresh);
  m_timer->start();
}

void WebMainUI::resetTimer(qint32 interval)
{
  m_timer->stop();
  m_timer->setInterval(interval);
  m_timer->start();
}

void WebMainUI::handleRefresh(void)
{
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
  Wt::WAnchor* anchor = new Wt::WAnchor(Wt::WLink("http://realopinsight.com/"),
                                        new Wt::WImage("/images/built-in/logo-mini.png"));
  anchor->setTarget(Wt::TargetNewWindow);
  anchor->setMargin(10, Wt::Right);
  return anchor;
}


void WebMainUI::selectFileToOpen(void)
{
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
  std::string realPath = CONFIG_DIR.toStdString()+"/"+path;
  WebDashboard* dashboard = new WebDashboard(m_userRole,
                                             QString::fromStdString(realPath));
  if (! dashboard->errorState()) {
    std::string platform = dashboard->rootService()->name.toStdString();
    std::pair<DashboardListT::iterator, bool> result;
    result = m_dashboards.insert(std::pair<std::string, WebDashboard*>(platform, dashboard));
    if (result.second) {
      Wt::WMenuItem *item = m_dashboardMenu->addItem(platform,
                                                     dashboard->get(),
                                                     Wt::WMenuItem::LazyLoading);
      item->triggered().connect(std::bind([=](){
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
  m_currentDashboard->map()->scaleMap(factor);
}

void WebMainUI::handleInternalPath(void)
{
  Wt::WApplication *app = Wt::WApplication::instance();
  if (app->internalPath() == LINK_LOAD) {
    selectFileToOpen();
    setInternalPath("");
  } else if (app->internalPath() == LINK_IMPORT) {
    openFileUploadDialog();
    setInternalPath("");
  } else  {
    m_infoBox->setText(QObject::tr("Sorry, the request resource "
                                   "is not available or has been removed").toStdString());
    m_infoBox->setHidden(false);
  }
}

void WebMainUI::createAdminHome(void)
{
  Wt::WTemplate *tpl = new Wt::WTemplate(Wt::WString::tr("template.home"));
  tpl->bindWidget("info-box", m_infoBox);
  tpl->bindWidget("andhor-load-file",
                  createAnchorForHomeLink("Open", "An existing platform", LINK_LOAD));
  tpl->bindWidget("andhor-import-file",
                  createAnchorForHomeLink("Import", "A platform description", LINK_IMPORT));
  m_dashboardMenu->addItem("Home", tpl, Wt::WMenuItem::LazyLoading)
      ->triggered().connect(std::bind([=](){setInternalPath("/home");}));
}

Wt::WWidget* WebMainUI::createLoginHome(void)
{
  Wt::WTemplate *tpl = new Wt::WTemplate(Wt::WString::tr("template.login.form"));
  //  tpl->bindWidget("info-box", m_infoBox);
  tpl->bindWidget("username-field", new Wt::WLineEdit());

  Wt::WLineEdit* pwdField(new Wt::WLineEdit());
  pwdField->setEchoMode(Wt::WLineEdit::Password);
  tpl->bindWidget("password-field", pwdField);
  tpl->bindWidget("login-button",new Wt::WPushButton(QObject::tr("Login").toStdString()));
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

