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

void WebUI::setupTimer(void)
{
  m_timer->setInterval(m_dashboard->getTimerInterval());
  m_timer->timeout().connect(this, &WebUI::handleRefresh);
  m_timer->start();
}

void WebUI::render(void)
{
  setTitle(QObject::tr("%1 - %2 Operations Console").arg(m_dashboard->getConfig(), APP_NAME).toStdString());
  m_mainWidget->setStyleClass("maincontainer");
  Wt::WVBoxLayout* mainLayout(new Wt::WVBoxLayout(m_mainWidget));
  m_mainWidget->setLayout(mainLayout);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(createMenuBarWidget());
  mainLayout->addWidget(m_dashboard->get());
  root()->addWidget(m_mainWidget);
  handleRefresh();
  refresh();
  setupTimer();
}


Wt::WContainerWidget* WebUI::createMenuBarWidget(void)
{
  Wt::WContainerWidget* menuBar(new Wt::WContainerWidget());
  Wt::WHBoxLayout* layout(new Wt::WHBoxLayout(menuBar));
  Wt::WToolBar* toolBar(new Wt::WToolBar());
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(toolBar, Wt::AlignLeft);

  Wt::WPushButton* b(NULL);
  b = createMenuButton("images/built-in/menu_refresh.png", QObject::tr("Refresh").toStdString());
  b->setStyleClass("button");
  b->clicked().connect(this, &WebUI::handleRefresh);
  toolBar->addButton(b);

  b = createMenuButton("images/built-in/menu_zoomin.png", QObject::tr("Zoom in").toStdString());
  b->setStyleClass("button");
  b->clicked().connect(m_dashboard->getMap(), &WebMap::zoomIn);
  toolBar->addButton(b);

  b = createMenuButton("images/built-in/menu_zoomout.png",QObject::tr("Zoom out").toStdString());
  b->setStyleClass("button");
  b->clicked().connect(m_dashboard->getMap(), &WebMap::zoomOut);
  toolBar->addButton(b);

  b = createMenuButton("images/built-in/menu_disket.png", QObject::tr("Save map").toStdString());
  b->setStyleClass("button");
  b->clicked().connect(this, &WebUI::handleRefresh);
  toolBar->addButton(b);

  layout->addWidget(createPopupMenu(), 0, Wt::AlignRight);
  return menuBar;
}


Wt::WContainerWidget* WebUI::createPopupMenu(void)
{
  Wt::WContainerWidget* container(new Wt::WContainerWidget());
  Wt::WPopupMenu* popup(new Wt::WPopupMenu());
  Wt::WMenuItem *item = popup->addItem("images/built-in/help.png", QObject::tr("Help").toStdString());
  popup->addItem(item);

  popup->addSeparator();

  item = popup->addItem("images/built-in/logout.png", QObject::tr("Quit").toStdString());
  popup->addItem(item);

  Wt::WPushButton *button = new Wt::WPushButton(container);
  button->setMenu(popup);

  return container;
}



Wt::WPushButton* WebUI::createMenuButton(const std::string& icon, const std::string& text)
{
  Wt::WPushButton *button = new Wt::WPushButton();
  button->setTextFormat(Wt::XHTMLText);
  button->setText(text);
  button->setIcon(icon);
  return button;
}

void WebUI::resetTimer(qint32 interval)
{
  killTimer(m_dashboard->getTimerId());
  m_dashboard->setTimerId(startTimer(interval));
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
