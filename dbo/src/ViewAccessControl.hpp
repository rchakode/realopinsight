/*
 * ViewMgnt.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
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


#ifndef VIEWMGNT_HPP
#define VIEWMGNT_HPP

#include <set>
#include <Wt/WDialog.h>
#include <Wt/WTemplate.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTableView.h>
#include <Wt/WPushButton.h>
#include <Wt/WContainerWidget.h>

class DboView;
class DbSession;

class ViewAccessControl: public Wt::WContainerWidget
{
public:
  ViewAccessControl(DbSession* dbSession);
  virtual ~ViewAccessControl(void);

  void filter(const std::string& username);
  void updateFieldStates(void);
  Wt::Signal<std::string>& viewDeleted(void) {return m_viewDeleted;}

private:
  typedef std::set<std::string> KeyListT;
  DbSession* m_dbSession;
  Wt::WStandardItemModel* m_userListModelRef;
  Wt::WStandardItemModel* m_assignedViewModelRef;
  Wt::WStandardItemModel* m_nonAssignedViewModelRef;
  Wt::WSelectionBox* m_assignedViewList;
  Wt::WSelectionBox* m_nonAssignedViewList;
  std::string m_username;
  Wt::WPushButton* m_assignBtnRef;
  Wt::WPushButton* m_revokeBtnRef;
  Wt::WPushButton* m_deleteBtnRef;
  KeyListT m_selectedViews;
  Wt::Signal<std::string> m_viewDeleted;
  Wt::WComboBox* m_userSelectorRef;

  void addView(Wt::WStandardItemModel* model, const DboView& view);
  void setModelHeaderTitles(Wt::WStandardItemModel* model);
  std::unique_ptr<Wt::WSelectionBox> createViewList(std::unique_ptr<Wt::WStandardItemModel> model);

  std::string itemText(Wt::WStandardItemModel* model, int index);
  void assignViews(void);
  void revokeViews(void);
  void deleteViews(void);
  void removeViewItemInModel(Wt::WStandardItemModel* model, const std::string& viewName);
  void addViewItemInModel(Wt::WStandardItemModel* model, const std::string& viewName);
  void enableButtonIfApplicable(Wt::WStandardItemModel* model,
                                Wt::WPushButton* button,
                                void (ViewAccessControl::* targetSlot)(void));
  void disableButtons(void);
  void setSelectedViews(Wt::WSelectionBox* list, Wt::WStandardItemModel* model);
  void handleUserSelectionChanged();
};

#endif // VIEWMGNT_HPP
