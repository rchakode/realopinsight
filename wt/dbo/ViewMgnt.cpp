/*
 * ViewMgnt.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 22-12-2013                                                 #
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

#include "ViewMgnt.hpp"
#include "DbSession.hpp"
#include <Wt/WSelectionBox>
#include <QObject>
#include <Wt/WPushButton>
#include <Wt/WComboBox>

ViewAssignmentUI::ViewAssignmentUI(DbSession* dbSession, Wt::WObject* parent)
  : Wt::WDialog(QObject::tr("View Assignations").toStdString(), parent),
    m_dbSession(dbSession),
    m_userListModel(new Wt::WStandardItemModel(this)),
    m_assignedViewModel(new Wt::WStandardItemModel(this)),
    m_nonAssignedViewModel(new Wt::WStandardItemModel(this))
{
  contents()->setMargin(30, Wt::Left|Wt::Right);
  contents()->setMargin(10, Wt::Bottom);
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("view-assignment-tpl"), contents());
  setModelHeaderTitles(m_assignedViewModel);
  setModelHeaderTitles(m_nonAssignedViewModel);

  tpl->bindWidget("assigned-views", createViewList(m_assignedViewModel, contents()));
  tpl->bindWidget("non-assigned-views", createViewList(m_nonAssignedViewModel, contents()));

  Wt::WComboBox* userSelector = new Wt::WComboBox(contents());
  userSelector->setModel(m_userListModel);
  userSelector->setSelectable(true);
  userSelector->changed().connect(std::bind([=](){
    if (userSelector->currentIndex()>0) {
      m_username = userSelector->currentText().toUTF8();
      filter(m_username);
    } else {
      resetModelData();
    }
  }));
  tpl->bindWidget("user-selector", userSelector);

  Wt::WPushButton* assignButton = new Wt::WPushButton(QObject::tr("Assign >>").toStdString(),contents());
  assignButton->setStyleClass("btn-success");
  assignButton->clicked().connect(this, &ViewAssignmentUI::assignView);

  Wt::WPushButton* revokeButton = new Wt::WPushButton(QObject::tr("<<Revoke").toStdString(),contents());
  revokeButton->setStyleClass("btn-danger");
  revokeButton->clicked().connect(this, &ViewAssignmentUI::revokeView);

  Wt::WPushButton* closeButton = new Wt::WPushButton(QObject::tr("Close").toStdString(),contents());
  closeButton->setStyleClass("btn-primary");
  closeButton->clicked().connect(std::bind([=](){
    accept();
  }));

  tpl->bindWidget("button-assign", assignButton);
  tpl->bindWidget("button-revoke", revokeButton);
  tpl->bindWidget("close-button", closeButton);

  resetModelData();
}

ViewAssignmentUI::~ViewAssignmentUI(void)
{
  // delete explicitly
  //  delete m_userListModel;
  //  delete m_assignedViewModel;
  //  delete m_nonAssignedViewModel;
}

void ViewAssignmentUI::filter(const std::string& username)
{
  std::cout << username << "\n";
  m_assignedViewModel->clear();
  m_nonAssignedViewModel->clear();
  m_dbSession->updateUserViewList();
  UserViewListT userViews = m_dbSession->userViewList();
  for (auto view: m_dbSession->viewList()) {
    if (userViews.find(username+":"+view.name) != userViews.end()) {
      addView(m_assignedViewModel, view);
    } else {
      addView(m_nonAssignedViewModel, view);
    }
  }
}

void ViewAssignmentUI::addView(Wt::WStandardItemModel* model, const View& view)
{
  int count = m_assignedViewModel->rowCount();
  model->insertRows(count, 1);
  model->setData(count, 0, view.name);
  model->setData(count, 1, view.service_count);
}

void ViewAssignmentUI::setModelHeaderTitles(Wt::WStandardItemModel* model)
{
  model->insertColumns(model->columnCount(), 2);
  model->setHeaderData(0, Wt::WString("Name"));
  model->setHeaderData(1, Wt::WString("Number of services"));
}

Wt::WSelectionBox* ViewAssignmentUI::createViewList(Wt::WStandardItemModel* model, Wt::WContainerWidget* parent)
{
  Wt::WSelectionBox* list = new Wt::WSelectionBox (parent);
  list->setMargin(10, Wt::Top | Wt::Bottom);
  list->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);
  list->setModel(model);
  return list;
}


void ViewAssignmentUI::resetModelData(void)
{
  m_userListModel->clear();
  m_assignedViewModel->clear();
  m_nonAssignedViewModel->clear();

  int count = 0;
  m_userListModel->insertRows(count, 1);
  m_userListModel->setData(count, 0, Wt::WString("Select a user"));
  ++count;
  m_dbSession->updateUserList();
  for (auto user: m_dbSession->userList()) {
    m_userListModel->insertRows(count, 1);
    m_userListModel->setData(count, 0, user.username);
    //m_userListModel->setData(count, 1, user.service_count);
    ++count;
  }
}
