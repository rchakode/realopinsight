/*
 * ViewMgnt.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 06-12-2013                                                 #
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

#include <Wt/WDialog>
#include <Wt/WTemplate>
#include <Wt/WStandardItemModel>
#include <Wt/WTableView>
#include <Wt/WPushButton>
#include <set>

class View;
class DbSession;

class ViewAssignmentUI : public Wt::WDialog
{
public:
  ViewAssignmentUI(DbSession* dbSession, Wt::WObject* parent=0);
  virtual ~ViewAssignmentUI(void);

  void filter(const std::string& username);
  void resetModelData(void);

private:
  typedef std::set<std::string> KeyListT;
  DbSession* m_dbSession;
  Wt::WStandardItemModel* m_userListModel;
  Wt::WStandardItemModel* m_assignedViewModel;
  Wt::WStandardItemModel* m_nonAssignedViewModel;
  Wt::WSelectionBox* m_assignedViewList;
  Wt::WSelectionBox* m_nonAssignedViewList;
  std::string m_username;
  Wt::WPushButton* m_assignButton;
  Wt::WPushButton* m_revokeButton;
  Wt::WPushButton* m_deleteViewButton;
  KeyListT m_selectedViews;
  Wt::WText* m_infoBox;

  void addView(Wt::WStandardItemModel* model, const View& view);
  void setModelHeaderTitles(Wt::WStandardItemModel* model);
  Wt::WSelectionBox* createViewList(Wt::WStandardItemModel* model, Wt::WContainerWidget* parent);

  std::string itemText(Wt::WStandardItemModel* model, int index);
  void assignViews(void);
  void revokeViews(void);
  void deleteViews(void);
  void removeViewItemInModel(Wt::WStandardItemModel* model, const std::string& viewName);
  void addViewItemInModel(Wt::WStandardItemModel* model, const std::string& viewName);
  void enableButtonIfApplicable(Wt::WStandardItemModel* model,
                                Wt::WPushButton* button,
                                void (ViewAssignmentUI::* targetSlot)(void));
  void disableButtons(void);
  void setSelectedViews(Wt::WSelectionBox* list, Wt::WStandardItemModel* model);
};

#endif // VIEWMGNT_HPP
