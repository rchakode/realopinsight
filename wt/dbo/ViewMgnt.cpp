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
#include <Wt/WStandardItem>

ViewAssignmentUI::ViewAssignmentUI(DbSession* dbSession, Wt::WObject* parent)
  : Wt::WDialog(QObject::tr("View Assignations").toStdString(), parent),
    m_dbSession(dbSession),
    m_userListModel(new Wt::WStandardItemModel(this)),
    m_assignedViewModel(new Wt::WStandardItemModel(this)),
    m_nonAssignedViewModel(new Wt::WStandardItemModel(this))
{
  Wt::WContainerWidget* container = contents();
  container->setMargin(30, Wt::Left|Wt::Right);
  container->setMargin(10, Wt::Bottom);
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("view-assignment-tpl"), container);
  setModelHeaderTitles(m_assignedViewModel);
  setModelHeaderTitles(m_nonAssignedViewModel);

  m_assignedViewList = createViewList(m_assignedViewModel, container);
  m_nonAssignedViewList = createViewList(m_nonAssignedViewModel, container);

  m_assignedViewList->setSelectable(true);
  m_nonAssignedViewList->setSelectable(true);

  tpl->bindWidget("assigned-views", m_assignedViewList);
  tpl->bindWidget("non-assigned-views",m_nonAssignedViewList);

  Wt::WComboBox* userSelector = new Wt::WComboBox(container);
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

  Wt::WPushButton* assignButton = new Wt::WPushButton(QObject::tr("<<Assign").toStdString(),container);
  assignButton->setStyleClass("btn-success");
  assignButton->clicked().connect(this, &ViewAssignmentUI::assignView);

  Wt::WPushButton* revokeButton = new Wt::WPushButton(QObject::tr("Revoke>>").toStdString(),container);
  revokeButton->setStyleClass("btn-danger");
  revokeButton->clicked().connect(this, &ViewAssignmentUI::revokeView);

  Wt::WPushButton* closeButton = new Wt::WPushButton(QObject::tr("Close").toStdString(),container);
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
  if (! username.empty()) {
    m_assignedViewModel->clear();
    m_nonAssignedViewModel->clear();
    //FIXME segfault m_dbSession->updateUserViewList();
    UserViewListT userViews = m_dbSession->userViewList();
    for (auto view: m_dbSession->viewList()) {
      if (userViews.find(username+":"+view.name) != userViews.end()) {
        addView(m_assignedViewModel, view);
      } else {
        addView(m_nonAssignedViewModel, view);
      }
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
  list->setSelectionMode(Wt::ExtendedSelection);
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
  for (const auto& user: m_dbSession->userList()) {
    m_userListModel->insertRows(count, 1);
    m_userListModel->setData(count, 0, user.username);
    ++count;
  }
  filter(m_username);
}


std::string ViewAssignmentUI::itemText(Wt::WStandardItemModel* model, int index)
{
  return model->item(index, 0)->text().toUTF8();
}

void ViewAssignmentUI::assignView(void)
{
  //FIXME: segfault on duplication
  for (auto index : m_nonAssignedViewList->selectedIndexes()) {
    std::string viewName = itemText(m_nonAssignedViewModel, index) ;
    m_dbSession->assignView(m_username, viewName);
    removeViewItemInModel(m_nonAssignedViewModel, viewName);
    addViewItemInModel(m_assignedViewModel, viewName);
  }
}

void ViewAssignmentUI::revokeView(void)
{
  for (auto index : m_assignedViewList->selectedIndexes()) {
    std::string viewName = itemText(m_assignedViewModel, index);
    m_dbSession->revokeView(m_username,viewName);
    removeViewItemInModel(m_assignedViewModel, viewName);
    addViewItemInModel(m_nonAssignedViewModel, viewName);
  }
}

void ViewAssignmentUI::removeViewItemInModel(Wt::WStandardItemModel* model, const std::string& viewName)
{
  int index = model->rowCount() - 1;
  while (index >=0) {
    if (itemText(model, index) != viewName) {
      model->removeRow(index);
      break;
    }
    --index;
  }
}

void ViewAssignmentUI::addViewItemInModel(Wt::WStandardItemModel* model, const std::string& viewName)
{
  ViewListT::const_iterator vit;
  vit = std::find_if(m_dbSession->viewList().begin(),
                     m_dbSession->viewList().end(),
                     [=](View v){return v.name == viewName;});
  if (vit !=m_dbSession->viewList().end()) {
    addView(model, *vit);
  }
}
