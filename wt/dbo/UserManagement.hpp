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
#include <Wt/WBoostAny>
#include <Wt/WDateEdit>
#include <Wt/WDateValidator>
#include <Wt/WFormModel>
#include <Wt/WIntValidator>
#include <Wt/WLengthValidator>
#include <Wt/WModelIndex>
#include <Wt/WPushButton>
#include <Wt/WString>
#include <Wt/WTemplateFormView>
#include <Wt/WValidator>
#include <Wt/WStackedWidget>
#include <Wt/WRegExpValidator>
#include <Wt/WSignal>
#include <Wt/WScrollArea>

class DbSession;
class UserFormModel;
class UserFormView;

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
  static constexpr Wt::WFormModel::Field InfoBoxField = "infox-box";
  static constexpr Wt::WFormModel::Field UsernameField = "user-name";
  static constexpr Wt::WFormModel::Field CurrentPasswordField = "current-password";
  static constexpr Wt::WFormModel::Field PasswordField = "password";
  static constexpr Wt::WFormModel::Field PasswordConfimationField = "confirm-password";
  static constexpr Wt::WFormModel::Field FirstNameField = "first-name";
  static constexpr Wt::WFormModel::Field LastNameField = "last-name";
  static constexpr Wt::WFormModel::Field EmailField = "email";
  static constexpr Wt::WFormModel::Field UserLevelField = "role";
  static constexpr Wt::WFormModel::Field RegistrationDateField = "registration-date";

  UserFormModel(const RoiDboUser* user, bool changePassword, bool userForm, Wt::WObject *parent = 0);
  void setWritable(bool writtable);
  void setData(const RoiDboUser& user);

private:
  static const int MAX_LENGTH = 25;
  static const int MAX_CHILDREN = 15;
  bool m_userForm;

  Wt::WValidator* createNameValidator(void);
  Wt::WValidator* createEmailValidator(void);
  Wt::WValidator* createPasswordValidator(void);
  Wt::WValidator* createConfirmPasswordValidator(void);

};

class UserFormView : public Wt::WTemplateFormView
{
public:
  enum ActionT {
    CREATE_USER = 1,
    UPDATE_USER = 2
  };
  UserFormView(const RoiDboUser* user, bool changePassword, bool userForm);
  ~UserFormView(void);
  Wt::Signal<RoiDboUser>& validated(void) {return m_validated;}
  Wt::Signal<std::string>& deleteTriggered(void) {return m_deleteTriggered;}
  Wt::Signal<std::string, std::string, std::string>& changePasswordTriggered(void) {return m_changePasswordTriggered;}
  Wt::Signal<void>& closeTriggered(void) {return m_close;}
  void reset(void);
  void setWritable(bool writtable);

  void resetValidationState(bool writtable);

private:
  RoiDboUser m_user;
  bool m_changePassword;
  UserFormModel* m_model;
  Wt::WText* m_infoBox;
  Wt::WDialog *m_changePasswordDialog;

  Wt::Signal<RoiDboUser> m_validated;
  Wt::Signal<std::string> m_deleteTriggered;
  Wt::Signal<std::string, std::string, std::string> m_changePasswordTriggered;
  Wt::Signal<void> m_close;


  void process(void);
  void handleDeleteRequest(void);
  Wt::WComboBox* createUserLevelField(void);
  Wt::WLineEdit* createPaswordField(void);
  void createChangePasswordDialog(void);
};


class UserList : public Wt::WScrollArea
{
public:
  enum {
    AddUserAction=0,
    ListUserAction=1
  };
  UserList(DbSession* dbSession);
  ~UserList(void);
  void updateUserList(void);
  Wt::WPanel* createUserPanel(const RoiDboUser& user);
  UserFormView* userForm() {return m_userForm;}
  Wt::WContainerWidget* userListContainer(void) {return m_userListContainer;}
  void createUserList(void);
  Wt::WWidget* userListWidget(void) {return m_userListWidget;}
  Wt::Signal<int>& updateCompleted(void) {return m_updateCompleted;}
  void resetUserForm(void) {m_userForm->reset();}

private:
  DbSession* m_dbSession;
  UserFormView* m_userForm;
  Wt::WContainerWidget* m_userListContainer;
  Wt::WStackedWidget* m_contents;
  Wt::WWidget* m_userListWidget;
  Wt::Signal<int> m_updateCompleted;
};


#endif // USERFORM_HPP
