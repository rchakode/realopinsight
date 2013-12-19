/*
 * UserForm.hpp
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

#ifndef USERFORM_HPP
#define USERFORM_HPP

#include "User.hpp"
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

  UserFormModel(const User* user, bool changePassword, Wt::WObject *parent = 0);
  void setWritable(bool writtable);

private:
  static const int MAX_LENGTH = 25;
  static const int MAX_CHILDREN = 15;


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
  UserFormView(const User* user, bool changePassword);
  ~UserFormView(void);
  Wt::Signal<User>& validated(void) {return m_validated;}
  Wt::Signal<std::string>& deleteTriggered(void) {return m_deleteTriggered;}
  Wt::Signal<std::string, std::string>& changePasswordTriggered(void) {return m_changePasswordTrigerred;}

  void showMessage(int opStatus,
                   const std::string& errorMsg,
                   const std::string& successMsg)
  {
    WTemplate* tpl = NULL;
    if (opStatus != 0){
      tpl = new WTemplate(Wt::WString::tr("error-msg-div-tpl"));
      tpl->bindString("msg", errorMsg);
    } else {
      tpl = new WTemplate(Wt::WString::tr("success-msg-div-tpl"));
      tpl->bindString("msg", successMsg);
    }
    if (tpl) {
      std::ostringstream oss;
      tpl->renderTemplate(oss);
      m_infoBox->setText(oss.str());
      delete tpl;
    }
  }

private:
  const User* m_user;
  bool m_changePassword;
  UserFormModel* m_model;
  Wt::Signal<User> m_validated;
  Wt::Signal<std::string> m_deleteTriggered;
  Wt::Signal<std::string, std::string> m_changePasswordTrigerred;
  Wt::WText* m_infoBox;
  Wt::WDialog *m_changePasswordDialog;

  void process(void);
  Wt::WComboBox* createUserLevelField(void);
  Wt::WLineEdit* createPaswordField(void);
  void createChangePasswordDialog(void);
};


class UserMngtUI : public Wt::WScrollArea
{
public:
  enum {
    AddUserAction=0,
    ListUserAction=1
  };
  UserMngtUI(DbSession* dbSession, Wt::WContainerWidget* parent=0);
  ~UserMngtUI(void);
  void updateUserList(void);
  Wt::WPanel* createUserPanel(const User& user);
  void showDestinationView(int dest);

private:
  DbSession* m_dbSession;
  UserFormView* m_userForm;
  Wt::WContainerWidget* m_userListContainer;
  Wt::WStackedWidget* m_contents;
  Wt::WMenu* m_menu;
  std::map<int, Wt::WMenuItem*> m_menus;

  void addJsEventScript(void);
  Wt::WWidget* createUserList(void);
  Wt::WWidget* createMainUI(void);
};


#endif // USERFORM_HPP
