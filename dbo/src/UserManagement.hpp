/*
 * UserForm.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
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

#ifndef USERFORM_HPP
#define USERFORM_HPP

#include "DbObjects.hpp"
#include "LdapHelper.hpp"
#include <Wt/WDateEdit.h>
#include <Wt/WDateValidator.h>
#include <Wt/WFormModel.h>
#include <Wt/WIntValidator.h>
#include <Wt/WLengthValidator.h>
#include <Wt/WModelIndex.h>
#include <Wt/WPushButton.h>
#include <Wt/WString.h>
#include <Wt/WTemplateFormView.h>
#include <Wt/WValidator.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WRegExpValidator.h>
#include <Wt/WSignal.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTableView.h>

class DbSession;
class UserFormModel;
class UserFormView;
class ConfirmPasswordValidator;
class UserFormModel;
class UserFormView;
class DbUserManager;
class DbUserTable;

class ConfirmPasswordValidator : public Wt::WValidator
{
public:
  ConfirmPasswordValidator(UserFormModel* model, Wt::WFormModel::Field passField);

protected:
  virtual Wt::WValidator::Result validate(const Wt::WString &input) const;

private:
  UserFormModel* m_model;
  Wt::WFormModel::Field m_passwordField;
};

class UserFormModel : public Wt::WFormModel
{
public:
  static constexpr Wt::WFormModel::Field InfoBoxField = "info-box";
  static constexpr Wt::WFormModel::Field UsernameField = "user-name";
  static constexpr Wt::WFormModel::Field CurrentPasswordField = "current-password";
  static constexpr Wt::WFormModel::Field PasswordField = "password";
  static constexpr Wt::WFormModel::Field PasswordConfirmationField = "confirm-password";
  static constexpr Wt::WFormModel::Field FirstNameField = "first-name";
  static constexpr Wt::WFormModel::Field LastNameField = "last-name";
  static constexpr Wt::WFormModel::Field EmailField = "email";
  static constexpr Wt::WFormModel::Field UserLevelField = "role";
  static constexpr Wt::WFormModel::Field RegistrationDateField = "registration-date";
  static constexpr Wt::WFormModel::Field OperationsProfileMode = "dashboard-mode";
  static constexpr Wt::WFormModel::Field OperationsProfileTilesPerRow = "dashboard-tilesperrow";

  UserFormModel(const DboUserT* user, bool changePassword, bool userForm);
  void setWritable(bool writable);
  void setData(const DboUserT & user);

private:
  static const int MAX_LENGTH = 25;
  static const int MAX_CHILDREN = 15;
  bool m_userForm;

  std::unique_ptr<Wt::WValidator> createNameValidator(void);
  std::unique_ptr<Wt::WValidator> createEmailValidator(void);
  std::unique_ptr<Wt::WValidator> createPasswordValidator(void);
  std::unique_ptr<Wt::WValidator> createConfirmPasswordValidator(void);

};

class UserFormView : public Wt::WTemplateFormView
{
public:
  enum ActionT {
    CREATE_USER = 1,
    UPDATE_USER = 2
  };
  UserFormView(const DboUserT* user, bool changePassword, bool userForm);
  ~UserFormView(void);
  Wt::Signal<DboUserT>& validated(void) {return m_validated;}
  Wt::Signal<std::string>& deleteTriggered(void) {return m_deleteTriggered;}
  Wt::Signal<std::string, std::string, std::string>& changePasswordTriggered(void) {return m_changePasswordTriggered;}
  Wt::Signal<Wt::WMouseEvent>& closeTriggered(void) {return m_close;}
  void reset(void);
  void setWritable(bool writable);
  void resetValidationState(bool writable);

private:
  DboUserT m_user;
  bool m_changePassword;
  UserFormModel* m_modelRef;
  Wt::WText* m_infoBox;
  Wt::WDialog *m_changePasswordDialog;
  Wt::WComboBox* m_opsProfileModeFieldRef;
  Wt::WSpinBox* m_opsProfileTilesPerRowFieldRef;

  Wt::Signal<DboUserT> m_validated;
  Wt::Signal<std::string> m_deleteTriggered;
  Wt::Signal<std::string, std::string, std::string> m_changePasswordTriggered;
  Wt::Signal<Wt::WMouseEvent> m_close;
  Wt::WLineEdit* m_lastNameFieldRef;


  void process(void);
  void handleDeleteClick(void);
  void handleCloseClick(Wt::WMouseEvent ev);
  void handleCancelClick(Wt::WMouseEvent ev);
  void handleChangePasswordClick(const std::string& login, const std::string& currentPass,const std::string& newPass);
  std::unique_ptr<Wt::WComboBox> createUserRoleField(void);
  std::unique_ptr<Wt::WLineEdit> createPasswordField(void);
  void createChangePasswordDialog(void);
  std::unique_ptr<Wt::WComboBox> createDashboardDisplayModeField(void);
  std::unique_ptr<Wt::WSpinBox> createDashboardTilesPerRowField(void);
};


class DbUserManager : public Wt::WContainerWidget
{
public:
  enum {
    AddUserAction=0,
    ListUserAction=1
  };
  DbUserManager(DbSession* dbSession);
  ~DbUserManager(void);

  void updateDbUsers(void);
  std::unique_ptr<Wt::WTemplate> dbUserListWidget(void) {return std::move(m_dbUserListWidget);}
  Wt::WTemplate* dbUserListWidgetRef(void) {return m_dbUserListWidgetRef;}
  std::unique_ptr<Wt::WPanel> createUserPanel(const DboUserT& user);
  std::unique_ptr<UserFormView> userForm() {return std::move(m_userForm);}
  UserFormView* userFormRef() {return m_userFormRef;}
  Wt::Signal<int>& updateCompleted(void) {return m_updateCompleted;}
  void resetUserForm(void) {m_userFormRef->reset();}
  std::string lastError(void) const {return m_lastError.toStdString();}

private:
  /** signals **/
  Wt::Signal<int> m_updateCompleted;

  /** Private member **/
  QString m_lastError;
  DbSession* m_dbSession;
  Wt::WStackedWidget* m_contents;
  UserFormView* m_userFormRef;
  std::unique_ptr<UserFormView> m_userForm;
  std::unique_ptr<Wt::WTemplate> m_dbUserListWidget;
  Wt::WTemplate* m_dbUserListWidgetRef;

  void handleFormAddUser(DboUserT dboUser);
  void handleFormUpdateUser(DboUserT dboUser);
  void handleDeleteUser(std::string username);
  void handleFormChangePassword(std::string username, std::string oldPass, std::string newPass);
};


#endif // USERFORM_HPP
