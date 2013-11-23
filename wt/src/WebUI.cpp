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

WebUI::WebUI(const Wt::WEnvironment& env, const QString& config)
  : Wt::WApplication(env),
    m_dashboard(new WebDashboard(Auth::OpUserRole, config))
{
}

WebUI::~WebUI()
{
  delete m_dashboard;
}

void WebUI::render(void)
{
  Wt::WContainerWidget* mainContainer(new Wt::WContainerWidget());
  mainContainer->setStyleClass("maincontainer");
  Wt::WVBoxLayout* mainLayout(new Wt::WVBoxLayout(mainContainer));
  mainContainer->setLayout(mainLayout);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(createMenuBarWidget());
  mainLayout->addWidget(m_dashboard->get());
  setTitle(QObject::tr("%1 Operations Console - %2").arg(APP_NAME, m_dashboard->getConfig()).toStdString());
  root()->addWidget(mainContainer);
  handleRefresh();
  refresh();
}


Wt::WContainerWidget* WebUI::createMenuBarWidget(void)
{
  Wt::WContainerWidget* menuBar(new Wt::WContainerWidget());
  Wt::WHBoxLayout* layout(new Wt::WHBoxLayout(menuBar));
  Wt::WToolBar* toolBar(new Wt::WToolBar());
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(toolBar);
  Wt::WPushButton* b = createMenuButton("images/built-in/menu_refresh.png", QObject::tr("Refresh").toStdString());
  b->setStyleClass("button");
  b->clicked().connect(this, &WebUI::handleRefresh);
  toolBar->addButton(b);
  toolBar->addButton(createMenuButton("images/built-in/menu_zoomin.png", QObject::tr("Zoom in").toStdString()));
  toolBar->addButton(createMenuButton("images/built-in/menu_zoomout.png",QObject::tr("Zoom out").toStdString()));
  toolBar->addButton(createMenuButton("images/built-in/menu_disket.png", QObject::tr("Save map").toStdString()));
  toolBar->addButton(createMenuButton("images/built-in/help.png", QObject::tr("Help").toStdString()));
  toolBar->addButton(createMenuButton("images/built-in/logout.png",QObject::tr("Quit").toStdString()));
  return menuBar;
}


Wt::WPushButton* WebUI::createMenuButton(const std::string& icon, const std::string& text)
{
  Wt::WPushButton *button = new Wt::WPushButton();
  button->setTextFormat(Wt::XHTMLText);
  button->setText(text);
  button->setIcon(icon);
  return button;
}



void WebUI::handleRefresh(void)
{
  m_dashboard->setEnabled(false);
  //FIXME: handleUpdateStatusBar(tr("updating..."));
  m_dashboard->runMonitor();
  m_dashboard->updateMap();
  //FIXME: handleUpdateStatusBar(tr("update completed"));
  m_dashboard->setEnabled(true);
}
