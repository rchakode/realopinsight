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
#include "Utils.hpp"
#include <QObject>

using namespace std;

const QString Parser::CHILD_NODES_SEP = ",";
const QString Parser::dotFileHeader = "strict graph\n{\n node[shape=plaintext]\n";
const QString Parser::dotFileFooter = "}";


Parser::Parser(){}

Parser::~Parser()
{
    QFile dotFile;

    if(dotFile.exists(graphFilename))
        dotFile.remove(graphFilename);

    if(dotFile.exists(graphFilename + ".plain"))
        dotFile.remove(graphFilename + ".plain");
}

bool Parser::parseSvConfig(const QString & _configFile, Struct & _coreData)
{
    QString graphContent;
    QDomDocument xmlDoc;
    QDomElement xmlRoot;
    QFile file(_configFile);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        utils::alert(QObject::tr("Unable to open the file %1").arg(_configFile));
        return false;
    }
    if (! xmlDoc.setContent(&file)) {
        file.close();
        utils::alert(QObject::tr("Error while parsing the file %1").arg(_configFile));
        return false;
    }
    xmlRoot = xmlDoc.documentElement();
    _coreData.monitor = xmlRoot.attribute("monitor").toInt();
    QDomNodeList services = xmlRoot.elementsByTagName("Service");

    qint32 serviceCount = services.length();
    for (qint32 srv = 0; srv < serviceCount; srv++) {
        NodeT node;
        QDomElement service = services.item(srv).toElement();
        node.id = service.attribute("id").trimmed();
        node.type = service.attribute("type").toInt();
        node.status_crule = service.attribute("statusCalcRule").toInt();
        node.status_prule = service.attribute("statusPropRule").toInt();
        node.icon = service.firstChildElement("Icon").text().trimmed();
        node.name = service.firstChildElement("Name").text().trimmed();
        node.description = service.firstChildElement("Description").text().trimmed();
        node.propagation_rule = service.firstChildElement("PropagationRule").text().trimmed();
        node.alarm_msg = service.firstChildElement("AlarmMsg").text().trimmed();
        node.notification_msg = service.firstChildElement("NotificationMsg").text().trimmed();
        node.child_nodes = service.firstChildElement("SubServices").text().trimmed();
        node.criticity = MonitorBroker::NAGIOS_UNKNOWN;
        node.parent = "";

        node.icon.remove("--> "); //FBWC

        if(node.icon.length() == 0) {
            node.icon = GraphView::DEFAULT_ICON;
        }

        if(node.status_crule < 0) {
            node.status_crule = StatusCalcRules::HighCriticity; //FBWC
        }

        if(node.status_prule < 0) {
            node.status_prule = StatusPropRules::Unchanged;
        }

        if(node.type == NodeType::ALARM_NODE) {
            QString host = node.child_nodes.left(node.child_nodes.indexOf("/"));
            _coreData.hosts[host] << node.id;
            _coreData.cnodes.insert(node.id, node);
        } else {
            _coreData.bpnodes.insert(node.id, node);
        }
    }
    file.close();
    updateNodeHierachy(_coreData.bpnodes, _coreData.cnodes, graphContent);
    buildNodeTree(_coreData.bpnodes, _coreData.cnodes, _coreData.tree_items);
    graphContent = dotFileHeader + graphContent;
    graphContent += dotFileFooter;
    saveCoordinatesDotFile(graphContent);

    return true;
}

void Parser::updateNodeHierachy(NodeListT & _bpnodes,
                                NodeListT & _cnodes,
                                QString & _graphContent)
{
    _graphContent = "\n";

    for(NodeListT::iterator node = _bpnodes.begin(); node != _bpnodes.end(); node++) {
        QString nname = node->name;
        _graphContent = "\t"%node->id%"[label=\""%nname.replace(' ', '#')%"\"];\n"%_graphContent;

        if(node->child_nodes != "") {

            QStringList nodeIds = node->child_nodes.split(CHILD_NODES_SEP);
            for(QStringList::iterator nodeId = nodeIds.begin(); nodeId != nodeIds.end(); nodeId ++) {

                QString nidTrimmed = (*nodeId).trimmed();
                NodeListT::iterator childNode = _cnodes.find(nidTrimmed);
                if(childNode != _cnodes.end()) {
                    childNode->parent = node->id;
                    _graphContent += "\t" + node->id%"--"%childNode->id%"\n";
                } else {
                    childNode = _bpnodes.find(nidTrimmed);
                    if(childNode != _bpnodes.end()) {
                        childNode->parent = node->id;
                        _graphContent += "\t" + node->id%"--"%childNode->id%"\n";
                    }
                }
            }
        }
    }

    for(NodeListT::iterator node = _cnodes.begin(); node != _cnodes.end(); node++) {
        QString nname = node->name;
        _graphContent = "\t"%node->id%"[label=\""%nname.replace(' ', '#')%"\"];\n"%_graphContent;
    }
}

void Parser::buildNodeTree(NodeListT & _bpnodes,
                            NodeListT & _cnodes,
                            TreeNodeItemListT & _tree)
{
    for(NodeListT::iterator node = _bpnodes.begin(); node != _bpnodes.end(); node++) {
        _tree.insert(node->id, createTreeItem(*node));
    }
    for(NodeListT::iterator node = _cnodes.begin(); node != _cnodes.end(); node++) {
        _tree.insert(node->id, createTreeItem(*node));
    }

    for(NodeListT::iterator node = _bpnodes.begin(); node != _bpnodes.end(); node++) {

        if(node->type == NodeType::ALARM_NODE ||
                node->child_nodes.length() == 0) {
            continue;
        }

        QStringList childs = node->child_nodes.split(Parser::CHILD_NODES_SEP);
        for(QStringList::iterator childId = childs.begin(); childId != childs.end(); childId++) {

            TreeNodeItemListT::iterator nitem = _tree.find(node->id);
            if (nitem == _tree.end()) {
                utils::alert(QObject::tr("service not found %1").arg(node->name));
                continue;
            }

            TreeNodeItemListT::iterator child = _tree.find(*childId);
            if(child != _tree.end()) {
                _tree[node->id]->addChild(*child);
            }
        }
    }
}

void Parser::saveCoordinatesDotFile(const QString& _graph_content)
{
    graphFilename = QDir::tempPath() + "/graphviz-" + QTime().currentTime().toString("hhmmsszzz") + ".dot";
    QFile file(graphFilename);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
        utils::alert(QObject::tr("Unable into write the file %1").arg(graphFilename));
        exit(1);
    }
    QTextStream file_stream(&file);
    file_stream << _graph_content;
    file.close();
}

QTreeWidgetItem * Parser::createTreeItem(const NodeT & _node){
    QTreeWidgetItem *item = new QTreeWidgetItem(QTreeWidgetItem::UserType);
    item->setIcon(0, QIcon(":/images/built-in/unknown.png"));
    item->setText(0, _node.name);
    item->setData(0, QTreeWidgetItem::UserType, _node.id);

    return item;
}
