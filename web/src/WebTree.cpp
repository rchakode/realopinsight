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
#include "utilsCore.hpp"
#include "WebUtils.hpp"
#include <Wt/WLength>

WebTree::WebTree(void)
  : Wt::WTreeView(0),
    m_model(new Wt::WStandardItemModel(0,1))
{
  setModel(m_model);
  activateDashboardFeatures();
}


WebTree::~WebTree()
{
  delete m_model;
}


void WebTree::activateDashboardFeatures(void)
{
  setHeaderHeight(0);
  setSelectionMode(Wt::SingleSelection);
  setSelectable(false);
  setSelectionBehavior(Wt::SelectItems);
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
  bool bindToParent = false;

  for(NodeListT::ConstIterator node  = m_cdata->bpnodes.begin(), end = m_cdata->bpnodes.end();  node != end; ++node) {
    WebTree::addTreeEntry(*node, bindToParent);
  }

  for(NodeListT::ConstIterator node=m_cdata->cnodes.begin(), end=m_cdata->cnodes.end();  node != end; ++node) {
    WebTree::addTreeEntry(*node, bindToParent);
  }

  for (StringListT::Iterator edge=m_cdata->edges.begin(), end=m_cdata->edges.end(); edge != end; ++edge) {
    bindChildToParent(edge.value(), edge.key());
  }

  renewModel();
}


void WebTree::renewModel(void)
{
  Wt::WStandardItemModel* oldModel = m_model;
  m_model = new Wt::WStandardItemModel();

  m_model->appendRow(m_treeItems[ngrt4n::ROOT_ID]);
  m_model->setHeaderData(0, Wt::Horizontal, Q_TR("Service Exporer"));

  setModel(m_model);
  delete oldModel;

  expandRootNode();
}


void WebTree::expandRootNode(void)
{
  auto root = m_treeItems[ngrt4n::ROOT_ID];
  if (root) {
    expand(root->index());
  }
}


void WebTree::selectRootNode(void)
{
  auto root = m_treeItems[ngrt4n::ROOT_ID];
  if (root) {
    select(root->index());
  }
}

Wt::WStandardItem* WebTree::addTreeEntry(const NodeT& _node, bool _bindToParent)
{
  auto item = new Wt::WStandardItem();

  item->setText(Wt::WString(_node.name.toStdString()));
  item->setIcon("images/built-in/unknown.png");
  item->setData(_node.id, Wt::UserRole);

  m_treeItems.insertMulti(_node.id, item);

  if (_bindToParent) {
    bindChildToParent(_node.id, _node.parent);
  }

  return item;
}


Wt::WStandardItem* WebTree::findTreeItem(const QString& _nodeId)
{
  auto item = m_treeItems.find(_nodeId);
  return (item != m_treeItems.end())? *item : NULL;
}

void WebTree::bindChildToParent(const QString& childId, const QString& parentId)
{
  auto parentItem = findTreeItem(parentId);
  auto childItem = findTreeItem(childId);
  if (parentItem && childItem) {
    parentItem->appendRow(childItem);
  }
}

void WebTree::updateItemDecoration(const NodeT& _node, const QString& _tip)
{
  auto item = findTreeItem(_node.id);
  if (item) {
    item->setIcon(ngrt4n::getIconPath(_node.sev).toStdString());
    item->setToolTip(Wt::WString::fromUTF8(_tip.toStdString()));
  }
}

QString WebTree::getNodeIdFromTreeItem(const Wt::WModelIndex& _index) const {

  auto item = m_model->itemFromIndex(_index);
  if (! item) {
    return "";
  }

  return boost::any_cast<QString>(item->data(Wt::UserRole));
}


void WebTree::updateItemLabel(const QString& _nodeId, const QString& label)
{
  auto item = findTreeItem(_nodeId);
  if (item) {
    item->setText(label.toStdString());
  }
}
