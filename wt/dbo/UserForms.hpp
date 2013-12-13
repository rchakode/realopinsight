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

class DbSession;
class UserFormModel : public Wt::WFormModel
{
public:
  static constexpr Wt::WFormModel::Field UsernameField = "user-name";
  static constexpr Wt::WFormModel::Field FirstNameField = "first-name";
  static constexpr Wt::WFormModel::Field LastNameField = "last-name";
  static constexpr Wt::WFormModel::Field RoleField = "role";
  static constexpr Wt::WFormModel::Field RegistrationDateField = "registration-date";

  UserFormModel(Wt::WObject *parent = 0)
    : Wt::WFormModel(parent)
  {

    addField(FirstNameField);
    addField(LastNameField);
    addField(RoleField);
    addField(UsernameField);

    setValidator(FirstNameField, createNameValidator(FirstNameField));
    setValidator(LastNameField, createNameValidator(LastNameField));

    // Here you could populate the model with initial data using
    // setValue() for each field.

    setValue(RoleField, std::string());
  }

  // Get the user data from the model
  Wt::WString userData() {
    return
        Wt::asString(value(FirstNameField)) + " " +
        Wt::asString(value(LastNameField))
        + ": role=" + Wt::asString(value(RoleField))
        + ", username=" + Wt::asString(value(UsernameField))
        + ".";
  }

private:
  static const int MAX_LENGTH = 25;
  static const int MAX_CHILDREN = 15;


  Wt::WValidator *createNameValidator(const std::string& field) {
    Wt::WLengthValidator *v = new Wt::WLengthValidator();
    v->setMandatory(true);
    v->setMinimumLength(1);
    v->setMaximumLength(MAX_LENGTH);
    return v;
  }


  Wt::WValidator *createBirthValidator() {
    Wt::WDateValidator *v = new Wt::WDateValidator();
    v->setBottom(Wt::WDate(1900, 1, 1));
    v->setTop(Wt::WDate::currentDate());
    v->setFormat("dd/MM/yyyy");
    v->setMandatory(true);
    return v;
  }

  Wt::WValidator *createChildrenValidator() {
    Wt::WIntValidator *v = new Wt::WIntValidator(0, MAX_CHILDREN);
    v->setMandatory(true);
    return v;
  }

};

class UserFormView : public Wt::WTemplateFormView
{
public:
  // inline constructor
  UserFormView();
private:
  void process();
  UserFormModel *model;
};


Wt::WContainerWidget* createUserForms(const UserListT& users);
Wt::WContainerWidget* createUserList(const UserListT& users);
Wt::WPanel* createUserPanel(const User& user);

#endif // USERFORM_HPP
