/*
# ViewMgnt.cpp
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

#include "ViewAccessControl.hpp"
#include "DbSession.hpp"
#include "WebUtils.hpp"
#include <QObject>
#include <Wt/WSelectionBox.h>
#include <Wt/WComboBox.h>
#include <Wt/WStandardItem.h>
#include <Wt/WText.h>

ViewAccessControl::ViewAccessControl(DbSession* dbSession)
  : Wt::WContainerWidget(),
    m_dbSession(dbSession)
{
  auto page = std::make_unique<Wt::WTemplate>(Wt::WString::tr("view-assignment-tpl"));

  auto assignedViewModel = std::make_unique<Wt::WStandardItemModel>();
  m_assignedViewModelRef = assignedViewModel.get();
  setModelHeaderTitles(m_assignedViewModelRef);
  auto assignedViewList = createViewList(std::move(assignedViewModel));
  m_assignedViewList = assignedViewList.get();
  m_assignedViewList->setSelectable(true);
  page->bindWidget("assigned-views", std::move(assignedViewList));

  auto nonAssignedViewModel = std::make_unique<Wt::WStandardItemModel>();
  m_nonAssignedViewModelRef = nonAssignedViewModel.get();
  setModelHeaderTitles(m_nonAssignedViewModelRef);
  auto nonAssignedViewList = createViewList(std::move(nonAssignedViewModel));
  m_nonAssignedViewList = nonAssignedViewList.get();
  m_nonAssignedViewList->setSelectable(true);
  page->bindWidget("non-assigned-views", std::move(nonAssignedViewList));

  auto userSelector = std::make_unique<Wt::WComboBox>();
  m_userSelectorRef = userSelector.get();
  auto userListModel = std::make_unique<Wt::WStandardItemModel>();
  m_userListModelRef = userListModel.get();
  userSelector->setModel(std::move(userListModel));
  userSelector->setSelectable(true);
  userSelector->changed().connect(this, &ViewAccessControl::handleUserSelectionChanged);
  page->bindWidget("user-selector", std::move(userSelector));

  auto assignBtn = std::make_unique<Wt::WPushButton>(QObject::tr("<< Assign").toStdString());
  m_assignBtnRef = assignBtn.get();
  m_assignBtnRef->setStyleClass("btn btn-success");
  m_assignBtnRef->clicked().connect(this, &ViewAccessControl::assignViews);
  page->bindWidget("button-assign", std::move(assignBtn));

  auto revokeBtn = std::make_unique<Wt::WPushButton>(QObject::tr("Revoke").toStdString());
  m_revokeBtnRef = revokeBtn.get();
  m_revokeBtnRef->setStyleClass("btn btn-warning");
  m_revokeBtnRef->clicked().connect(this, &ViewAccessControl::revokeViews);
  page->bindWidget("button-revoke", std::move(revokeBtn));

  auto deleteBtn = std::make_unique<Wt::WPushButton>(QObject::tr("Delete").toStdString());
  m_deleteBtnRef = deleteBtn.get();
  m_deleteBtnRef->setStyleClass("btn btn-danger");
  m_deleteBtnRef->clicked().connect(this, &ViewAccessControl::deleteViews);
  page->bindWidget("delete-view-button", std::move(deleteBtn));

  updateFieldStates();

  setMargin(30, Wt::AllSides);

  addWidget(std::move(page));
}


ViewAccessControl::~ViewAccessControl(void){}


void ViewAccessControl::filter(const std::string& username)
{
  if (! username.empty()) {
    m_assignedViewModelRef->clear();
    m_nonAssignedViewModelRef->clear();
    UserViewsT userViews = m_dbSession->updateUserViewList();
    for (auto view: m_dbSession->listViews()) {
      if (userViews.find(username+":"+view.name) != userViews.end()) {
        addView(m_assignedViewModelRef, view);
      } else {
        addView(m_nonAssignedViewModelRef, view);
      }
    }
  } else {
    updateFieldStates();
  }
  if (! m_username.empty()) {
    enableButtonIfApplicable(m_nonAssignedViewModelRef,
                             m_assignBtnRef,
                             &ViewAccessControl::assignViews);
    enableButtonIfApplicable(m_assignedViewModelRef,
                             m_revokeBtnRef,
                             &ViewAccessControl::revokeViews);
  }
}

void ViewAccessControl::addView(Wt::WStandardItemModel* model, const DboView& view)
{
  int count = model->rowCount();
  model->insertRows(count, 1);
  model->setData(count, 0, view.name);
  model->setData(count, 1, view.service_count);
}

void ViewAccessControl::setModelHeaderTitles(Wt::WStandardItemModel* model)
{
  model->insertColumns(model->columnCount(), 2);
  model->setHeaderData(0, Wt::WString("Name"));
  model->setHeaderData(1, Wt::WString("Number of services"));
}

std::unique_ptr<Wt::WSelectionBox> ViewAccessControl::createViewList(std::unique_ptr<Wt::WStandardItemModel> model)
{
  auto list = std::make_unique<Wt::WSelectionBox>();
  list->setSelectionMode(Wt::SelectionMode::Extended);
  list->setMargin(10, Wt::Side::Top | Wt::Side::Bottom);
  list->setMargin(Wt::WLength::Auto, Wt::Side::Left|Wt::Side::Right);
  list->setModel(std::move(model));
  return list;
}

void ViewAccessControl::updateFieldStates(void)
{
  m_username.clear();
  m_userListModelRef->clear();
  m_assignedViewModelRef->clear();
  m_nonAssignedViewModelRef->clear();

  int count = 0;
  m_userListModelRef->insertRows(count, 1);
  m_userListModelRef->setData(count, 0, Wt::WString("-- Select user --"));
  ++count;

  for (const auto& user: m_dbSession->listUsers()) {
    m_userListModelRef->insertRows(count, 1);
    m_userListModelRef->setData(count, 0, user.username);
    ++count;
  }

  auto vlist = m_dbSession->listViews();
  for (auto view: vlist) {
    addView(m_nonAssignedViewModelRef, view);
  }
  disableButtons();
  enableButtonIfApplicable(m_nonAssignedViewModelRef, m_deleteBtnRef, &ViewAccessControl::deleteViews);
}


std::string ViewAccessControl::itemText(Wt::WStandardItemModel* model, int index)
{
  return model->item(index, 0)->text().toUTF8();
}

void ViewAccessControl::assignViews(void)
{
  disableButtons();

  setSelectedViews(m_nonAssignedViewList, m_nonAssignedViewModelRef);

  for (auto name : m_selectedViews) {
    m_dbSession->assignView(m_username, name);
  }

  filter(m_username); //Enables disable button if needed
}

void ViewAccessControl::revokeViews(void)
{
  disableButtons();

  setSelectedViews(m_assignedViewList, m_assignedViewModelRef);

  for (auto name : m_selectedViews) {
    m_dbSession->revokeView(m_username, name);
  }

  filter(m_username); //Enables disable button if needed
}

void ViewAccessControl::deleteViews(void)
{
  disableButtons();
  setSelectedViews(m_nonAssignedViewList, m_nonAssignedViewModelRef);
  std::string outputMsg;
  for (const auto& vname : m_selectedViews) {
    DboView curView;
    if (m_dbSession->findView(vname, curView)) {
      auto deleteViewOut = m_dbSession->deleteViewWithName(vname);
      if (deleteViewOut.first != ngrt4n::RcSuccess) {
        outputMsg.append("- Failed to delete view: "+vname + " -");
        CORE_LOG("warning", deleteViewOut.second.toStdString());
      } else {
        if (! QFile(curView.path.c_str()).remove()) {
          CORE_LOG("info", "Failed to removed file: "+curView.path);
        } else {
          CORE_LOG("info", "View removed: "+vname);
          m_viewDeleted.emit(vname);
        }
      }
    }
  }
  filter(m_username);
}


void ViewAccessControl::enableButtonIfApplicable(Wt::WStandardItemModel* model,
                                                 Wt::WPushButton* button,
                                                 void (ViewAccessControl::* )(void))
{
  if (model->rowCount() <=0) {
    button->setEnabled(false);
  } else {
    button->setEnabled(true);
  }
}

void ViewAccessControl::disableButtons(void)
{
  m_assignBtnRef->setEnabled(false);
  m_revokeBtnRef->setEnabled(false);
  m_deleteBtnRef->setEnabled(false);
}

void ViewAccessControl::setSelectedViews(Wt::WSelectionBox* list, Wt::WStandardItemModel* model)
{
  m_selectedViews.clear();
  for (auto index : list->selectedIndexes()) {
    m_selectedViews.insert(itemText(model, index));
  }
}

void ViewAccessControl::removeViewItemInModel(Wt::WStandardItemModel* model, const std::string& viewName)
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

void ViewAccessControl::addViewItemInModel(Wt::WStandardItemModel* model, const std::string& viewName)
{
  DbViewsT::const_iterator vit;
  vit = std::find_if(m_dbSession->listViews().begin(),
                     m_dbSession->listViews().end(),
                     [=](DboView v){return v.name == viewName;});
  if (vit !=m_dbSession->listViews().end()) {
    addView(model, *vit);
  }
}

void ViewAccessControl::handleUserSelectionChanged()
{
  if (m_userSelectorRef->currentIndex()>0) {
    m_username = m_userSelectorRef->currentText().toUTF8();
    filter(m_username);
  } else {
    updateFieldStates();
  }
}

