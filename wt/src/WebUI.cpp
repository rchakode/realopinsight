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

#include "WebUI.hpp"
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

WebUI::WebUI(const Wt::WEnvironment& env, const QString& config)
  : Wt::WApplication(env),
    m_timer(new Wt::WTimer(this)),
    m_dashboard(new WebDashboard(Auth::OpUserRole, config)), //FIXME: consider user role
    m_mainWidget(new Wt::WContainerWidget())
{
  addEvents();
}

WebUI::~WebUI()
{
  delete m_timer;
  delete m_fileUploadDialog;
  delete m_dashboardMenu;
  delete m_dashboard;
  delete m_mainWidget;
}


void WebUI::render(void)
{
  setTitle(QObject::tr("%1 - %2 Operations Console")
           .arg(m_dashboard->getRootService()->name, APP_NAME)
           .toStdString());
  m_mainWidget->setId("maincontainer");
  Wt::WVBoxLayout* mainLayout(new Wt::WVBoxLayout(m_mainWidget));
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(createMenuBarWidget());
  root()->addWidget(m_mainWidget);
  root()->setId("wrapper");
  handleRefresh();
  refresh();
  resetTimer();
}

Wt::WContainerWidget* WebUI::createMenuBarWidget(void)
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
  m_dashboardMenu->addItem(m_dashboard->getRootService()->name.toStdString(),
                           m_dashboard->get(),
                           Wt::WMenuItem::LazyLoading);
  navigation->addWidget(createToolBar());

  // Setup a Right-aligned menu.
  Wt::WMenu* rightMenu = new Wt::WMenu();
  navigation->addMenu(rightMenu, Wt::AlignRight);

  // Create a popup submenu for the Help menu.
  Wt::WPopupMenu* popup = new Wt::WPopupMenu();
  popup->addItem("Open...")
      ->triggered().connect(std::bind(&WebUI::selectFileToOpen, this));
  popup->addItem("Import")
      ->triggered().connect(std::bind(&WebUI::openFileUploadDialog, this));
  popup->addSeparator();
  popup->addItem("Documentation")
      ->setLink(Wt::WLink(Wt::WLink::Url,"http://realopinsight.com/en/index.php/page/documentation"));
  popup->addItem("About");
  popup->addSeparator();
  popup->addItem("Sign out");

  Wt::WMenuItem* item = new Wt::WMenuItem("Menu");
  item->setMenu(popup);
  rightMenu->addItem(item);

  // Add a Search control.
  Wt::WLineEdit* edit = new Wt::WLineEdit();
  edit->setEmptyText("Enter a search item");
  Wt::WText* searchResult = new Wt::WText("Oups, no match !");
  edit->enterPressed().connect(std::bind([=] () {
    m_dashboardMenu->select(0); // FIXME: is the index of the "Home"
    searchResult->setText(Wt::WString("Nothing found for {1}.").arg(edit->text()));
  }));

  navigation->addSearch(edit, Wt::AlignRight);
  return menuBar;
}


Wt::WContainerWidget* WebUI::createToolBar(void)
{
  Wt::WContainerWidget* container(new Wt::WContainerWidget());
  Wt::WHBoxLayout* layout(new Wt::WHBoxLayout(container));
  Wt::WToolBar* toolBar(new Wt::WToolBar());
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(toolBar, Wt::AlignLeft);

  Wt::WPushButton* b(NULL);
  b = createTooBarButton("images/built-in/menu_refresh.png");
  b->setStyleClass("button");
  b->clicked().connect(this, &WebUI::handleRefresh);
  toolBar->addButton(b);

  b = createTooBarButton("images/built-in/menu_zoomin.png");
  b->setStyleClass("button");
  b->clicked().connect(std::bind(&WebPieMap::scaleMap, m_dashboard->getMap(), utils::SCALIN_FACTOR));
  toolBar->addButton(b);

  b = createTooBarButton("images/built-in/menu_zoomout.png");
  b->setStyleClass("button");
  b->clicked().connect(std::bind(&WebPieMap::scaleMap, m_dashboard->getMap(), utils::SCALOUT_FACTOR));
  toolBar->addButton(b);

  b = createTooBarButton("images/built-in/menu_disket.png");
  b->setStyleClass("button");
  b->clicked().connect(this, &WebUI::handleRefresh);
  toolBar->addButton(createTooBarButton("images/built-in/menu_disket.png"));

  return container;
}

Wt::WPushButton* WebUI::createTooBarButton(const std::string& icon)
{
  Wt::WPushButton* button = new Wt::WPushButton();
  button->setIcon(icon);
  return button;
}

void WebUI::resetTimer(void)
{
  m_timer->setInterval(m_dashboard->getTimerInterval());
  m_timer->timeout().connect(this, &WebUI::handleRefresh);
  m_timer->start();
}

void WebUI::resetTimer(qint32 interval)
{
  m_timer->stop();
  m_timer->setInterval(interval);
  m_timer->start();
}

void WebUI::handleRefresh(void)
{
  m_timer->stop();
  m_mainWidget->disable();
  m_dashboard->runMonitor();
  m_dashboard->updateMap();
  m_mainWidget->enable();
  m_timer->start();
}

void WebUI::addEvents(void)
{
  connect(m_dashboard, SIGNAL(timerIntervalChanged(qint32)), this, SLOT(resetTimer(qint32)));
}

Wt::WAnchor* WebUI::createLogoLink(void)
{
  Wt::WAnchor* anchor = new Wt::WAnchor(Wt::WLink("http://realopinsight.com/"),
                                        new Wt::WImage("images/built-in/logo-mini.png"));
  anchor->setTarget(Wt::TargetNewWindow);
  anchor->setMargin(10, Wt::Right);
  return anchor;
}


void WebUI::selectFileToOpen(void)
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
  QFileInfoList files = cdir.entryInfoList(QStringList("*.ngrt4n.xml"), QDir::Files);
  if (! files.empty()) {
    Q_FOREACH(const QFileInfo& f, files) {
      flist->addItem(f.fileName().toStdString());
    }
    flist->setCurrentIndex(1);
  } else {
    infoBox->setText(QObject::tr("No configuration available").toStdString());
  }

  // Provide a button to close the window
  Wt::WPushButton* finish(new Wt::WPushButton(QObject::tr("Finish").toStdString(), container));
  finish->clicked().connect(std::bind([&]() {
    m_selectFile = CONFIG_DIR.toStdString() +"/"+ flist->itemText(flist->currentIndex()).toUTF8();
    finishFileDialog(OPEN);
    //openFile(flist->itemText(flist->currentIndex()).toUTF8());
  }));

  m_fileUploadDialog->show();
}

void WebUI::openFileUploadDialog(void)
{
  m_fileUploadDialog = new Wt::WDialog(QObject::tr("Import a file").toStdString());
  Wt::WContainerWidget* container(new Wt::WContainerWidget(m_fileUploadDialog->contents()));

  container->setMargin(10, Wt::All);

  m_uploader = new Wt::WFileUpload(container);
  m_uploader->uploaded().connect(std::bind(&WebUI::finishFileDialog, this, IMPORT));
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

void WebUI::finishFileDialog(int action)
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
      // m_fileUploadDialog->contents()->clear();
      openFile(m_selectFile);
      break;
    default:
      break;
  }
}


void WebUI::openFile(const std::string& path)
{
  std::cout << path;
  WebDashboard* dashboard(new WebDashboard(m_dashboard->getUserRole(),
                                           QString::fromStdString(path)));
  m_dashboardMenu->addItem(m_dashboard->getRootService()->name.toStdString(),
                           dashboard->get(),
                           Wt::WMenuItem::LazyLoading);
}

