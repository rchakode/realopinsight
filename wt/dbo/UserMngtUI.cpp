/*
 * UserMgntUI.cpp
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

#include "UserMngtUI.hpp"
#include "DbSession.hpp"
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

#define ROOT_DIV wApp->root()->id()
#define MAIN_USER_MNGT_DIV this->id()

#define JS_AUTO_RESIZING_SCRIPT(computeWindowHeight) \
  computeWindowHeight \
  "$('#wrapper').height(wh);" \
  "$('#maincontainer').height(wh);" \
  "$('#stackcontentarea').height(wh);" \
  "$('#"+ROOT_DIV+"').height(wh);" \
  "$('#"+MAIN_USER_MNGT_DIV+"').height(wh-40);" \
  "$('#user-mgnt-container').height(wh);"

#define JS_AUTO_RESIZING_FUNCTION \
  "function(self, width, height) {" \
  JS_AUTO_RESIZING_SCRIPT("wh=height;") \
  "}"

ConfirmPasswordValidator::ConfirmPasswordValidator(UserFormModel* model,
                                                   Wt::WFormModel::Field passField)
  : Wt::WValidator(),
    m_model(model),
    m_passwordField(passField)
{

}

Wt::WValidator::Result ConfirmPasswordValidator::validate (const Wt::WString &input) const
{
  return (m_model->valueText(m_passwordField) == input)?
        Wt::WValidator::Result(Wt::WValidator::Valid):
        Wt::WValidator::Result(Wt::WValidator::Invalid, "Confirmation don't match");
}

UserFormModel::UserFormModel(const User* user, bool changePassword, Wt::WObject *parent)
  : Wt::WFormModel(parent)
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

  setValidator(UsernameField, createNameValidator());
  //  setValidator(CurrentPasswordField, createPasswordValidator());
  setValidator(PasswordField, createPasswordValidator());
  setValidator(PasswordConfimationField, new ConfirmPasswordValidator(this, PasswordField));
  setValidator(FirstNameField, createNameValidator());
  setValidator(FirstNameField, createNameValidator());
  setValidator(LastNameField, createNameValidator());
  setValidator(EmailField, createEmailValidator());
  setValidator(UserLevelField, createNameValidator());

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
      setValue(PasswordField, Wt::WString("passwordissecret"));
      setValue(FirstNameField, user->firstname);
      setValue(LastNameField, user->lastname);
      setValue(EmailField, user->email);
      setValue(UserLevelField, User::role2Text(user->role));
      setValue(RegistrationDateField, user->registrationDate);
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
  setReadOnly(UsernameField, true);  // Always read only on update
  setReadOnly(PasswordField, true); // Should be changed differently
  setReadOnly(PasswordConfimationField, readonly);
  setReadOnly(FirstNameField, readonly);
  setReadOnly(LastNameField, readonly);
  setReadOnly(EmailField, readonly);
  setReadOnly(UserLevelField, readonly);
  setReadOnly(RegistrationDateField, readonly);
}

Wt::WValidator* UserFormModel::createNameValidator(void)
{
  Wt::WLengthValidator *v = new Wt::WLengthValidator();
  v->setMandatory(true);
  v->setMinimumLength(1);
  v->setMaximumLength(MAX_LENGTH);
  return v;
}

Wt::WValidator* UserFormModel::createPasswordValidator(void)
{
  Wt::Auth::PasswordStrengthValidator* v = new Wt::Auth::PasswordStrengthValidator();
  v->setMinimumLength(Wt::Auth::PasswordStrengthValidator::TwoCharClass, 6);
  v->setMandatory(true);
  return createNameValidator();
}

Wt::WValidator* UserFormModel::createEmailValidator(void)
{
  return new Wt::WRegExpValidator("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,4}");
}

Wt::WValidator* UserFormModel::createConfirmPasswordValidator(void)
{
  return createPasswordValidator();
}

UserFormView::UserFormView(const User* user, bool changePassword)
  : m_changePassword(changePassword),
    m_validated(this),
    m_deleteTriggered(this),
    m_changePasswordTrigerred(this),
    m_infoBox(new Wt::WText(""))
{
  m_model = new UserFormModel(user, changePassword, this);

  setTemplateText(tr("userForm-template"));
  addFunction("id", &WTemplate::Functions::id);
  bindWidget("info-box", m_infoBox);
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
      Wt::WPushButton* changedPwdAnchor = new Wt::WPushButton("change");
      bindWidget("change-password-link", changedPwdAnchor);
      changedPwdAnchor->clicked().connect(std::bind([=, &user]() {
        m_changePasswordDialog->show();
      }));
    }
  } else {
    bindString("change-password-link", "");
  }

  // Bind buttons, but alter later
  Wt::WPushButton* submitButton = new Wt::WPushButton("Submit");
  bindWidget("submit-button", submitButton);
  Wt::WPushButton* cancelButton = new Wt::WPushButton("Clear");
  bindWidget("cancel-button", cancelButton);
  Wt::WString title = Wt::WString("User information");

  if (user) {
    submitButton->setStyleClass("btn-success");
    if (changePassword) {
      title = Wt::WString("Set password information");
      submitButton->setText("Change password");
     // submitButton->clicked().connect(this, &UserFormView::triggerChangePassword);

      cancelButton->setText("Cancel");
      cancelButton->clicked().connect(std::bind([=](){
        //FIXME: m_changePasswordDialog->accept(); segfault
        m_changePasswordDialog->accept();
      }));
    } else {
      createChangePasswordDialog();
      submitButton->setText("Update");
      cancelButton->setText("Delete");
      cancelButton->setStyleClass("btn-danger");
      submitButton->clicked().connect(std::bind([=](){
        m_model->setWritable(true);
        updateView(m_model);
      }));
      cancelButton->clicked().connect(std::bind([=](){
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
      }));
    }
  } else {
    submitButton->setStyleClass("btn-success");
    cancelButton->clicked().connect(std::bind([=]() {
      m_infoBox->setText("");
      m_model->reset();
      updateView(m_model);
      refresh();
    }));
  }
  bindString("title", title);
  submitButton->clicked().connect(this, &UserFormView::process);
  updateView(m_model);
}

UserFormView::~UserFormView(void)
{
  delete m_infoBox;
}

void UserFormView::process(void)
{
  updateModel(m_model);
  bool isvalid = m_model->validate();
  updateView(m_model);
  if (isvalid) {
    if (m_changePassword) {
      m_changePasswordTrigerred.emit(m_user.username, m_model->valueText(UserFormModel::PasswordField).toUTF8());
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
  m_changePasswordDialog = new Wt::WDialog("Change password");
  UserFormView* changedPasswdForm = new UserFormView(&m_user, true);
  m_changePasswordDialog->contents()->addWidget(changedPasswdForm);
}

UserMngtUI::UserMngtUI(DbSession* dbSession, Wt::WContainerWidget* parent)
  : Wt::WScrollArea(parent),
    m_dbSession(dbSession),
    m_userForm(new UserFormView(NULL, false)),
    m_userListContainer(new Wt::WContainerWidget()),
    m_contents(new Wt::WStackedWidget(0)),
    m_menu(new Wt::WMenu(m_contents, Wt::Vertical, 0))
{
  m_dbSession->updateUserList();
  m_menu->setStyleClass("nav nav-pills");
  m_userForm->validated().connect(std::bind([=](User user) {
    int ret = m_dbSession->addUser(user);
    m_userForm->showMessage(ret,
                            "Operation failed. More details in log.",
                            "User added.");
  }, std::placeholders::_1));

  Wt::WMenuItem* item = m_menu->addItem("Add User", m_userForm);
  m_menus.insert(std::pair<int, Wt::WMenuItem*>(AddUserAction, item));
  item = m_menu->addItem("User List", createUserList());
  item->triggered().connect(std::bind([=](){
    updateUserList();
  }));
  m_menus.insert(std::pair<int, Wt::WMenuItem*>(ListUserAction, item));

  this->setWidget(createMainUI());

  addJsEventScript();
}

UserMngtUI::~UserMngtUI(void)
{
  delete m_userForm;
  delete m_userListContainer;
  delete m_contents;
  delete m_menu;
}

void UserMngtUI::updateUserList(void)
{
  m_userListContainer->clear();
  for (auto user: m_dbSession->getUserList()) {
    m_userListContainer->addWidget(createUserPanel(user));
  }
}

Wt::WPanel* UserMngtUI::createUserPanel(const User& user)
{
  Wt::WAnimation animation(Wt::WAnimation::SlideInFromTop,
                           Wt::WAnimation::EaseOut, 100);

  UserFormView* userForm(new UserFormView(&user, false));
  userForm->validated().connect(std::bind([=](User userToUpdate) {
    int ret = m_dbSession->updateUser(userToUpdate);
    userForm->showMessage(ret,
                          "Update failed. More details in log.",
                          "Update completed.");
  }, std::placeholders::_1));

  userForm->deleteTriggered().connect(std::bind([=](std::string username) {
    int ret = m_dbSession->deleteUser(username);
    m_userForm->showMessage(ret,
                            "Deletion failed. More details in log.",
                            QObject::tr("User %1 deleted").arg(username.c_str()).toStdString());
    updateUserList();
  }, std::placeholders::_1));

  Wt::WPanel *panel(new Wt::WPanel());
  panel->setAnimation(animation);
  panel->setCentralWidget(userForm);
  panel->setTitle(user.username);
  panel->setCollapsible(true);
  panel->setCollapsed(true);
  return panel;
}

void UserMngtUI::showDestinationView(int dest)
{
  m_menu->select(m_menus[dest]);
}

void UserMngtUI::addJsEventScript(void)
{
  this->setJavaScriptMember("wtResize", JS_AUTO_RESIZING_FUNCTION);
  wApp->root()->doJavaScript(JS_AUTO_RESIZING_SCRIPT("wh=$(window).height();"));
}

Wt::WWidget* UserMngtUI::createUserList(void)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("user-list-tpl"));
  tpl->bindString("title", "User list");
  tpl->bindWidget("user-list", m_userListContainer);
  return tpl;
}

Wt::WWidget* UserMngtUI::createMainUI(void)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("user-mgnt-tpl"));
  tpl->bindWidget("menu", m_menu);
  tpl->bindWidget("contents", m_contents);
  return tpl;
}
