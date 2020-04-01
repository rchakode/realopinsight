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
#include "WebUtils.hpp"
#include <Wt/WTreeView.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WModelIndex.h>
#include <Wt/WPopupMenu.h>

class WebTree : public Wt::WTreeView
{
  public:
    WebTree(CoreDataT* cdata);
    virtual ~WebTree();
    void updateItemDecoration(const NodeT& nodeInfo, const QString& tooltip);
    void build(void);
    void activateEditionFeatures(void);
    void newNodeItem(const NodeT& nodeInfo, const QString& parentId, bool selectNewNode);
    QString findNodeIdFromTreeItem(const Wt::WModelIndex& index) const;
    void expandNodeById(const QString& nodeId);
    void selectNodeById(const QString& nodeId);
    void updateItemLabel(const QString& nodeId, const std::string& label);

  private:
    CoreDataT* m_cdata;
    std::map<QString, Wt::WStandardItem*> m_items;
    std::shared_ptr<Wt::WStandardItemModel> m_model;

    void activateDashboardFeatures(void);
    Wt::WStandardItem* findItemByNodeId(const QString& nodeId);
    void bindChildToParent(const QString& childId, const QString& parentId);

    static std::string childMgntTip(void) {
      return Q_TR("Clik on a node; Shift+C to add a child; Shift+D to delete");
    }
    static std::string newEditionTip(void) {
      return Q_TR("Click on this area; Shift+N to start a new edition");
    }
};

#endif /* WEBTREE_HPP */
