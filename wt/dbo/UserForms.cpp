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
#include "DbSession.hpp"
#include <Wt/WMenu>
#include <Wt/WPanel>
#include <Wt/WComboBox>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>

UserFormModel::UserFormModel(const User* user, Wt::WObject *parent)
  : Wt::WFormModel(parent)
{
  addField(UsernameField);
  addField(PasswordField);
  addField(FirstNameField);
  addField(LastNameField);
  addField(EmailField);
  addField(UserLevelField);

  setValidator(UsernameField, createNameValidator(UsernameField));
  setValidator(FirstNameField, createNameValidator(FirstNameField));
  setValidator(FirstNameField, createNameValidator(FirstNameField));
  setValidator(LastNameField, createNameValidator(LastNameField));
  setValidator(EmailField, createEmailValidator(EmailField));

  if (user) {
    setValue(UsernameField, user->username);
    setValue(FirstNameField, user->firstname);
    setValue(LastNameField, user->lastname);
    setValue(EmailField, user->email);
    setValue(UserLevelField, User::role2Text(user->role));
    setValue(RegistrationDateField, user->registrationDate);
  }
}

UserFormView::UserFormView(const User* user):
  m_validated(this)
{
  m_model = new UserFormModel(user, this);

  setTemplateText(tr("userForm-template"));
  addFunction("id", &WTemplate::Functions::id);

  setFormWidget(UserFormModel::UsernameField, new Wt::WLineEdit());
  setFormWidget(UserFormModel::PasswordField, createPaswordField());
  setFormWidget(UserFormModel::FirstNameField, new Wt::WLineEdit());
  setFormWidget(UserFormModel::LastNameField, new Wt::WLineEdit());
  setFormWidget(UserFormModel::EmailField, new Wt::WLineEdit());
  setFormWidget(UserFormModel::UserLevelField, createUserLevelField());

  // Title & Buttons
  Wt::WString title = Wt::WString("User information");
  bindString("title", title);
  Wt::WPushButton *button = new Wt::WPushButton("Submit");
  bindWidget("submit-button", button);

  button->clicked().connect(this, &UserFormView::process);

  updateView(m_model);
}

void UserFormView::process()
{
  updateModel(m_model);
  bool isvalid = m_model->validate();
  updateView(m_model);
  if (isvalid) {
    User user;
    user.username = m_model->valueText(UserFormModel::UsernameField).toUTF8();
    user.firstname = m_model->valueText(UserFormModel::FirstNameField).toUTF8();
    user.lastname = m_model->valueText(UserFormModel::LastNameField).toUTF8();
    user.email = m_model->valueText(UserFormModel::EmailField).toUTF8();
    user.role = User::role2Int(m_model->valueText(UserFormModel::UserLevelField).toUTF8());
    user.registrationDate = Wt::WDateTime::currentDateTime().toString().toUTF8();
    std::string password = m_model->valueText(UserFormModel::PasswordField).toUTF8();
    m_validated.emit(user, password);
  }
}

Wt::WComboBox* UserFormView::createUserLevelField(void)
{
  Wt::WStandardItemModel* roleModel =  new Wt::WStandardItemModel(2, 1, this);
  Wt::WStandardItem* item = new Wt::WStandardItem(User::role2Text(User::OpRole));
  item->setData(User::OpRole, Wt::UserRole);
  roleModel->setItem(0, 0, item);

  item = new Wt::WStandardItem(User::role2Text(User::AdmRole));
  item->setData(User::AdmRole, Wt::UserRole);
  roleModel->setItem(1, 0, item);

  Wt::WComboBox* roleCbox = new Wt::WComboBox();
  roleCbox->setModel(roleModel);
  return roleCbox;
}


Wt::WLineEdit* UserFormView::createPaswordField(void)
{
  Wt::WLineEdit* field(new Wt::WLineEdit());
  field->setEchoMode(Wt::WLineEdit::Password);
  return field;
}

Wt::WContainerWidget* createUserList(DbSession* dbSession)
{
  Wt::WContainerWidget *container = new Wt::WContainerWidget();
  dbSession->updateUserList();
  for (auto user: dbSession->getUserList()) {
    container->addWidget(createUserPanel(user, dbSession));
  }
  return container;
}

Wt::WContainerWidget* createUserForms(DbSession* dbSession)
{
  Wt::WContainerWidget *container(new Wt::WContainerWidget());
  Wt::WStackedWidget* contents(new Wt::WStackedWidget());
  Wt::WMenu *menu(new Wt::WMenu(contents, Wt::Vertical, container));
  menu->setStyleClass("nav nav-pills");

  UserFormView* userForm(new UserFormView(NULL));
  userForm->validated().connect(dbSession, &DbSession::addUser);
  menu->addItem("Add User", userForm);
  menu->addItem("User List", createUserList(dbSession));

  container->addWidget(contents);
  return container;
}


Wt::WPanel* createUserPanel(const User& user, DbSession* dbSession)
{
  Wt::WAnimation animation(Wt::WAnimation::SlideInFromTop,
                           Wt::WAnimation::EaseOut, 100);

  UserFormView* userForm(new UserFormView(&user));
  userForm->validated().connect(dbSession, &DbSession::updateUser);
  Wt::WPanel *panel(new Wt::WPanel());
  panel->setAnimation(animation);
  panel->setCentralWidget(userForm);
  panel->setTitle(user.username);
  panel->setCollapsible(true);
  panel->setCollapsed(true);
  return panel;
}

