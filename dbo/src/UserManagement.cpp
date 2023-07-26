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

#include "UserManagement.hpp"
#include "DbSession.hpp"
#include "WebUtils.hpp"
#include "WebBaseSettings.hpp"
#include <QObject>
#include <Wt/WMenu.h>
#include <Wt/WPanel.h>
#include <Wt/WComboBox.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/WMessageBox.h>
#include <Wt/WApplication.h>
#include <Wt/WTemplate.h>
#include <Wt/WString.h>
#include <Wt/WImage.h>
#include <Wt/WSpinBox.h>


ConfirmPasswordValidator::ConfirmPasswordValidator(UserFormModel* model, Wt::WFormModel::Field passField)
  : Wt::WValidator(),
    m_model(model),
    m_passwordField(passField)
{

}

Wt::WValidator::Result ConfirmPasswordValidator::validate(const Wt::WString &input) const
{
  return (m_model->valueText(m_passwordField) == input)?
        Wt::WValidator::Result(Wt::ValidationState::Valid):
        Wt::WValidator::Result(Wt::ValidationState::Invalid, Q_TR("Confirmation don't match"));
}

UserFormModel::UserFormModel(const DboUserT* user, bool changePassword, bool userForm)
  : Wt::WFormModel(),
    m_userForm(userForm)
{
  addField(UsernameField);
  addField(PasswordField);
  addField(CurrentPasswordField);
  addField(PasswordConfirmationField);
  addField(FirstNameField);
  addField(LastNameField);
  addField(EmailField);
  addField(UserLevelField);
  addField(RegistrationDateField);
  addField(OperationsProfileMode);
  addField(OperationsProfileTilesPerRow);

  setValidator(UsernameField, createNameValidator());;
  setValidator(PasswordField, createPasswordValidator());
  setValidator(PasswordConfirmationField, std::make_unique<ConfirmPasswordValidator>(this, PasswordField));
  setValidator(FirstNameField, createNameValidator());
  setValidator(FirstNameField, createNameValidator());
  setValidator(LastNameField, createNameValidator());
  setValidator(EmailField, createEmailValidator());
  setValidator(UserLevelField, createNameValidator());

  // These fields should always be read only on update
  if (user && ! changePassword) {
    setReadOnly(UsernameField, true);
    setReadOnly(PasswordField, true);
    setReadOnly(RegistrationDateField, true);
  }

  if (changePassword) {
    setVisible(CurrentPasswordField, true);
    setVisible(PasswordField, true);
    setVisible(PasswordConfirmationField, true);
    setVisible(UsernameField, false);
    setVisible(FirstNameField, false);
    setVisible(LastNameField, false);
    setVisible(EmailField, false);
    setVisible(UserLevelField, false);
    setVisible(RegistrationDateField, false);
    setVisible(OperationsProfileMode, false);
    setVisible(OperationsProfileTilesPerRow, false);
  } else {
    setVisible(CurrentPasswordField, false);
    if (user) {
      setData(*user);
      setVisible(PasswordField, false);
      setVisible(PasswordConfirmationField, false);
      setWritable(false);
    } else {
      setVisible(RegistrationDateField, false);
    }
  }
}

void UserFormModel::setWritable(bool writtable)
{
  bool readonly = ! writtable;
  setReadOnly(PasswordConfirmationField, readonly);
  setReadOnly(FirstNameField, readonly);
  setReadOnly(LastNameField, readonly);
  setReadOnly(EmailField, readonly);
  setReadOnly(OperationsProfileMode, readonly);
  setReadOnly(OperationsProfileTilesPerRow, readonly);
  if (readonly) {
    setReadOnly(UserLevelField, readonly);
  } else {
    setReadOnly(UserLevelField, m_userForm);
  }
}

void UserFormModel::setData(const DboUserT & user)
{
  setValue(UsernameField, user.username);
  setValue(FirstNameField, user.firstname);
  setValue(LastNameField, user.lastname);
  setValue(EmailField, user.email);
  setValue(UserLevelField, DboUser::role2Text(user.role));
  setValue(OperationsProfileMode, DboUser::dashboardMode2Text(user.opsProfileMode));
  setValue(OperationsProfileTilesPerRow, user.opsProfileTilesPerRow);
  setValue(RegistrationDateField, user.registrationDate);
}

std::unique_ptr<Wt::WValidator> UserFormModel::createNameValidator(void)
{
  auto validator = std::make_unique<Wt::WLengthValidator>();
  validator->setInvalidBlankText(Q_TR("Required field"));
  validator->setMandatory(true);
  validator->setMinimumLength(1);
  validator->setMaximumLength(MAX_LENGTH);
  return validator;
}

std::unique_ptr<Wt::WValidator> UserFormModel::createPasswordValidator(void)
{
  auto validator = std::make_unique<Wt::Auth::PasswordStrengthValidator>();
  validator->setMinimumLength(Wt::Auth::PasswordStrengthType::TwoCharClass, 6);
  validator->setMandatory(true);
  return validator;
}

std::unique_ptr<Wt::WValidator> UserFormModel::createEmailValidator(void)
{
  auto validator = std::make_unique<Wt::WRegExpValidator>("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,4}");
  validator->setInvalidBlankText(Q_TR("Required field"));
  validator->setInvalidBlankText(Q_TR("Invalid email"));
  validator->setMandatory(true);
  return validator;
}

std::unique_ptr<Wt::WValidator> UserFormModel::createConfirmPasswordValidator(void)
{
  return createPasswordValidator();
}

UserFormView::UserFormView(const DboUserT* user, bool changePasswordTriggered, bool userForm)
  : m_changePassword(changePasswordTriggered),
    m_infoBox(new Wt::WText(""))
{
  m_modelRef = new UserFormModel(user, changePasswordTriggered, userForm);

  setTemplateText(tr("userForm-template"));
  addFunction("id", &WTemplate::Functions::id);
  setFormWidget(UserFormModel::UsernameField, std::make_unique<Wt::WLineEdit>());
  setFormWidget(UserFormModel::CurrentPasswordField, std::move(createPasswordField()));
  setFormWidget(UserFormModel::PasswordField, std::move(createPasswordField()));
  setFormWidget(UserFormModel::PasswordConfirmationField, std::move(createPasswordField()));
  setFormWidget(UserFormModel::FirstNameField, std::make_unique<Wt::WLineEdit>());

  auto lastNameField = std::make_unique<Wt::WLineEdit>();
  m_lastNameFieldRef = lastNameField.get();
  setFormWidget(UserFormModel::LastNameField, std::move(lastNameField));
  setFormWidget(UserFormModel::EmailField, std::make_unique<Wt::WLineEdit>());
  setFormWidget(UserFormModel::UserLevelField, std::move(createUserRoleField()));
  setFormWidget(UserFormModel::RegistrationDateField, std::make_unique<Wt::WLineEdit>());


  auto opsProfileModeField = createDashboardDisplayModeField();
  m_opsProfileModeFieldRef = opsProfileModeField.get();
  setFormWidget(UserFormModel::OperationsProfileMode, std::move(opsProfileModeField));

  auto opsProfileTilesPerRowField = createDashboardTilesPerRowField();
  m_opsProfileTilesPerRowFieldRef = opsProfileTilesPerRowField.get();
  setFormWidget(UserFormModel::OperationsProfileTilesPerRow, std::move(opsProfileTilesPerRowField));

  Wt::WPushButton* changedPwdButtonRef = nullptr;
  if (user) {
    m_user = *user;
    if (m_changePassword) {
      bindString("change-password-link", "");
    } else {
      if (! userForm) {
        bindEmpty("title");
        changedPwdButtonRef = bindNew<Wt::WPushButton>("change-password-link");
        changedPwdButtonRef->setText(Q_TR("Change password"));
        changedPwdButtonRef->setStyleClass("btn btn-warning");
        changedPwdButtonRef->clicked().connect(std::bind([=, &user]() { m_changePasswordDialog->show();}));
      } else {
        bindEmpty("change-password-link");
      }
    }
  } else {
    bindEmpty("change-password-link");
  }

  // Bind buttons, but alter later
  auto submitButton = std::make_unique<Wt::WPushButton>(Q_TR("Submit") );
  auto cancelButton = std::make_unique<Wt::WPushButton>(Q_TR("Clear") );
  Wt::WPushButton* submitButtonRef = submitButton.get();
  Wt::WPushButton* cancelButtonRef = cancelButton.get();

  Wt::WString title = Wt::WString( Q_TR("User information") );
  if (user) {
    submitButtonRef->setStyleClass("btn btn-success");
    if (changePasswordTriggered) {
      title = Wt::WString( Q_TR("Set password information") );
      submitButtonRef->setText( Q_TR("Change password") );
      cancelButtonRef->setText( Q_TR("Close") );
      //FIXED me uncomment connect
      cancelButtonRef->clicked().connect(this, &UserFormView::handleCancelClick);
    } else {
      createChangePasswordDialog();
      submitButtonRef->setText( Q_TR("Update") );
      if (! userForm) {
        cancelButtonRef->setText( Q_TR("Delete") );
        cancelButtonRef->setStyleClass("btn btn-danger");
        cancelButtonRef->clicked().connect(this, &UserFormView::handleDeleteClick);
      } else {
        cancelButtonRef->setText( Q_TR("Close") );
        cancelButtonRef->clicked().connect(this, &UserFormView::handleCloseClick);
        cancelButtonRef->hide();
      }
    }
  } else {
    submitButtonRef->setStyleClass("btn btn-success");
    cancelButtonRef->clicked().connect(std::bind([=]() {m_modelRef->reset(); updateView(m_modelRef);}));
  }

  // If user is not null,  it means it's an update request.
  // By default all the fields are disabled.
  if (user && ! changePasswordTriggered) {
    submitButtonRef->clicked().connect(std::bind([=](){
      setWritable(true);
      submitButtonRef->clicked().connect(this, &UserFormView::process);
    }));
  } else {
    submitButtonRef->clicked().connect(this, &UserFormView::process);
  }

  if (user && user->authsystem == WebBaseSettings::LDAP) {
    if (submitButtonRef) {
      submitButtonRef->setDisabled(true);
    }
    if (cancelButtonRef) {
      cancelButtonRef->setDisabled(true);
    }
    if (changedPwdButtonRef) {
      changedPwdButtonRef->setDisabled(true);
    }
  }

  bindWidget("submit-button", std::move(submitButton));
  bindWidget("cancel-button", std::move(cancelButton));
  bindString("title", title);
  updateView(m_modelRef);
}

UserFormView::~UserFormView(void)
{
}

void UserFormView::reset(void)
{
  m_modelRef->reset();
  updateView(m_modelRef);
}

void UserFormView::setWritable(bool writtable)
{
  m_modelRef->setWritable(writtable);
  updateView(m_modelRef);
}

void UserFormView::resetValidationState(bool writtable)
{
  m_modelRef->reset();
  m_modelRef->setData(m_user);
  setWritable(writtable);
}

void UserFormView::process(void)
{
  updateModel(m_modelRef);
  bool isValid = m_modelRef->validate();
  updateView(m_modelRef);
  if (isValid) {
    if (m_changePassword) {
      m_changePasswordTriggered.emit(m_user.username,
                                     m_modelRef->valueText(UserFormModel::CurrentPasswordField).toUTF8(),
                                     m_modelRef->valueText(UserFormModel::PasswordField).toUTF8()
                                     );
    } else {
      m_user.username = m_modelRef->valueText(UserFormModel::UsernameField).toUTF8();
      m_user.password = m_modelRef->valueText(UserFormModel::PasswordField).toUTF8();
      m_user.firstname = m_modelRef->valueText(UserFormModel::FirstNameField).toUTF8();
      m_user.lastname = m_modelRef->valueText(UserFormModel::LastNameField).toUTF8();
      m_user.email = m_modelRef->valueText(UserFormModel::EmailField).toUTF8();
      m_user.role = DboUser::role2Int(m_modelRef->valueText(UserFormModel::UserLevelField).toUTF8());
      m_user.opsProfileMode = m_opsProfileModeFieldRef->currentIndex();
      m_user.opsProfileTilesPerRow = m_opsProfileTilesPerRowFieldRef->value();
      m_user.registrationDate = Wt::WDateTime::currentDateTime().toString().toUTF8();
      m_validated.emit(m_user);
    }
  }
}


void UserFormView::handleDeleteClick(void)
{
  auto confirmationBox = std::make_shared<Wt::WMessageBox>("Warning !", "<p>Do you really want to delete this user?</p>",
                                                           Wt::Icon::Information, Wt::StandardButton::Yes | Wt::StandardButton::No);
  confirmationBox->setModal(false);
  confirmationBox->buttonClicked().connect(std::bind([=] () {
    if (confirmationBox->buttonResult() == Wt::StandardButton::Yes) {
      m_deleteTriggered.emit(m_modelRef->valueText(UserFormModel::UsernameField).toUTF8());
    }
  }));
  confirmationBox->show();
}

void UserFormView::handleCloseClick(Wt::WMouseEvent ev)
{
  m_close.emit(ev);
}

void UserFormView::handleCancelClick(Wt::WMouseEvent ev)
{
  m_close.emit(ev);
}


void UserFormView::handleChangePasswordClick(const std::string& login, const std::string& currentPass,const std::string& newPass)
{
  m_changePasswordTriggered.emit(login, currentPass, newPass);
  m_changePasswordDialog->accept();
}

std::unique_ptr<Wt::WComboBox> UserFormView::createUserRoleField(void)
{
  auto fieldModel = std::make_shared<Wt::WStandardItemModel>(2, 1);

  auto opRoleItem = std::make_unique<Wt::WStandardItem>(DboUser::role2Text(DboUser::OpRole));
  opRoleItem->setData(DboUser::OpRole, Wt::ItemDataRole::User);
  fieldModel->setItem(0, 0, std::move(opRoleItem));

  auto admRoleItem = std::make_unique<Wt::WStandardItem>(DboUser::role2Text(DboUser::AdmRole));
  admRoleItem->setData(DboUser::AdmRole, Wt::ItemDataRole::User);
  fieldModel->setItem(1, 0, std::move(admRoleItem));

  auto fieldWidget = std::make_unique<Wt::WComboBox>();
  fieldWidget->setModel(fieldModel);
  return fieldWidget;
}


std::unique_ptr<Wt::WLineEdit> UserFormView::createPasswordField(void)
{
  auto field = std::make_unique<Wt::WLineEdit>();
  field->setEchoMode(Wt::EchoMode::Password);
  return field;
}


void UserFormView::createChangePasswordDialog(void)
{
  bool changedPasswd(true);
  bool forUserProfile(false);
  m_changePasswordDialog = new Wt::WDialog( Q_TR("Change password") );
  m_changePasswordDialog->setStyleClass("Wt-dialog");
  auto changedPasswdForm = std::make_unique<UserFormView>(&m_user, changedPasswd, forUserProfile);
  changedPasswdForm->changePasswordTriggered().connect(this, &UserFormView::handleChangePasswordClick);
  changedPasswdForm->closeTriggered().connect(std::bind([=](){m_changePasswordDialog->accept();}));
  m_changePasswordDialog->contents()->addWidget(std::move(changedPasswdForm));
}

std::unique_ptr<Wt::WComboBox> UserFormView::createDashboardDisplayModeField(void)
{
  auto model = std::make_shared<Wt::WStandardItemModel>(2, 1);

  auto opsProfileTacticalViewOnly = std::make_unique<Wt::WStandardItem>(DboUser::dashboardMode2Text(DboUser::OperationsProfileTacticalViewOnly));
  opsProfileTacticalViewOnly->setData(DboUser::OperationsProfileTacticalViewOnly, Wt::ItemDataRole::User);
  model->setItem(DboUser::OperationsProfileTacticalViewOnly, 0, std::move(opsProfileTacticalViewOnly));

  auto opsProfileModeFull = std::make_unique<Wt::WStandardItem>(DboUser::dashboardMode2Text(DboUser::OperationsProfileFull));
  opsProfileModeFull->setData(DboUser::OperationsProfileFull, Wt::ItemDataRole::User);
  model->setItem(DboUser::OperationsProfileFull, 0, std::move(opsProfileModeFull));

  auto opsProfileNoAnalytics = std::make_unique<Wt::WStandardItem>(DboUser::dashboardMode2Text(DboUser::OperationsProfileNoAnalytics));
  opsProfileNoAnalytics->setData(DboUser::OperationsProfileNoAnalytics, Wt::ItemDataRole::User);
  model->setItem(DboUser::OperationsProfileNoAnalytics, 0, std::move(opsProfileNoAnalytics));

  auto fieldWidget = std::make_unique<Wt::WComboBox>();
  fieldWidget->setModel(model);

  return fieldWidget;
}

std::unique_ptr<Wt::WSpinBox> UserFormView::createDashboardTilesPerRowField(void)
{
  auto spinbox = std::make_unique<Wt::WSpinBox>();
  spinbox->setMinimum(1);
  spinbox->setMaximum(16);
  spinbox->setValue(5);
  return spinbox;
}


DbUserManager::DbUserManager(DbSession* dbSession)
  : m_dbSession(dbSession),
    m_contents(new Wt::WStackedWidget())
{
  m_userForm = std::make_unique<UserFormView>(nullptr, false, false);
  m_userFormRef = m_userForm.get();
  m_dbUserListWidget = std::make_unique<Wt::WTemplate>(Wt::WString::tr("user-list-tpl"));
  m_dbUserListWidgetRef =  m_dbUserListWidget.get();
  setOverflow(Wt::Overflow::Scroll);
  m_dbUserListWidgetRef->bindString("title", Q_TR("User list"));
  m_userFormRef->validated().connect(this, &DbUserManager::handleFormAddUser);
}

DbUserManager::~DbUserManager(void){}


void DbUserManager::updateDbUsers(void)
{
  m_dbUserListWidgetRef->bindWidget("user-list", nullptr);
  auto usersListContainer = std::make_unique<Wt::WContainerWidget>();
  for (auto& user: m_dbSession->listUsers()) {
    usersListContainer->addWidget(createUserPanel(user.data()));
  }
  m_dbUserListWidgetRef->bindWidget("user-list", std::move(usersListContainer));
}


std::unique_ptr<Wt::WPanel> DbUserManager::createUserPanel(const DboUserT& user)
{
  bool changePassword(false);
  bool userForm(false);
  auto form = std::make_unique<UserFormView>(&user,  changePassword, userForm);
  form->validated().connect(this, &DbUserManager::handleFormUpdateUser);
  form->changePasswordTriggered().connect(this, &DbUserManager::handleFormChangePassword);
  form->deleteTriggered().connect(this, &DbUserManager::handleDeleteUser);
  auto panel = std::make_unique<Wt::WPanel>();
  panel->setTitle(Wt::WString("{1} ({2})").arg(user.username).arg(WebBaseSettings::authTypeString(user.authsystem)));
  panel->setAnimation(Wt::WAnimation(Wt::AnimationEffect::SlideInFromTop,  Wt::TimingFunction::EaseOut, 100));
  panel->setCentralWidget(std::move(form));
  panel->setCollapsible(true);
  panel->setCollapsed(true);
  return panel;
}

void DbUserManager::handleFormAddUser(DboUserT dboUser)
{
  auto addUserOut = m_dbSession->addUser(dboUser);
  m_updateCompleted.emit(addUserOut.first);
}


void DbUserManager::handleFormUpdateUser(DboUserT dboUser)
{
  auto updateUserOut = m_dbSession->updateUser(dboUser);
  m_updateCompleted.emit(updateUserOut.first);
}


void DbUserManager::handleDeleteUser(std::string username)
{
  m_updateCompleted.emit(m_dbSession->deleteUser(username));
  updateDbUsers();
}


void DbUserManager::handleFormChangePassword(std::string username, std::string oldPass, std::string newPass)
{
  auto updatePasswordOut = m_dbSession->updatePassword(username, oldPass, newPass);
  m_updateCompleted.emit(updatePasswordOut.first);
}


