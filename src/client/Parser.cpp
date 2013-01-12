/*
 * Parser.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
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

bool Parser::parseSvConfig(const QString& _configFile, CoreDataT& _coreData)
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
  _coreData.monitor = xmlRoot.attribute("monitor").toInt();
  QDomNodeList services = xmlRoot.elementsByTagName("Service");

  NodeT node;
  qint32 serviceCount = services.length();
  for (qint32 srv = 0; srv < serviceCount; srv++) {
      QDomElement service = services.item(srv).toElement();
      node.id = service.attribute("id").trimmed();
      node.type = service.attribute("type").toInt();
      node.criticity_crule = service.attribute("statusCalcRule").toInt();
      node.criticity_prule = service.attribute("statusPropRule").toInt();
      node.icon = service.firstChildElement("Icon").text().trimmed();
      node.name = service.firstChildElement("Name").text().trimmed();
      node.description = service.firstChildElement("Description").text().trimmed();
      node.propagation_rule = service.firstChildElement("PropagationRule").text().trimmed();
      node.alarm_msg = service.firstChildElement("AlarmMsg").text().trimmed();
      node.notification_msg = service.firstChildElement("NotificationMsg").text().trimmed();
      node.child_nodes = service.firstChildElement("SubServices").text().trimmed();
      node.criticity = MonitorBroker::NagiosUnknown;
      node.parent = "";
      node.icon.remove("--> "); //FBWC
      if (node.icon.length() == 0) {
          node.icon = GraphView::DEFAULT_ICON;
        }
      if (node.criticity_crule < 0) {
          node.criticity_crule = CalcRules::HighCriticity; //FBWC
        }
      if (node.criticity_prule < 0) {
          node.criticity_prule = PropRules::Unchanged;
        }
      if (node.type == NodeType::ALARM_NODE) {
          QString host = node.child_nodes.left(node.child_nodes.indexOf("/"));
          _coreData.hosts[host] << node.id;
          _coreData.cnodes.insert(node.id, node);
        } else {
          _coreData.bpnodes.insert(node.id, node);
        }
    }
  updateNodeHierachy(_coreData.bpnodes, _coreData.cnodes, graphContent);
  buildNodeTree(_coreData.bpnodes, _coreData.cnodes, _coreData.tree_items);
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
  for (auto node : _bpnodes) {
      QString nname = node.name;
      _graphContent = "\t"%node.id%"[label=\""%nname.replace(' ', '#')%"\"];\n"%_graphContent;
      if (node.child_nodes != "") {
          QStringList nodeIds = node.child_nodes.split(CHILD_SEP);
          for (auto nodeId : nodeIds) {
              QString nidTrimmed = nodeId.trimmed();
              auto childNode = _cnodes.find(nidTrimmed);
              if (utils::findNode(_bpnodes, _cnodes, nidTrimmed, childNode)) {
                  childNode->parent = node.id;
                  _graphContent += "\t" + node.id%"--"%childNode->id%"\n";
                }
            }
        }
    }

  for (auto node : _cnodes) {
      QString nname = node.name;
      _graphContent = "\t"%node.id%"[label=\""%nname.replace(' ', '#')%"\"];\n"%_graphContent;
    }
}

void Parser::buildNodeTree(const NodeListT& _bpnodes,
                           const NodeListT& _cnodes,
                           TreeNodeItemListT& _tree)
{
  for (auto node : _bpnodes) _tree.insert(node.id, SvNavigatorTree::createTreeItem(node));
  for (auto node : _cnodes) _tree.insert(node.id, SvNavigatorTree::createTreeItem(node));
  for (auto node : _bpnodes) {
      if (node.child_nodes.isEmpty()) continue;
      for (auto childId : node.child_nodes.split(Parser::CHILD_SEP)) {
          auto treeItem = _tree.find(node.id);
          if (treeItem == _tree.end()) {
              utils::alert(QObject::tr("Service not found %1").arg(node.name));
              continue;
            }
          auto child = _tree.find(childId);
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
