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
#include "WebUtils.hpp"
#include <Wt/WSelectionBox>
#include <QObject>
#include <Wt/WComboBox>
#include <Wt/WStandardItem>
#include <Wt/WText>

ViewAssignmentUI::ViewAssignmentUI(DbSession* dbSession, Wt::WObject* parent)
  : Wt::WDialog(QObject::tr("View Managment").toStdString(), parent),
    m_dbSession(dbSession),
    m_userListModel(new Wt::WStandardItemModel(this)),
    m_assignedViewModel(new Wt::WStandardItemModel(this)),
    m_nonAssignedViewModel(new Wt::WStandardItemModel(this))
{
  setStyleClass("ngrt4n-gradient Wt-dialog");
  titleBar()->setStyleClass("titlebar");
  Wt::WContainerWidget* container = contents();
  container->setMargin(30, Wt::Left|Wt::Right);
  container->setMargin(10, Wt::Bottom);
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("view-assignment-tpl"), container);

  m_infoBox = new Wt::WText(container);
  tpl->bindWidget("info-box", m_infoBox);

  setModelHeaderTitles(m_assignedViewModel);
  m_assignedViewList = createViewList(m_assignedViewModel, container);
  m_assignedViewList->setSelectable(true);
  tpl->bindWidget("assigned-views", m_assignedViewList);

  setModelHeaderTitles(m_nonAssignedViewModel);
  m_nonAssignedViewList = createViewList(m_nonAssignedViewModel, container);
  m_nonAssignedViewList->setSelectable(true);
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

  m_assignButton = new Wt::WPushButton(QObject::tr("<< Assign").toStdString(), container);
  m_assignButton->setStyleClass("btn btn-success");
  m_assignButton->clicked().connect(this, &ViewAssignmentUI::assignViews);

  m_revokeButton = new Wt::WPushButton(QObject::tr("Revoke>>").toStdString(), container);
  m_revokeButton->setStyleClass("btn btn-warning");
  m_revokeButton->clicked().connect(this, &ViewAssignmentUI::revokeViews);

  m_deleteViewButton = new Wt::WPushButton(QObject::tr("Delete View").toStdString(), container);
  m_deleteViewButton->setStyleClass("btn btn-danger");
  m_deleteViewButton->clicked().connect(this, &ViewAssignmentUI::deleteViews);

  tpl->bindWidget("button-assign", m_assignButton);
  tpl->bindWidget("button-revoke", m_revokeButton);
  tpl->bindWidget("delete-view-button", m_deleteViewButton);


  resetModelData();
}

ViewAssignmentUI::~ViewAssignmentUI(void)
{
  // deleted implicitly
  //  delete m_userListModel;
  //  delete m_assignedViewModel;
  //  delete m_nonAssignedViewModel;
}

void ViewAssignmentUI::filter(const std::string& username)
{
  if (! username.empty()) {
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
  } else {
    resetModelData();
  }
  if (! m_username.empty()) {
    enableButtonIfApplicable(m_nonAssignedViewModel,
                             m_assignButton,
                             &ViewAssignmentUI::assignViews);
    enableButtonIfApplicable(m_assignedViewModel,
                             m_revokeButton,
                             &ViewAssignmentUI::revokeViews);
  }
}

void ViewAssignmentUI::addView(Wt::WStandardItemModel* model, const View& view)
{
  int count = model->rowCount();
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
  m_username.clear();
  m_userListModel->clear();
  m_assignedViewModel->clear();
  m_nonAssignedViewModel->clear();

  m_dbSession->updateUserList();
  m_dbSession->updateViewList();

  int count = 0;
  m_userListModel->insertRows(count, 1);
  m_userListModel->setData(count, 0, Wt::WString("Select a user"));
  ++count;

  for (const auto& user: m_dbSession->userList()) {
    m_userListModel->insertRows(count, 1);
    m_userListModel->setData(count, 0, user.username);
    ++count;
  }

  for (auto view: m_dbSession->viewList()) {
    addView(m_nonAssignedViewModel, view);
  }

  disableButtons();
  enableButtonIfApplicable(m_nonAssignedViewModel,
                           m_deleteViewButton,
                           &ViewAssignmentUI::deleteViews);
}


std::string ViewAssignmentUI::itemText(Wt::WStandardItemModel* model, int index)
{
  return model->item(index, 0)->text().toUTF8();
}

void ViewAssignmentUI::assignViews(void)
{
  disableButtons();

  setSelectedViews(m_nonAssignedViewList, m_nonAssignedViewModel);

  for (auto name : m_selectedViews) {
    m_dbSession->assignView(m_username, name);
  }

  filter(m_username); //Enables disable button if needed
}

void ViewAssignmentUI::revokeViews(void)
{
  disableButtons();

  setSelectedViews(m_assignedViewList, m_assignedViewModel);

  for (auto name : m_selectedViews) {
    m_dbSession->revokeView(m_username, name);
  }

  filter(m_username); //Enables disable button if needed
}

void ViewAssignmentUI::deleteViews(void)
{
  disableButtons();
  setSelectedViews(m_nonAssignedViewList, m_nonAssignedViewModel);

  std::string msg;
  int summary = 0;
  for (auto vname : m_selectedViews) {
    int ret = m_dbSession->deleteView(vname);
    if (ret != 0) {
      msg+="- Failed to delete "+vname + " -";
      Wt::log("warning") <<"[realopinsight] "<< m_dbSession->lastError();
    } else {
      msg+="- View deleted "+vname + " -";
    }
    summary+=ret;
  }
  utils::showMessage(summary, msg, msg, m_infoBox);

  filter(m_username);
}


void ViewAssignmentUI::enableButtonIfApplicable(Wt::WStandardItemModel* model,
                                                Wt::WPushButton* button,
                                                void (ViewAssignmentUI::* targetSlot)(void))
{
  if (model->rowCount() <=0) {
    button->setEnabled(false);
  } else {
    button->setEnabled(true);
    //button->clicked().connect(this, targetSlot);
  }
}

void ViewAssignmentUI::disableButtons(void)
{
  m_assignButton->setEnabled(false);
  m_revokeButton->setEnabled(false);
  m_deleteViewButton->setEnabled(false);
}

void ViewAssignmentUI::setSelectedViews(Wt::WSelectionBox* list, Wt::WStandardItemModel* model)
{
  m_selectedViews.clear();
  for (auto index : list->selectedIndexes()) {
    std::cout << itemText(model, index) << "\n";
    m_selectedViews.insert(itemText(model, index));
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
