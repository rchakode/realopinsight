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
#include <Wt/WToolBar>
#include <Wt/WPushButton>
#include <Wt/WPopupMenu>
#include <functional>
#include <Wt/WNavigationBar>
#include <Wt/WMessageBox>
#include <Wt/WLineEdit>

WebUI::WebUI(const Wt::WEnvironment& env, const QString& config)
  : Wt::WApplication(env),
    m_timer(new Wt::WTimer(this)),
    m_dashboard(new WebDashboard(Auth::OpUserRole, config)),
    m_mainWidget(new Wt::WContainerWidget())
{
  addEvents();
}

WebUI::~WebUI()
{
  delete m_timer;
  delete m_dashboard;
  delete m_mainWidget;
}


void WebUI::render(void)
{
  setTitle(QObject::tr("%1 - %2 Operations Console")
           .arg(m_dashboard->getRootService()->name, APP_NAME).toStdString());
  m_mainWidget->setStyleClass("maincontainer");
  Wt::WVBoxLayout* mainLayout(new Wt::WVBoxLayout(m_mainWidget));
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(createMenuBarWidget());
  root()->addWidget(m_mainWidget);
  handleRefresh();
  refresh();
  resetTimer();
}

Wt::WContainerWidget* WebUI::createMenuBarWidget(void)
{
  Wt::WContainerWidget* menuBar(new Wt::WContainerWidget());
  Wt::WNavigationBar* navigation = new Wt::WNavigationBar(menuBar);
  navigation->setTitle("(c) RealOpInsight.com", "http://realopinsight.com/");
  navigation->addStyleClass("panel");
  navigation->setResponsive(true);
  Wt::WStackedWidget* contentsStack = new Wt::WStackedWidget(menuBar);
  contentsStack->addStyleClass("maincontainer");

  // Setup a Left-aligned menu.
  Wt::WMenu *leftMenu = new Wt::WMenu(contentsStack, menuBar);
  navigation->addMenu(leftMenu);

  leftMenu->addItem("images/built-in/menu_refresh.png",
                    m_dashboard->getRootService()->name.toStdString(),
                    m_dashboard->get(),
                    Wt::WMenuItem::LazyLoading);
  navigation->addWidget(createToolBar());

  // Setup a Right-aligned menu.
  Wt::WMenu* rightMenu = new Wt::WMenu();
  navigation->addMenu(rightMenu, Wt::AlignRight);

  // Create a popup submenu for the Help menu.
  Wt::WPopupMenu* popup = new Wt::WPopupMenu();
  popup->addItem("Documentation")
      ->setLink(Wt::WLink(Wt::WLink::Url, "http://realopinsight.com/en/index.php/page/documentation"));
  popup->addSeparator();
  popup->addItem("About");

  Wt::WMenuItem *item = new Wt::WMenuItem("Help");
  item->setMenu(popup);
  rightMenu->addItem(item);

  // Add a Search control.
  Wt::WLineEdit *edit = new Wt::WLineEdit();
  edit->setEmptyText("Enter a search item");
  Wt::WText* searchResult = new Wt::WText("Oups, no match !");
  edit->enterPressed().connect(std::bind([=] () {
    leftMenu->select(0); // FIXME: is the index of the "Home"
    searchResult->setText(Wt::WString("Nothing found for {1}.").arg(edit->text()));
  }));

  navigation->addSearch(edit, Wt::AlignRight);

  menuBar->addWidget(contentsStack);

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

  b = createTooBarButton("images/built-in/zoomin.png");
  b->setStyleClass("button");
  b->clicked().connect(m_dashboard->getMap(), &WebMap::zoomIn);
  toolBar->addButton(b);

  b = createTooBarButton("images/built-in/zoomout.png");
  b->setStyleClass("button");
  b->clicked().connect(m_dashboard->getMap(), &WebMap::zoomOut);
  toolBar->addButton(b);

  b = createTooBarButton("images/built-in/disket.png");
  b->setStyleClass("button");
  b->clicked().connect(this, &WebUI::handleRefresh);
  toolBar->addButton(createTooBarButton("images/built-in/disket.png"));

  return container;
}

Wt::WPushButton* WebUI::createTooBarButton(const std::string& icon)
{
  Wt::WPushButton *button = new Wt::WPushButton();
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
