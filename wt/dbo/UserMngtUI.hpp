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

class DbSession;
class UserFormModel : public Wt::WFormModel
{
public:
  static constexpr Wt::WFormModel::Field UsernameField = "user-name";
  static constexpr Wt::WFormModel::Field PasswordField = "password";
  static constexpr Wt::WFormModel::Field PasswordConfimationField = "confirm-password";
  static constexpr Wt::WFormModel::Field FirstNameField = "first-name";
  static constexpr Wt::WFormModel::Field LastNameField = "last-name";
  static constexpr Wt::WFormModel::Field EmailField = "email";
  static constexpr Wt::WFormModel::Field UserLevelField = "role";
  static constexpr Wt::WFormModel::Field RegistrationDateField = "registration-date";

  UserFormModel(const User* user, Wt::WObject *parent = 0);
  void setWritable(bool writtable);

private:
  static const int MAX_LENGTH = 25;
  static const int MAX_CHILDREN = 15;


  Wt::WValidator* createNameValidator(const std::string& field);
  Wt::WValidator* createEmailValidator(const std::string& field);
  Wt::WValidator* createPasswordValidator(const std::string& field);
  Wt::WValidator* createConfirmPasswordValidator(const std::string& field);

};

class UserFormView : public Wt::WTemplateFormView
{
public:
  enum ActionT {
    CREATE_USER = 1,
    UPDATE_USER = 2
  };
  UserFormView(const User* user);
  Wt::Signal<User, std::string>& validated(void) {return m_validated;}

private:
  UserFormModel* m_model;
  Wt::Signal<User, std::string> m_validated;

  void process(void);
  Wt::WComboBox* createUserLevelField(void);
  Wt::WLineEdit* createPaswordField(void);
};


class UserMngtUI : public Wt::WContainerWidget
{
public:
  UserMngtUI(DbSession* dbSession, Wt::WContainerWidget* parent=0);
  void createUserForms(void);
  void updateUserList(void);
  Wt::WPanel* createUserPanel(const User& user);
private:
  DbSession* m_dbSession;
  UserFormView* m_userForm;
  Wt::WContainerWidget* m_userListContainer;
};


#endif // USERFORM_HPP
