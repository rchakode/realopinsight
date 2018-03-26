/*
 * WebTree.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
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

#ifndef WEBTREE_HPP
#define WEBTREE_HPP


#include "Base.hpp"
#include "utilsCore.hpp"
#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WModelIndex>
#include <Wt/WPopupMenu>

class WebTree : public Wt::WTreeView
{
  public:
    WebTree(void);
    virtual ~WebTree();
    void setCdata(CoreDataT* cdata) {m_cdata = cdata;}
    void updateItemDecoration(const NodeT& _node, const QString& _tip);
    void build(void);
    void renewModel(void);
    void activateEditionFeatures(void);
    void addTreeItem(const NodeT& _node, bool _bindToParent, bool _selectItemAfterProcessing);
    //void dropParentChildDependency(const QString& parentId, const QString& childId);
    QString findNodeIdFromTreeItem(const Wt::WModelIndex& _index) const;
    void expandNodeById(const QString& nodeId);
    void selectNodeById(const QString& nodeId);
    void updateItemLabel(const QString& nodeId, const std::string& label);

  private:
    Wt::WStandardItemModel* m_model;
    CoreDataT* m_cdata;
    QMap<QString,  Wt::WStandardItem*> m_treeItems;

    void activateDashboardFeatures(void);
    Wt::WStandardItem* findItemByNodeId(const QString& _nodeId);
    void bindChildToParent(const QString& childId, const QString& parentId);
};

#endif /* WEBTREE_HPP */
