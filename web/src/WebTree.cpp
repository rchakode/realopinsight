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
  // just clear m_treeItems, because the containing pointer shall be deleted with the tree model
  m_treeItems.clear();


  // now reconstruct the tree
  bool bindToParent = false;
  bool selectItemAfterProcessing = false;

  for(NodeListT::ConstIterator node = m_cdata->bpnodes.begin(), end = m_cdata->bpnodes.end();  node != end; ++node) {
    WebTree::addTreeItem(*node, bindToParent, selectItemAfterProcessing);
  }

  for(NodeListT::ConstIterator node=m_cdata->cnodes.begin(), end=m_cdata->cnodes.end();  node != end; ++node) {
    WebTree::addTreeItem(*node, bindToParent, selectItemAfterProcessing);
  }

  for (QMultiMap<QString, QString>::Iterator edge=m_cdata->edges.begin(), end=m_cdata->edges.end(); edge != end; ++edge) {
    bindChildToParent(edge.value(), edge.key());
  }

  renewModel();
}


void WebTree::renewModel(void)
{
  Wt::WStandardItemModel* oldModel = m_model;

  m_model = new Wt::WStandardItemModel();

  m_model->appendRow(m_treeItems[ngrt4n::ROOT_ID]);

  m_model->setHeaderData(0, Wt::Horizontal, Q_TR("Tree Explorer"));

  setModel(m_model);
  delete oldModel;
}



void WebTree::expandNodeById(const QString& nodeId)
{
  auto item = m_treeItems[nodeId];
  if (item) {
    expand(item->index());
  }
}

void WebTree::selectNodeById(const QString& nodeId)
{
  auto item = m_treeItems[nodeId];
  if (item) {
    select(item->index());
  }
}

void WebTree::addTreeItem(const NodeT& _node, bool _bindToParent, bool _selectItemAfterProcessing)
{
  auto item = new Wt::WStandardItem();

  item->setText(Wt::WString(_node.name.toStdString()));
  item->setIcon("images/built-in/unknown.png");
  item->setData(_node.id, Wt::UserRole);
  //FIXME item->setFlags(item->flags() | Wt::ItemFlag::ItemIsDragEnabled | Wt::ItemFlag::ItemIsDropEnabled) ;

  m_treeItems.insert(_node.id, item);

  if (_bindToParent && ! _node.parents.isEmpty()) {
    bindChildToParent(_node.id, *_node.parents.begin());
  }

  if (_selectItemAfterProcessing) {
    select(item->index());
  }
}


Wt::WStandardItem* WebTree::findItemByNodeId(const QString& _nodeId)
{
  auto item = m_treeItems.find(_nodeId);
  return (item != m_treeItems.end())? *item : nullptr;
}


void WebTree::bindChildToParent(const QString& childId, const QString& parentId)
{
  auto parentItem = findItemByNodeId(parentId);
  auto childItem = findItemByNodeId(childId);
  if (parentItem != NULL && childItem != NULL) {
    parentItem->appendRow(childItem);
  }
}


void WebTree::updateItemDecoration(const NodeT& _node, const QString& _tip)
{
  auto item = findItemByNodeId(_node.id);
  if (item) {
    item->setIcon(ngrt4n::getIconPath(_node.sev).toStdString());
    item->setToolTip(Wt::WString::fromUTF8(_tip.toStdString()));
  }
}

QString WebTree::findNodeIdFromTreeItem(const Wt::WModelIndex& _index) const {

  if (! _index.isValid()) {
    return "";
  }

  auto item = m_model->itemFromIndex(_index);
  if (! item) {
    return "";
  }

  QString id = "";
  try {
    id = boost::any_cast<QString>(item->data(Wt::UserRole));
  } catch(...) {
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
