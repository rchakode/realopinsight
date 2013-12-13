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
#include <Wt/WComboBox>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>

UserFormView::UserFormView()
{
  model = new UserFormModel(this);

  setTemplateText(tr("userForm-template"));
  addFunction("id", &WTemplate::Functions::id);

  setFormWidget(UserFormModel::UsernameField, new Wt::WLineEdit());
  setFormWidget(UserFormModel::FirstNameField, new Wt::WLineEdit());
  setFormWidget(UserFormModel::LastNameField, new Wt::WLineEdit());

  Wt::WStandardItemModel* roleModel =  new Wt::WStandardItemModel(2, 1, this);
  Wt::WStandardItem* item = new Wt::WStandardItem(User::role2Text(User::OpRole));
  item->setData(User::OpRole, Wt::UserRole);
  roleModel->setItem(0, 0, item);

  item = new Wt::WStandardItem(User::role2Text(User::AdmRole));
  item->setData(User::AdmRole, Wt::UserRole);
  roleModel->setItem(1, 0, item);

  Wt::WComboBox* roleCbox = new Wt::WComboBox();
  roleCbox->setModel(roleModel);

  setFormWidget(UserFormModel::RoleField, roleCbox);


  // Title & Buttons
  Wt::WString title = Wt::WString("User information");
  bindString("title", title);

  Wt::WPushButton *button = new Wt::WPushButton("Save");
  bindWidget("submit-button", button);

  bindString("submit-info", Wt::WString());

  button->clicked().connect(this, &UserFormView::process);

  updateView(model);
}

void UserFormView::process()
{
  updateModel(model);

  if (model->validate()) {
    // Do something with the data in the model: show it.
    bindString("submit-info",
               Wt::WString::fromUTF8("Saved user data for ")
               + model->userData(), Wt::PlainText);
    // Udate the view: Delete any validation message in the view, etc.
    updateView(model);
    // Set the focus on the first field in the form.
    Wt::WLineEdit *viewField =
        resolve<Wt::WLineEdit*>(UserFormModel::FirstNameField);
    viewField->setFocus();
  } else {
    bindEmpty("submit-info"); // Delete the previous user data.
    updateView(model);
  }
}


Wt::WContainerWidget* createUserList(const UserListT& users)
{
  Wt::WContainerWidget *container = new Wt::WContainerWidget();

  for (auto user: users) {
    container->addWidget(createUserPanel(user));
  }


  return container;
}

Wt::WContainerWidget* createUserForms(const UserListT& users)
{
  Wt::WContainerWidget *container = new Wt::WContainerWidget();
  Wt::WStackedWidget* contents(new Wt::WStackedWidget());

  Wt::WMenu *menu = new Wt::WMenu(contents, Wt::Vertical, container);
  menu->setStyleClass("nav nav-pills");
  menu->addItem("Add User", new UserFormView());
  menu->addItem("User List", createUserList(users));

  container->addWidget(contents);

  return container;
}


Wt::WPanel* createUserPanel(const User& user)
{
  Wt::WAnimation animation(Wt::WAnimation::SlideInFromTop,
                           Wt::WAnimation::EaseOut, 100);
  Wt::WPanel *panel = new Wt::WPanel();
  panel->setTitle(user.username);
  panel->setCollapsible(true);
  panel->setAnimation(animation);
  UserFormView* userForm = new UserFormView();
  UserFormModel* userModel = new UserFormModel();
  userModel->setValue(UserFormModel::FirstNameField, user.firstname);
  userModel->setValue(UserFormModel::LastNameField, user.lastname);
  userModel->setValue(UserFormModel::RoleField, User::role2Text(user.role));
  userModel->setValue(UserFormModel::RegistrationDateField, user.registrationDate);
  userForm->updateView(userModel);
  panel->setCentralWidget(userForm);

  return panel;
}
