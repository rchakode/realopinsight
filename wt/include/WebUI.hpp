/*
 * MainWebWindow.hpp
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


#ifndef MAINWEBWINDOW_HPP
#define MAINWEBWINDOW_HPP

#include <Wt/WTimer>
#include <Wt/WApplication>
#include "WebDashboard.hpp"

class WebUI : public QObject, public Wt::WApplication
{
  Q_OBJECT
public:
  WebUI(const Wt::WEnvironment& env, const QString& config);
  virtual ~WebUI();
  void render(void);
  QString getConfig (void) const {return m_dashboard->getConfig();}
  void enable(void) {m_mainWidget->enable();}
  void disbale(void) {m_mainWidget->disable();}
  void resetTimer(void);

public slots:
  void resetTimer(qint32 interval);

private:
  Wt::WTimer* m_timer;
  WebDashboard* m_dashboard;
  Wt::WContainerWidget* createMenuBarWidget(void);
  Wt::WContainerWidget* createPopupMenu(void);

  Wt::WContainerWidget* m_mainWidget;

  Wt::WPushButton* createMenuButton(const std::string& icon,const std::string& text);
  void handleRefresh(void);


  void addEvents(void);
};

#endif // MAINWEBWINDOW_HPP
