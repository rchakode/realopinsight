/*
 * WebServiceTree.cpp
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

#include "WebTree.hpp"
#include <any>
#include <utility>
#include <Wt/WLength.h>

WebTree::WebTree(CoreDataT* cdata)
  : Wt::WTreeView(),
    m_cdata(cdata)
{
  m_model = std::make_shared<Wt::WStandardItemModel>(0, 1);
  m_model->setHeaderData(0, Wt::Orientation::Horizontal, newEditionTip());
  setModel(m_model);
  activateDashboardFeatures();
}

WebTree::~WebTree(){}

void WebTree::activateDashboardFeatures(void)
{
  setHeaderHeight(0);
  setSelectionMode(Wt::SelectionMode::Single);
  setSelectable(false);
  setSelectionBehavior(Wt::SelectionBehavior::Items);
  setColumnWidth(0, 500);
}


void WebTree::activateEditionFeatures(void)
{
  setHeaderHeight(Wt::WLength(20));
  setDragEnabled(true);
  setDropsEnabled(true);
}


void WebTree::build(void)
{
  m_items.clear();
  m_model->clear();

  // create bpnodes
  for(const auto & node : m_cdata->bpnodes) {
    WebTree::newNodeItem(node, "", true);
  }
  // create cnodes
  for(const auto & node : m_cdata->cnodes) {
    WebTree::newNodeItem(node, "", true);
  }
  // bind dependencies
  for (QMultiMap<QString, QString>::Iterator edge=m_cdata->edges.begin(), end=m_cdata->edges.end(); edge != end; ++edge) {
    bindChildToParent(edge.value(), edge.key());
  }
  // update model
  auto treeRoot = m_items.find(ngrt4n::ROOT_ID);
  if (treeRoot != m_items.end() && treeRoot->second) {
    m_model->appendRow(std::move(std::unique_ptr<Wt::WStandardItem>(treeRoot->second)));
    m_model->setHeaderData(0, Wt::Orientation::Horizontal, childMgntTip());
  }
}


void WebTree::expandNodeById(const QString& nodeId)
{
  auto item = m_items.find(nodeId);
  if (item != m_items.end() && item->second) {
    expand((item->second)->index());
  }
}

void WebTree::selectNodeById(const QString& nodeId)
{
  auto item = m_items.find(nodeId);
  if (item != m_items.end() && item->second) {
    select((item->second)->index());
  }
}

void WebTree::newNodeItem(const NodeT& nodeInfo, const QString& parentId, bool selectNewNode)
{
  auto item = new Wt::WStandardItem();
  item->setText(Wt::WString(nodeInfo.name.toStdString()));
  item->setIcon("images/built-in/unknown.png");
  item->setData(nodeInfo.id, Wt::ItemDataRole::User);
  m_items[nodeInfo.id] = item;

  if (! parentId.isEmpty()) {
    bindChildToParent(nodeInfo.id, parentId);
  }
  if (selectNewNode) {
    select(item->index());
  }
}


Wt::WStandardItem* WebTree::findItemByNodeId(const QString& nodeId)
{
  auto itemIt = m_items.find(nodeId);
  return (itemIt != m_items.end())? itemIt->second : nullptr;
}


void WebTree::bindChildToParent(const QString& childId, const QString& parentId)
{
  auto citem = findItemByNodeId(childId);
  auto pitem = findItemByNodeId(parentId);
  if (citem != nullptr && pitem != nullptr) {
    pitem->appendRow(std::move(std::unique_ptr<Wt::WStandardItem>(citem)));
  } else {
    CORE_LOG("debug", QObject::tr("ignoring dependency with child or parent not found").arg(parentId, childId).toStdString());
  }
}


void WebTree::updateItemDecoration(const NodeT& nodeInfo, const QString& tooltip)
{
  auto item = findItemByNodeId(nodeInfo.id);
  if (item) {
    item->setIcon(ngrt4n::getIconPath(nodeInfo.sev).toStdString());
    item->setToolTip(Wt::WString::fromUTF8(tooltip.toStdString()));
  }
}

QString WebTree::findNodeIdFromTreeItem(const Wt::WModelIndex& index) const
{
  if (! index.isValid()) {
    return "";
  }

  auto item = m_model->itemFromIndex(index);
  if (! item) {
    return "";
  }
  QString id("");
  try {
    id = Wt::cpp17::any_cast<QString>(item->data(Wt::ItemDataRole::User));
  } catch (...) {
    id = "";
  }
  return id;
}


void WebTree::updateItemLabel(const QString& nodeId, const std::string& label)
{
  auto item = findItemByNodeId(nodeId);
  if (item) {
    item->setText(label);
  }
}
