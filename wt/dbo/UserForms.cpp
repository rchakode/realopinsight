/*
 * UserForm.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 13-12-2013                                                 #
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


#include "UserForms.hpp"
#include <Wt/WMenu>
#include <Wt/WPanel>

Wt::WContainerWidget* createUserList(void)
{
  Wt::WContainerWidget *container = new Wt::WContainerWidget();

//  dbo::Transaction transaction(session);

//  typedef dbo::collection< dbo::ptr<User> > Users;

//  Users users = session.find<User>();

//  std::cerr << "We have " << users.size() << " users:" << std::endl;

//  for (Users::const_iterator i = users.begin(); i != users.end(); ++i)
//    std::cerr << " user " << (*i)->name
//  << " with karma of " << (*i)->karma << std::endl;
//  transaction.commit();

  Wt::WPanel *panel = new Wt::WPanel();
  panel->setTitle("User 1");
  panel->setCollapsible(true);
  Wt::WAnimation animation(Wt::WAnimation::SlideInFromTop,
         Wt::WAnimation::EaseOut, 100);

  panel->setAnimation(animation);
  panel->setCentralWidget(new Wt::WText("User details."));
  container->addWidget(panel);
  return container;
}

Wt::WContainerWidget* createUserForms(void)
{
  Wt::WContainerWidget *container = new Wt::WContainerWidget();
  Wt::WStackedWidget* contents(new Wt::WStackedWidget());

  Wt::WMenu *menu = new Wt::WMenu(contents, Wt::Vertical, container);
  menu->setStyleClass("nav nav-pills");
  menu->addItem("Add User", new UserFormView());
  menu->addItem("User List", createUserList());

  container->addWidget(contents);

  return container;
}
