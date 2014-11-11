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
#include "WebPreferences.hpp"
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
#include <Wt/WImage>


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
        Wt::WValidator::Result(Wt::WValidator::Invalid, Q_TR("Confirmation don't match"));
}

UserFormModel::UserFormModel(const DbUserT* user, bool changePassword, bool userForm, Wt::WObject *parent)
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
      setData(*user);
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

void UserFormModel::setData(const DbUserT& user)
{
  setValue(UsernameField, user.username);
  setValue(FirstNameField, user.firstname);
  setValue(LastNameField, user.lastname);
  setValue(EmailField, user.email);
  setValue(UserLevelField, DbUserT::role2Text(user.role));
  setValue(RegistrationDateField, user.registrationDate);
}

Wt::WValidator* UserFormModel::createNameValidator(void)
{
  Wt::WLengthValidator* validator = new Wt::WLengthValidator();
  validator->setInvalidBlankText(Q_TR("Required field"));
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
  validator->setInvalidBlankText(Q_TR("Required field"));
  validator->setInvalidBlankText(Q_TR("Invalid email"));
  validator->setMandatory(true);
  return validator;
}

Wt::WValidator* UserFormModel::createConfirmPasswordValidator(void)
{
  return createPasswordValidator();
}

UserFormView::UserFormView(const DbUserT* user, bool changePassword, bool userForm)
  : m_changePassword(changePassword),
    m_infoBox(new Wt::WText("")),
    m_validated(this),
    m_deleteTriggered(this),
    m_changePasswordTriggered(this)
{
  m_model = new UserFormModel(user, changePassword, userForm, this);

  Wt::WPushButton* changedPwdButton = NULL;
  Wt::WPushButton* submitButton = NULL;
  Wt::WPushButton* cancelButton = NULL;

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
        changedPwdButton = new Wt::WPushButton(Q_TR("Change password"));
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
  submitButton = new Wt::WPushButton( Q_TR("Submit") );
  cancelButton = new Wt::WPushButton( Q_TR("Clear") );
  bindWidget("submit-button", submitButton);
  bindWidget("cancel-button", cancelButton);

  Wt::WString title = Wt::WString( Q_TR("User information") );
  if (user) {
    submitButton->setStyleClass("btn btn-success");
    if (changePassword) {
      title = Wt::WString( Q_TR("Set password information") );
      submitButton->setText( Q_TR("Change password") );
      cancelButton->setText( Q_TR("Close") );
      cancelButton->clicked().connect(std::bind([=](){m_close.emit();}));
    } else {
      createChangePasswordDialog();
      submitButton->setText( Q_TR("Update") );
      if (! userForm) {
        cancelButton->setText( Q_TR("Delete") );
        cancelButton->setStyleClass("btn btn-danger");
        cancelButton->clicked().connect(this, &UserFormView::handleDeleteRequest);
      } else {
        cancelButton->setText( Q_TR("Close") );
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
      setWritable(true);
      submitButton->clicked().connect(this, &UserFormView::process);
    }));
  } else {
    submitButton->clicked().connect(this, &UserFormView::process);
  }

  if (user && user->authsystem == WebPreferences::LDAP) {
    if (submitButton) submitButton->setDisabled(true);
    if (cancelButton) cancelButton->setDisabled(true);
    if (changedPwdButton) changedPwdButton->setDisabled(true);
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

void UserFormView::setWritable(bool writtable)
{
  m_model->setWritable(writtable);
  updateView(m_model);
}

void UserFormView::resetValidationState(bool writtable)
{
  m_model->reset();
  m_model->setData(m_user);
  setWritable(writtable);
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
      m_user.role = DbUserT::role2Int(m_model->valueText(UserFormModel::UserLevelField).toUTF8());
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
  Wt::WStandardItem* item = new Wt::WStandardItem(DbUserT::role2Text(DbUserT::OpRole));
  item->setData(DbUserT::OpRole, Wt::UserRole);
  roleModel->setItem(0, 0, item);

  item = new Wt::WStandardItem(DbUserT::role2Text(DbUserT::AdmRole));
  item->setData(DbUserT::AdmRole, Wt::UserRole);
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
  m_changePasswordDialog = new Wt::WDialog( Q_TR("Change password") );
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

DbUserManager::DbUserManager(DbSession* dbSession)
  : m_updateCompleted(this),
    m_dbSession(dbSession),
    m_userForm(new UserFormView(NULL, false, false)),
    m_usersListContainer(new Wt::WContainerWidget()),
    m_contents(new Wt::WStackedWidget(0)),
    m_dbUserListWidget(new Wt::WTemplate(Wt::WString::tr("user-list-tpl")))
{
  m_dbUserListWidget->bindString("title", Q_TR("User list"));
  m_dbUserListWidget->bindWidget("user-list", m_usersListContainer);
  m_userForm->validated().connect(std::bind([=](DbUserT user) { m_updateCompleted.emit(m_dbSession->addUser(user));}, std::placeholders::_1));
}

DbUserManager::~DbUserManager(void)
{
  delete m_userForm;
  delete m_usersListContainer;
}


void DbUserManager::updateDbUsers(void)
{
  m_usersListContainer->clear();
  m_dbSession->updateUserList();
  for (auto user: m_dbSession->userList()) {
    m_usersListContainer->addWidget( createUserPanel(user) );
  }
}


Wt::WPanel* DbUserManager::createUserPanel(const DbUserT& user)
{
  bool changePassword(false);
  bool userForm(false);
  Wt::WAnimation animation(Wt::WAnimation::SlideInFromTop,
                           Wt::WAnimation::EaseOut, 100);

  UserFormView* form(new UserFormView(&user,  changePassword, userForm));

  // connect signal for add user
  form->validated().connect(std::bind([=](DbUserT userToUpdate) {
    m_dbSession->updateUser(userToUpdate);
    m_updateCompleted.emit(m_dbSession->updateUser(userToUpdate));
  }, std::placeholders::_1));

  // connect signal for change password
  form->changePasswordTriggered().connect(std::bind([=](const std::string& login,
                                                    const std::string& currentPass,
                                                    const std::string& newPass) {
    m_updateCompleted.emit(m_dbSession->updatePassword(login, currentPass, newPass));
  }, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  // connect signal for delete user
  form->deleteTriggered().connect(std::bind([=](std::string username) {
    m_updateCompleted.emit(m_dbSession->deleteUser(username));
    updateDbUsers();
  }, std::placeholders::_1));

  Wt::WPanel *panel(new Wt::WPanel());
  panel->setTitle(Wt::WString("{1} ({2})")
                  .arg(user.username)
                  .arg(WebPreferences::authTypeString(user.authsystem)));
  panel->setAnimation(animation);
  panel->setCentralWidget(form);
  panel->setCollapsible(true);
  panel->setCollapsed(true);

  return panel;
}


/**
 * @brief LdapUserTable::LdapUserTable
 * @param parent
 */
LdapUserManager::LdapUserManager(DbSession* dbSession, Wt::WContainerWidget* parent)
  : Wt::WTableView(parent),
    m_userEnableStatusChanged(this),
    m_model(new Wt::WStandardItemModel(0, 5, this)),
    m_dbSession(dbSession)
{
  setSortingEnabled(true);
  setLayoutSizeAware(true);
  setColumnResizeEnabled(true);
  setSelectable(true);
  setSelectionMode(Wt::SingleSelection);
  setSelectionBehavior(Wt::SelectRows);
  setHeaderHeight(26);
  setAlternatingRowColors(true);

  setModelHeader();
  setModel(m_model);
  addEvent();
}

/**
 * @brief Add signa/slot event handling
 */
void LdapUserManager::addEvent()
{
  m_model->itemChanged().connect(this, &LdapUserManager::handleImportationAction);
}

/**
 * @brief Set the table view header
 */
void LdapUserManager::setModelHeader(void)
{
  m_model->setHeaderData(0, Q_TR("DN"));
  m_model->setHeaderData(1, Q_TR("Full Name"));
  m_model->setHeaderData(2, Q_TR("UID"));
  m_model->setHeaderData(3, Q_TR("Email"));
  m_model->setHeaderData(4, Q_TR("Enable Auth"));
}

/**
 * @brief LdapUserTable::updateLdapUsers
 * @return
 */
int LdapUserManager::updateUserList(void)
{
  setDisabled(true);
  WebPreferences preferences;
  m_ldapUidField = preferences.getLdapIdField();
  LdapHelper ldapHelper(preferences.getLdapServerUri(),
                        preferences.getLdapVersion(),
                        preferences.getLdapSslUseMyCert(),
                        preferences.getLdapSslCertFile(),
                        preferences.getLdapSslCaFile());

  std::string filter = "(objectClass=person)";
  m_users.clear();
  int count = ldapHelper.listUsers(preferences.getLdapSearchBase(),
                                   preferences.getLdapBindUserDn(),
                                   preferences.getLdapBindUserPassword(),
                                   filter,
                                   m_users);
  m_model->clear();
  if (count <= 0) {
    m_lastError = ldapHelper.lastError();
  } else {
    for (const auto& userInfo : m_users) {
      DbUserT dbUserInfo;
      bool imported = m_dbSession->findUser(userInfo[m_ldapUidField], dbUserInfo);
      addUserRow(userInfo, imported);
    }
    setModelHeader();
  }
  setDisabled(false);
  return count;
}


void LdapUserManager::addUserRow(const LdapUserAttrsT& userInfo, bool imported)
{
  int row = m_model->rowCount();
  std::string dn = userInfo["dn"];
  m_model->setItem(row, 0, createEntryItem(dn, dn));
  m_model->setItem(row, 1, createEntryItem(userInfo["cn"], dn));
  m_model->setItem(row, 2, createEntryItem(userInfo[m_ldapUidField], dn));
  m_model->setItem(row, 3, createEntryItem(userInfo["mail"], dn));
  m_model->setItem(row, 4, createImportationItem(dn, imported));
}

Wt::WStandardItem* LdapUserManager::createEntryItem(const std::string& text, const std::string& data)
{
  Wt::WStandardItem* item = new Wt::WStandardItem(text);
  item->setData(data, Wt::UserRole);
  return item;
}

Wt::WStandardItem* LdapUserManager::createImportationItem(const std::string& data, bool alreadyImported)
{
  Wt::WStandardItem* item = createEntryItem("", data);
  item->setCheckable(true);
  item->setChecked(alreadyImported);
  return item;
}


void LdapUserManager::handleImportationAction(Wt::WStandardItem* item)
{
  if (item->isCheckable()) {
    std::string ldapDn = getItemData(item);
    LdapUserMapT::ConstIterator userInfo =  m_users.find(ldapDn);
    if (userInfo != m_users.end()) {
      std::string username = (*userInfo)[m_ldapUidField];
      if (item->checkState() == Wt::Checked) { // enable LDAP authentication
        if (insertIntoDatabase(*userInfo) != 0) {
          updateUserList();
        }
      } else { // disable LDAP authentication
        if (m_dbSession->deleteUser(username) == 0) {
          m_userEnableStatusChanged.emit(DisableAuthSuccess, username);
        } else {
          m_userEnableStatusChanged.emit(GenericError, m_dbSession->lastError());
        }
      }
    } else {
      m_userEnableStatusChanged.emit(GenericError,
                                     Q_TR("User DN not found in the directory: ")+ldapDn);
    }
  }
}


std::string LdapUserManager::getItemData(Wt::WStandardItem* item)
{
  std::string data;
  try {
    data = boost::any_cast<std::string>(item->data(Wt::UserRole));
  } catch(...) {
    data = "";
  }

  return data;
}


int LdapUserManager::insertIntoDatabase(const LdapUserAttrsT& userInfo)
{
  int retCode = -1;
  DbUserT dbUser;
  dbUser.username = userInfo[m_ldapUidField];

  if (dbUser.username.empty()) {
    m_userEnableStatusChanged.emit(GenericError, Q_TR("The ID attribute is empty: ")+m_ldapUidField);
    return retCode;
  }

  dbUser.password = userInfo["userpassword"];
  dbUser.email = userInfo["mail"];
  dbUser.firstname = userInfo["gn"];
  dbUser.lastname = userInfo["sn"];
  dbUser.role = DbUserT::OpRole;
  dbUser.authsystem = WebPreferences::LDAP;

  if (m_dbSession->addUser(dbUser) == 0) {
    m_userEnableStatusChanged.emit(EnableAuthSuccess, dbUser.username);
    retCode = 0;
  } else {
    m_userEnableStatusChanged.emit(GenericError, m_dbSession->lastError());
  }

  return retCode;
}
