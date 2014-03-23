/*
# UserMgntUI.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 23-03-2014                                                  #
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

#include "UserMngtUI.hpp"
#include "DbSession.hpp"
#include "WebUtils.hpp"
#include <Wt/WMenu>
#include <Wt/WPanel>
#include <Wt/WComboBox>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/Auth/PasswordStrengthValidator>
#include <Wt/WMessageBox>
#include <Wt/WApplication>
#include <Wt/WTemplate>
#include <Wt/WString>
#include <QObject>


ConfirmPasswordValidator::ConfirmPasswordValidator(UserFormModel* model,
                                                   Wt::WFormModel::Field passField)
  : Wt::WValidator(),
    m_model(model),
    m_passwordField(passField)
{

}

Wt::WValidator::Result ConfirmPasswordValidator::validate(const Wt::WString &input) const
{
  return (m_model->valueText(m_passwordField) == input)?
        Wt::WValidator::Result(Wt::WValidator::Valid):
        Wt::WValidator::Result(Wt::WValidator::Invalid, "Confirmation don't match");
}

UserFormModel::UserFormModel(const User* user,
                             bool changePassword,
                             bool userForm,
                             Wt::WObject *parent)
  : Wt::WFormModel(parent),
    m_userForm(userForm)
{
  addField(UsernameField);
  addField(PasswordField);
  addField(CurrentPasswordField);
  addField(PasswordConfimationField);
  addField(FirstNameField);
  addField(LastNameField);
  addField(EmailField);
  addField(UserLevelField);
  addField(RegistrationDateField);

  setValidator(UsernameField, createNameValidator());;
  setValidator(PasswordField, createPasswordValidator());
  setValidator(PasswordConfimationField, new ConfirmPasswordValidator(this, PasswordField));
  setValidator(FirstNameField, createNameValidator());
  setValidator(FirstNameField, createNameValidator());
  setValidator(LastNameField, createNameValidator());
  setValidator(EmailField, createEmailValidator());
  setValidator(UserLevelField, createNameValidator());

  // These fields shoudl always be read only on update
  if (user && ! changePassword) {
    setReadOnly(UsernameField, true);
    setReadOnly(PasswordField, true);
    setReadOnly(RegistrationDateField, true);
  }

  if (changePassword) {
    setVisible(CurrentPasswordField, true);
    setVisible(PasswordField, true);
    setVisible(PasswordConfimationField, true);
    setVisible(UsernameField, false);
    setVisible(FirstNameField, false);
    setVisible(LastNameField, false);
    setVisible(EmailField, false);
    setVisible(UserLevelField, false);
    setVisible(RegistrationDateField, false);
  } else {
    setVisible(CurrentPasswordField, false);
    if (user) {
      setValue(UsernameField, user->username);
      setValue(FirstNameField, user->firstname);
      setValue(LastNameField, user->lastname);
      setValue(EmailField, user->email);
      setValue(UserLevelField, User::role2Text(user->role));
      setValue(RegistrationDateField, user->registrationDate);

      setVisible(PasswordField, false);
      setVisible(PasswordConfimationField, false);

      setWritable(false);
    } else {
      setVisible(RegistrationDateField, false);
    }
  }
}

void UserFormModel::setWritable(bool writtable)
{
  bool readonly = ! writtable;
  setReadOnly(PasswordConfimationField, readonly);
  setReadOnly(FirstNameField, readonly);
  setReadOnly(LastNameField, readonly);
  setReadOnly(EmailField, readonly);
  if (readonly) {
    setReadOnly(UserLevelField, readonly);
  } else {
    setReadOnly(UserLevelField, m_userForm);
  }
}

Wt::WValidator* UserFormModel::createNameValidator(void)
{
  Wt::WLengthValidator* validator = new Wt::WLengthValidator();
  validator->setInvalidBlankText("Required field");
  validator->setMandatory(true);
  validator->setMinimumLength(1);
  validator->setMaximumLength(MAX_LENGTH);
  return validator;
}

Wt::WValidator* UserFormModel::createPasswordValidator(void)
{
  Wt::Auth::PasswordStrengthValidator* v = new Wt::Auth::PasswordStrengthValidator();
  v->setMinimumLength(Wt::Auth::PasswordStrengthValidator::TwoCharClass, 6);
  v->setMandatory(true);
  return v;
}

Wt::WValidator* UserFormModel::createEmailValidator(void)
{
  Wt::WRegExpValidator* validator = new Wt::WRegExpValidator("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,4}");
  validator->setInvalidBlankText("Required field");
  validator->setInvalidBlankText("Invalid email");
  validator->setMandatory(true);
  return validator;
}

Wt::WValidator* UserFormModel::createConfirmPasswordValidator(void)
{
  return createPasswordValidator();
}

UserFormView::UserFormView(const User* user, bool changePassword, bool userForm)
  : m_changePassword(changePassword),
    m_validated(this),
    m_deleteTriggered(this),
    m_changePasswordTriggered(this),
    m_infoBox(new Wt::WText(""))
{
  m_model = new UserFormModel(user, changePassword, userForm, this);

  setTemplateText(tr("userForm-template"));
  addFunction("id", &WTemplate::Functions::id);
  setFormWidget(UserFormModel::UsernameField, new Wt::WLineEdit());
  setFormWidget(UserFormModel::CurrentPasswordField, createPaswordField());
  setFormWidget(UserFormModel::PasswordField, createPaswordField());
  setFormWidget(UserFormModel::PasswordConfimationField, createPaswordField());
  setFormWidget(UserFormModel::FirstNameField, new Wt::WLineEdit());
  setFormWidget(UserFormModel::LastNameField, new Wt::WLineEdit());
  setFormWidget(UserFormModel::EmailField, new Wt::WLineEdit());
  setFormWidget(UserFormModel::UserLevelField, createUserLevelField());
  setFormWidget(UserFormModel::RegistrationDateField, new Wt::WLineEdit());

  if (user) {
    m_user = *user;
    if (m_changePassword) {
      bindString("change-password-link", "");
    } else {
      if (! userForm) {
        Wt::WPushButton* changedPwdButton = new Wt::WPushButton("Change password");
        changedPwdButton->setStyleClass("btn btn-warning");
        bindEmpty("title");
        bindWidget("change-password-link", changedPwdButton);
        changedPwdButton->clicked().connect(std::bind([=, &user]() { m_changePasswordDialog->show();}));
      } else {
        bindEmpty("change-password-link");
      }
    }
  } else {
    bindEmpty("change-password-link");
  }

  // Bind buttons, but alter later
  Wt::WPushButton* submitButton = new Wt::WPushButton("Submit");
  Wt::WPushButton* cancelButton = new Wt::WPushButton("Clear");
  bindWidget("submit-button", submitButton);
  bindWidget("cancel-button", cancelButton);

  Wt::WString title = Wt::WString("User information");
  if (user) {
    submitButton->setStyleClass("btn btn-success");
    if (changePassword) {
      title = Wt::WString("Set password information");
      submitButton->setText("Change password");
      cancelButton->setText("Close");
      cancelButton->clicked().connect(std::bind([=](){m_close.emit();}));
    } else {
      createChangePasswordDialog();
      submitButton->setText("Update");
      if (! userForm) {
        cancelButton->setText("Delete");
        cancelButton->setStyleClass("btn btn-danger");
        cancelButton->clicked().connect(this, &UserFormView::handleDeleteRequest);
      } else {
        cancelButton->setText("Close");
        cancelButton->clicked().connect(std::bind([=](){m_close.emit();}));
        cancelButton->hide();
      }
    }
  } else {
    submitButton->setStyleClass("btn btn-success");
    cancelButton->clicked().connect(std::bind([=]() {
      m_model->reset();
      updateView(m_model);
    }));
  }

  // If user, it's for update. At first time the fields are disable
  if (user && ! changePassword) {
    submitButton->clicked().connect(std::bind([=](){
      m_model->setWritable(true); updateView(m_model);
      submitButton->clicked().connect(this, &UserFormView::process);
    }));
  } else {
    submitButton->clicked().connect(this, &UserFormView::process);
  }

  bindString("title", title);
  updateView(m_model);
}

UserFormView::~UserFormView(void)
{
}

void UserFormView::reset(void)
{
  m_model->reset();
  updateView(m_model);
}


void UserFormView::process(void)
{
  updateModel(m_model);
  bool isvalid = m_model->validate();
  updateView(m_model);
  if (isvalid) {
    if (m_changePassword) {
      m_changePasswordTriggered.emit(m_user.username,
                                     m_model->valueText(UserFormModel::CurrentPasswordField).toUTF8(),
                                     m_model->valueText(UserFormModel::PasswordField).toUTF8());
    } else {
      m_user.username = m_model->valueText(UserFormModel::UsernameField).toUTF8();
      m_user.password = m_model->valueText(UserFormModel::PasswordField).toUTF8();
      m_user.firstname = m_model->valueText(UserFormModel::FirstNameField).toUTF8();
      m_user.lastname = m_model->valueText(UserFormModel::LastNameField).toUTF8();
      m_user.email = m_model->valueText(UserFormModel::EmailField).toUTF8();
      m_user.role = User::role2Int(m_model->valueText(UserFormModel::UserLevelField).toUTF8());
      m_user.registrationDate = Wt::WDateTime::currentDateTime().toString().toUTF8();
      m_validated.emit(m_user);
    }
  }
}


void UserFormView::handleDeleteRequest(void)
{
  Wt::WMessageBox *confirmationBox = new Wt::WMessageBox
      ("Warning !",
       "<p>Do you really want to delete this user?</p>",
       Wt::Information, Wt::Yes | Wt::No);
  confirmationBox->setModal(false);
  confirmationBox->buttonClicked().connect(std::bind([=] () {
    if (confirmationBox->buttonResult() == Wt::Yes) {
      m_deleteTriggered.emit(m_model->valueText(UserFormModel::UsernameField).toUTF8());
    }
    delete confirmationBox;
  }));
  confirmationBox->show();
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
void UserFormView::createChangePasswordDialog(void)
{
  bool changedPasswd(true);
  bool forUserProfile(false);
  m_changePasswordDialog = new Wt::WDialog("Change password");
  m_changePasswordDialog->setStyleClass("Wt-dialog");
  UserFormView* changedPasswdForm = new UserFormView(&m_user,
                                                     changedPasswd,
                                                     forUserProfile);
  m_changePasswordDialog->contents()->addWidget(changedPasswdForm);
  changedPasswdForm->changePasswordTriggered().connect(
        std::bind([=](const std::string& login, const std::string& currentPass,
                  const std::string& newPass){
    m_changePasswordTriggered.emit(login, currentPass, newPass);
    m_changePasswordDialog->accept();
  }, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  changedPasswdForm->closeTriggered().connect(std::bind([=](){m_changePasswordDialog->accept();}));
}

UserMngtUI::UserMngtUI(DbSession* dbSession)
  : m_dbSession(dbSession),
    m_userForm(new UserFormView(NULL, false, false)),
    m_userListContainer(new Wt::WContainerWidget()),
    m_contents(new Wt::WStackedWidget(0)),
    m_updateCompleted(this)
{
  m_userForm->validated().connect(std::bind([=](User user) {
    m_updateCompleted.emit(m_dbSession->addUser(user));}, std::placeholders::_1));
  createUserList();
}

UserMngtUI::~UserMngtUI(void)
{
  delete m_userForm;
  delete m_userListContainer;
  delete m_contents;
}

void UserMngtUI::updateUserList(void)
{
  m_userListContainer->clear();
  m_dbSession->updateUserList();
  for (auto user: m_dbSession->userList()) {
    m_userListContainer->addWidget(createUserPanel(user));
  }
}

Wt::WPanel* UserMngtUI::createUserPanel(const User& user)
{
  bool changePassword(false);
  bool userForm(false);
  Wt::WAnimation animation(Wt::WAnimation::SlideInFromTop,
                           Wt::WAnimation::EaseOut, 100);

  UserFormView* form(new UserFormView(&user,
                                      changePassword,
                                      userForm));
  form->validated().connect(std::bind([=](User userToUpdate) {
    int ret = m_dbSession->updateUser(userToUpdate);
    m_updateCompleted.emit(m_dbSession->updateUser(userToUpdate));
  }, std::placeholders::_1));

  form->changePasswordTriggered().connect(std::bind([=](const std::string& login,
                                                    const std::string& currentPass,
                                                    const std::string& newPass) {
    m_updateCompleted.emit(m_dbSession->updatePassword(login, currentPass, newPass));
  }, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));


  form->deleteTriggered().connect(std::bind([=](std::string username) {
    m_updateCompleted.emit(m_dbSession->deleteUser(username));
    updateUserList();
  }, std::placeholders::_1));

  Wt::WPanel *panel(new Wt::WPanel());
  panel->setAnimation(animation);
  panel->setCentralWidget(form);
  panel->setTitle(user.username);
  panel->setCollapsible(true);
  panel->setCollapsed(true);
  return panel;
}


void UserMngtUI::createUserList(void)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("user-list-tpl"));
  tpl->bindString("title", "User list");
  tpl->bindWidget("user-list", m_userListContainer);
  m_userListWidget = tpl;
}
