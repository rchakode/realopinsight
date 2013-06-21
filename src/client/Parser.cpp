/*
 * Parser.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
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

#include "core/ns.hpp"
#include "Parser.hpp"
#include "GraphView.hpp"
#include "SvNavigatorTree.hpp"
#include "utilsClient.hpp"
#include <QObject>

using namespace std;

const QString Parser::CHILD_SEP = ",";
const QString Parser::dotFileHeader = "strict graph\n{\n node[shape=plaintext]\n";
const QString Parser::dotFileFooter = "}";


Parser::Parser() {}

Parser::~Parser()
{
  QFile dotFile;
  if (dotFile.exists(graphFilename)) dotFile.remove(graphFilename);
  if (dotFile.exists(graphFilename+".plain")) dotFile.remove(graphFilename+".plain");
  dotFile.close();
}

bool Parser::parseSvConfig(const QString& _configFile, CoreDataT& _cdata)
{
  QString graphContent ="";
  QDomDocument xmlDoc;
  QDomElement xmlRoot;

  QFile file(_configFile);
  if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
    utils::alert(QObject::tr("Unable to open the file %1").arg(_configFile));
    file.close();
    return false;
  }
  if (!xmlDoc.setContent(&file)) {
    file.close();
    utils::alert(QObject::tr("Error while parsing the file %1").arg(_configFile));
    return false;
  }
  file.close(); // The content of the file is already in memory

  xmlRoot = xmlDoc.documentElement();
  _cdata.monitor = xmlRoot.attribute("monitor").toInt();
  QDomNodeList services = xmlRoot.elementsByTagName("Service");

  NodeT node;
  qint32 serviceCount = services.length();
  for (qint32 srv = 0; srv < serviceCount; ++srv) {
    QDomElement service = services.item(srv).toElement();
    node.id = service.attribute("id").trimmed();
    node.monitored = false;
    node.type = service.attribute("type").toInt();
    node.sev_crule = service.attribute("statusCalcRule").toInt();
    node.sev_prule = service.attribute("statusPropRule").toInt();
    node.icon = service.firstChildElement("Icon").text().trimmed();
    node.name = service.firstChildElement("Name").text().trimmed();
    node.description = service.firstChildElement("Description").text().trimmed();
    node.alarm_msg = service.firstChildElement("AlarmMsg").text().trimmed();
    node.notification_msg = service.firstChildElement("NotificationMsg").text().trimmed();
    node.child_nodes = service.firstChildElement("SubServices").text().trimmed();
    node.severity = MonitorBroker::NagiosUnknown;
    node.parent = "";
    if (node.icon.isEmpty()) {
      node.icon = GraphView::DEFAULT_ICON;
    }
    if (node.type == NodeType::ALARM_NODE) {

      QPair<QString, QString> info = utils::splitCheckInfo(node.child_nodes);
      QString srcid = utils::getSourceIdFromStr(info.first);
      if (srcid.isEmpty()) {
        _cdata.sources.insert(utils::sourceId(0));
      } else {
        _cdata.sources.insert(srcid);
      }
      _cdata.hosts[info.first] <<  info.second;
      _cdata.cnodes.insert(node.id, node);

    } else {
      _cdata.bpnodes.insert(node.id, node);
    }
  }
  updateNodeHierachy(_cdata.bpnodes, _cdata.cnodes, graphContent);
  buildNodeTree(_cdata.bpnodes, _cdata.cnodes, _cdata.tree_items);
  graphContent = dotFileHeader + graphContent;
  graphContent += dotFileFooter;
  saveCoordinatesFile(graphContent);

  return true;
}

void Parser::updateNodeHierachy(NodeListT& _bpnodes,
                                NodeListT& _cnodes,
                                QString& _graphContent)
{
  _graphContent = "\n";
  for (NodeListT::ConstIterator node = _bpnodes.begin();
       node != _bpnodes.end(); ++node)
  {
    QString nname = node->name;
    _graphContent = "\t"%node->id%"[label=\""%nname.replace(' ', '#')%"\"];\n"%_graphContent;
    if (node->child_nodes != "") {
      QStringList ids = node->child_nodes.split(CHILD_SEP);
      foreach (const QString& nid, ids) {
        QString nidTrimmed = nid.trimmed();
        auto childNode = _cnodes.find(nidTrimmed);
        if (utils::findNode(_bpnodes, _cnodes, nidTrimmed, childNode)) {
          childNode->parent = node->id;
          _graphContent += "\t" + node->id%"--"%childNode->id%"\n";
        }
      }
    }
  }

  for (NodeListT::ConstIterator node = _cnodes.begin();
       node != _cnodes.end(); ++node)
  {
    QString nname = node->name;
    _graphContent = "\t"%node->id%"[label=\""%nname.replace(' ', '#')%"\"];\n"%_graphContent;
  }
}

void Parser::buildNodeTree(const NodeListT& _bpnodes,
                           const NodeListT& _cnodes,
                           TreeNodeItemListT& _tree)
{
  for (NodeListT::ConstIterator node=_bpnodes.begin(), end=_bpnodes.end();
       node!=end; ++node) { _tree.insert(node->id, SvNavigatorTree::createTreeItem(*node)); }

  for (NodeListT::ConstIterator node=_cnodes.begin(), end = _cnodes.end();
       node!=end; ++node) {_tree.insert(node->id, SvNavigatorTree::createTreeItem(*node));}

  for (NodeListT::ConstIterator node=_bpnodes.begin(), end=_bpnodes.end();
       node!=end; ++node)
  {
    if (node->child_nodes.isEmpty()) continue;
    auto treeItem = _tree.find(node->id);
    if (treeItem == _tree.end()) {
      utils::alert(QObject::tr("Service not found (%1)").arg(node->name));
      continue;
    }
    QStringList ids = node->child_nodes.split(CHILD_SEP);
    foreach (const QString& id, ids) {
      auto child = _tree.find(id);
      if (child != _tree.end())
        (*treeItem)->addChild(*child);
    }
  }
}

void Parser::saveCoordinatesFile(const QString& _graphContent)
{
  graphFilename = QDir::tempPath()%"/graphviz-"%QTime().currentTime().toString("hhmmsszzz")%".dot";
  QFile file(graphFilename);
  if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
    utils::alert(QObject::tr("Unable into write the file %1").arg(graphFilename));
    file.close();
    exit(1);
  }
  QTextStream fstream(&file);
  fstream << _graphContent;
  file.close();
}
