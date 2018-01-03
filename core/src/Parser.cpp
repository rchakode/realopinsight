/*
 * Parser.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
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
#include "Base.hpp"
#include "Parser.hpp"
#include "utilsCore.hpp"
#include "ThresholdHelper.hpp"
#include <QObject>
#include <QtXml>
#include <iostream>

const QString Parser::m_dotHeader = "strict graph\n{\n node[shape=plaintext]\n";
const QString Parser::m_dotFooter = "}";


Parser::Parser(const QString& _descriptionFile, CoreDataT* _cdata)
  : m_descriptionFile(_descriptionFile),
    m_cdata(_cdata){}

Parser::~Parser()
{
  QFile fileHandler;
  if (fileHandler.exists(m_dotFile+".plain")) {
    fileHandler.remove(m_dotFile+".plain");
  }
  if (fileHandler.exists(m_dotFile) && m_lastErrorMsg.isEmpty()) {
    fileHandler.remove(m_dotFile);
  }
  fileHandler.close();
}

bool Parser::process(int parsingMode)
{
  m_parsingMode = parsingMode;
  ngrt4n::clearCoreData(*m_cdata);

  m_dotContent.clear();
  QDomDocument xmlDoc;
  QDomElement xmlRoot;


  QFile file(m_descriptionFile);
  if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
    m_lastErrorMsg = QObject::tr("Unable to open the file %1").arg(m_descriptionFile);
    Q_EMIT errorOccurred(m_lastErrorMsg);
    file.close();
    return false;
  }

  if (!xmlDoc.setContent(&file)) {
    file.close();
    m_lastErrorMsg = QObject::tr("Error while parsing the file %1").arg(m_descriptionFile);
    Q_EMIT errorOccurred(m_lastErrorMsg);
    return false;
  }
  file.close(); // The content of the file is already in memory

  xmlRoot = xmlDoc.documentElement();
  m_cdata->monitor = xmlRoot.attribute("monitor").toInt();
  m_cdata->format_version = xmlRoot.attribute("compat").toDouble();
  QDomNodeList services = xmlRoot.elementsByTagName("Service");

  NodeT node;
  qint32 serviceCount = services.length();
  for (qint32 srv = 0; srv < serviceCount; ++srv) {
    QDomElement service = services.item(srv).toElement();
    node.parent.clear();
    node.monitored = false;
    node.id = service.attribute("id").trimmed();
    node.type = service.attribute("type").toInt();
    node.sev = node.sev_prop = ngrt4n::Unknown;
    node.sev_crule = service.attribute("statusCalcRule").toInt();
    node.sev_prule = service.attribute("statusPropRule").toInt();
    node.icon = service.firstChildElement("Icon").text().trimmed();
    node.name = service.firstChildElement("Name").text().trimmed();
    node.description = service.firstChildElement("Description").text().trimmed();
    node.alarm_msg = service.firstChildElement("AlarmMsg").text().trimmed();
    node.notification_msg = service.firstChildElement("NotificationMsg").text().trimmed();
    node.child_nodes = service.firstChildElement("SubServices").text().trimmed();
    node.weight = (m_cdata->format_version >= 3.1) ? service.attribute("weight").toDouble() : ngrt4n::WEIGHT_UNIT;

    if (node.sev_crule == CalcRules::WeightedAverageWithThresholds) {
      QString thdata = service.firstChildElement("Thresholds").text().trimmed();
      node.thresholdLimits = ThresholdHelper::dataToList(thdata);
      qSort(node.thresholdLimits.begin(), node.thresholdLimits.end(), ThresholdLessthanFnt());
    }

    node.check.status = -1;
    if (node.icon.isEmpty()) node.icon = ngrt4n::DEFAULT_ICON;

    switch(node.type) {
      case NodeType::BusinessService:
        insertBusinessServiceNode(node);
        break;
      case NodeType::ITService:
        insertITServiceNode(node);
        break;
      case NodeType::ExternalService:
        insertExternalServiceNode(node);
        break;
      default:
        break;
    }
  }

  updateNodeHierachy();
  saveCoordinatesFile();

  if (m_parsingMode == ParsingModeDashboard)
    return parseDotResult();

  return true;
}


QString Parser::espacedNodeLabel(const QString& rawLabel)
{
  QString label = rawLabel;
  return label.replace("'", " ").replace("-", " ").replace("\"", " ").replace(' ', '#');
}


void Parser::updateNodeHierachy(void)
{
  m_dotContent.append("\n");
  // add business service dependencies
  for (NodeListT::ConstIterator bpnode = m_cdata->bpnodes.begin(),end = m_cdata->bpnodes.end(); bpnode != end; ++bpnode) {
    // Set node label
    m_dotContent.insert(0, QString("\t%1[label=\"%2\"];\n").arg(bpnode->id, espacedNodeLabel(bpnode->name)));

    // create the dependency when applicable
    if (! bpnode->child_nodes.isEmpty()) {
      QStringList childNodeIdList = bpnode->child_nodes.split(ngrt4n::CHILD_SEP.c_str());
      Q_FOREACH(const QString& childNodeId, childNodeIdList) {
        QString childNodeIdTrimmed = childNodeId.trimmed();
        NodeListIteratorT childNode;
        if (ngrt4n::findNode(m_cdata->bpnodes, m_cdata->cnodes, childNodeIdTrimmed, childNode)) {
          childNode->parent = bpnode->id;
          m_dotContent.append(QString("\t%1--%2\n").arg(bpnode->id, childNode->id));
        } else {
          qDebug()<< QObject::tr("Failed to found child dependency for node '%1' => %2").arg(bpnode->id, childNodeIdTrimmed);
        }
      }
    }
  }

  // Set IT service nodes' labels
  for (NodeListT::ConstIterator node = m_cdata->cnodes.begin(), end = m_cdata->cnodes.end(); node != end; ++node) {
    m_dotContent.insert(0, QString("\t%1[label=\"%2\"];\n").arg(node->id, espacedNodeLabel(node->name)));
  }

}

void Parser::saveCoordinatesFile(void)
{
  m_dotFile = QDir::tempPath()%"/graphviz-"%QTime().currentTime().toString("hhmmsszzz")%".dot";
  QFile file(m_dotFile);
  if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
    m_lastErrorMsg = QObject::tr("Unable into write the file %1").arg(m_dotFile);
    Q_EMIT errorOccurred(m_lastErrorMsg);
    file.close();
    exit(1);
  }
  QTextStream fstream(&file);
  fstream << m_dotHeader << m_dotContent << m_dotFooter;
  file.close();
}

bool Parser::parseDotResult(void)
{
  bool error = false;
  QProcess process;
  QString plainDotFile = m_dotFile%".plain";
  QStringList arguments = QStringList() << "-Tplain"<< "-o" << plainDotFile << m_dotFile;
  int exitCode = process.execute("dot", arguments);
  process.waitForFinished(60000);
  if (! exitCode) {
    parseDotResult(plainDotFile);
  } else {
    m_lastErrorMsg = QObject::tr("The graph engine exited on error (code: %1, file: %2").arg(QString::number(exitCode), m_dotFile);
    Q_EMIT errorOccurred(m_lastErrorMsg);
    error = true;
  }
  return ! error;
}


void Parser::parseDotResult(const QString& _plainDot)
{
  QStringList splitedLine;
  QFile qfile(_plainDot);
  if (qfile.open(QFile::ReadOnly)) {
    QRegExp regexSep("[ ]+");
    QTextStream coodFileStream(& qfile);
    QString line;

    //First parse the header
    if(line = coodFileStream.readLine(0), ! line.isNull()) {
      splitedLine = line.split (regexSep);
      m_cdata->map_width = splitedLine[2].trimmed().toFloat() * ngrt4n::XSCAL_FACTOR;
      m_cdata->map_height = splitedLine[3].trimmed().toFloat() * ngrt4n::YSCAL_FACTOR;
    }

    while (line = coodFileStream.readLine(0), ! line.isNull()) {
      splitedLine = line.split (regexSep);
      if (splitedLine[0] == "node") {
        NodeListT::Iterator node;
        QString nid = splitedLine[1].trimmed();
        if (ngrt4n::findNode(m_cdata->bpnodes, m_cdata->cnodes, nid, node)) {
          node->pos_x = splitedLine[2].trimmed().toDouble() * ngrt4n::XSCAL_FACTOR;
          node->pos_y = m_cdata->map_height - splitedLine[3].trimmed().toDouble() * ngrt4n::YSCAL_FACTOR;

          node->text_w = splitedLine[4].trimmed().toDouble() * ngrt4n::XSCAL_FACTOR;
          node->text_h = splitedLine[5].trimmed().toDouble() * ngrt4n::YSCAL_FACTOR;
        }
      } else if (splitedLine[0] == "edge") {
        // multiInsert since a node can have several childs
        m_cdata->edges.insertMulti(splitedLine[1], splitedLine[2]);
      } else if (splitedLine[0] == "stop") {
        break;
      }
    }
    qfile.close();
  }
}


void Parser::insertITServiceNode(NodeT& node)
{
  node.visibility = ngrt4n::Visible;
  StringPairT dataPointInfo = ngrt4n::splitDataPointInfo(node.child_nodes);
  m_cdata->hosts[dataPointInfo.first] << dataPointInfo.second;

  QString srcid = ngrt4n::getSourceIdFromStr(dataPointInfo.first);
  if (srcid.isEmpty()) {
    srcid = ngrt4n::sourceId(0);
    if (m_parsingMode == ParsingModeDashboard) {
      node.child_nodes = ngrt4n::realCheckId(srcid, node.child_nodes);
    }
  }
  m_cdata->sources.insert(srcid);
  m_cdata->cnodes.insert(node.id, node);
}


void Parser::insertBusinessServiceNode(NodeT& node)
{
  node.visibility = ngrt4n::Visible|ngrt4n::Expanded;
  m_cdata->bpnodes.insert(node.id, node);
}


void Parser::insertExternalServiceNode(NodeT& node)
{
  if (m_parsingMode == ParsingModeEditor) {
    m_cdata->bpnodes.insert(node.id, node);
    return; // in editor mode, add the service in the map and return
  }

  QString baseDir = QFileInfo(m_descriptionFile).dir().absolutePath();
  QString path = QString("%1/%2.ms.ngrt4n.xml").arg(baseDir).arg(node.child_nodes);

  CoreDataT cdata;
  Parser parser(path, &cdata);
  if (parser.process(ParsingModeExternamService)) {
    NodeListT::Iterator innerRootNodeIt = cdata.bpnodes.find(ngrt4n::ROOT_ID);

    if (innerRootNodeIt != cdata.bpnodes.end()) {
      // update default id for root node
      innerRootNodeIt->id = node.id;
      innerRootNodeIt->visibility = ngrt4n::Visible|ngrt4n::Expanded;
      NodeT innerRootNode = *innerRootNodeIt; // backup the node content
      cdata.bpnodes.remove(ngrt4n::ROOT_ID);  // Point to innerRootNodeIt, but its key is ngrt4n::ROOT_ID.
                                              // We remove it to avoid duplication when joining the two hashs
      cdata.bpnodes.insert(innerRootNode.id, innerRootNode); // now reinsert the map with its current id
      m_cdata->bpnodes.unite(cdata.bpnodes);
      m_cdata->cnodes.unite(cdata.cnodes);
      m_cdata->hosts.unite(cdata.hosts);
      m_cdata->sources.unite(cdata.sources);
    } else {
      qDebug() << QObject::tr("Invalid graph after parsing external description file: %1").arg(path);
    }
  } else {
    m_lastErrorMsg = parser.lastErrorMsg();
    qDebug() << m_lastErrorMsg;
  }
}
