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


Parser::Parser(const QString& _descriptionFile, CoreDataT* _cdata, int _parsingMode, int _graphLayout)
  : m_descriptionFile(_descriptionFile),
    m_cdata(_cdata),
    m_parsingMode(_parsingMode),
    m_graphLayout(_graphLayout)
{

}


Parser::~Parser()
{
  QFile fileHandler;
  if (fileHandler.exists(m_plainFile)) {
    fileHandler.remove(m_plainFile);
  }
  if (fileHandler.exists(m_dotFile) && m_lastErrorMsg.isEmpty()) {
    fileHandler.remove(m_dotFile);
  }
  fileHandler.close();
}

int Parser::process(void)
{
  int rc = parse();
  if (rc == 0) {
    fixParentChildDependenciesAndBuildDotContent();
    saveCoordinatesFile();
    rc = computeCoordinates();
  }

  return rc;
}

int Parser::parse(void)
{
  m_cdata->clear();

  m_dotContent.clear();
  QDomDocument xmlDoc;
  QDomElement xmlRoot;


  QFile file(m_descriptionFile);
  if (! file.open(QIODevice::ReadOnly|QIODevice::Text)) {
    m_lastErrorMsg = QObject::tr("Unable to open the file %1").arg(m_descriptionFile);
    file.close();
    return -1;
  }

  if (!xmlDoc.setContent(&file)) {
    file.close();
    m_lastErrorMsg = QObject::tr("Error while parsing the file %1").arg(m_descriptionFile);
    return -1;
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
    if (node.icon.isEmpty()) {
      node.icon = ngrt4n::DEFAULT_ICON;
    }

    node.type = service.attribute("type").toInt();
    switch(node.type) {
      case NodeType::ITService:
        insertITServiceNode(node);
        break;
      case NodeType::BusinessService:
      case NodeType::ExternalService:
        insertBusinessServiceNode(node);
        break;
      default:
        node.type = NodeType::BusinessService;
        insertBusinessServiceNode(node);
        break;
    }
  }

  return 0;
}


QString Parser::escapeLabel(const QString& label)
{
  QString rwLabel = label;
  return rwLabel.replace("'", " ").replace("-", " ").replace("\"", " ").replace(' ', '#');
}

QString Parser::escapeId(const QString& id)
{
  QString rwId = id;
  return rwId.replace("'", " ").replace("-", "_").replace("\"", "_").replace(' ', '_').replace('#', '_');
}



void Parser::fixParentChildDependenciesAndBuildDotContent(void)
{
  m_dotContent.append("\n");

  for (const auto& bpnode:  m_cdata->bpnodes) {
    auto graphParentId = escapeId(bpnode.id);

    m_dotContent.insert(0, QString("\t%1[label=\"%2\"];\n").arg(graphParentId, escapeLabel(bpnode.name)));

    if (bpnode.type == NodeType::ExternalService) {
      continue;
    }

    if (! bpnode.child_nodes.isEmpty()) {
      QStringList children = bpnode.child_nodes.split(ngrt4n::CHILD_SEP.c_str());
      for(const auto& childId: children) {
        NodeListIteratorT childIt;
        if (ngrt4n::findNode(m_cdata->bpnodes, m_cdata->cnodes, childId, childIt)) {
          childIt->parent = bpnode.id;
          auto graphChildId = escapeId(childIt->id);
          m_dotContent.append(QString("\t%1--%2\n").arg(graphParentId, graphChildId));
        } else {
          qDebug() << QObject::tr("Failed to find parent-child dependency'%1' => %2").arg(bpnode.id, childId);
        }
      }

    }
  }

  // Set IT service nodes' labels
  for (const auto& cnode: m_cdata->cnodes) {
    auto graphId = escapeId(cnode.id);
    m_dotContent.insert(0, QString("\t%1[label=\"%2\"];\n").arg(graphId, escapeLabel(cnode.name)));
  }

}

void Parser::saveCoordinatesFile(void)
{
  m_dotFile = QDir::tempPath()%"/realopinsight-gen-"%QTime().currentTime().toString("hhmmsszzz")%".dot";
  m_plainFile = m_dotFile % ".plain";
  QFile file(m_dotFile);
  if (! file.open(QIODevice::WriteOnly|QIODevice::Text)) {
    m_lastErrorMsg = QObject::tr("Unable into write the file %1").arg(m_dotFile);
    file.close();
    exit(1);
  }
  QTextStream fstream(&file);
  fstream << m_dotHeader << m_dotContent << m_dotFooter;
  file.close();
}

int Parser::computeCoordinates(void)
{
  QProcess process;
  QStringList arguments = QStringList() << "-Tplain"<< "-o" << m_plainFile << m_dotFile;

  int exitCode = -2;
  switch (m_graphLayout) {
    case ngrt4n::DotLayout:
      exitCode = process.execute("dot", arguments);
      break;
    case ngrt4n::NeatoLayout: //use dot as default
    default:
      exitCode = process.execute("neato", arguments);
      break;
  }

  process.waitForFinished(60000);
  if (exitCode != 0) {
    m_lastErrorMsg = QObject::tr("The graph engine exited on error (code: %1, file: %2").arg(QString::number(exitCode), m_dotFile);
    return -1;
  }

  QFile qfile(m_plainFile);
  if (! qfile.open(QFile::ReadOnly)) {
    m_lastErrorMsg = QObject::tr("Failed to open file: %1").arg(m_plainFile);
    return -1;
  }

  //start parsing
  QTextStream coodFileStream(& qfile);
  QString line;
  if(line = coodFileStream.readLine(0), line.isNull()) {
    m_lastErrorMsg = QObject::tr("Failed to read file: %1").arg(m_plainFile);
    return -1;
  }

  QRegExp regexSep("[ ]+");
  QStringList splitedLine = line.split (regexSep);
  if (splitedLine.size() != 4 || splitedLine[0] != "graph") {
    m_lastErrorMsg = QObject::tr("Invalid graphviz entry: %1").arg(line);
    return -1;
  }

  const ScaleFactors SCALE_FACTORS(m_graphLayout);
  m_cdata->graph_mode = m_graphLayout;
  m_cdata->map_width = splitedLine[2].trimmed().toDouble() * SCALE_FACTORS.x();
  m_cdata->map_height = splitedLine[3].trimmed().toDouble() * SCALE_FACTORS.y();
  m_cdata->min_x = 0;
  m_cdata->min_y = 0;
  double max_text_w = 0;
  double max_text_h = 0;

  int x_index = 2;
  int y_index = 3;

  while (line = coodFileStream.readLine(0), ! line.isNull()) {
    splitedLine = line.split (regexSep);
    if (splitedLine[0] == "node") {
      NodeListT::Iterator node;
      QString nid = splitedLine[1].trimmed();
      if (ngrt4n::findNode(m_cdata->bpnodes, m_cdata->cnodes, nid, node)) {
        node->pos_x = splitedLine[x_index].trimmed().toDouble() * SCALE_FACTORS.x();
        node->pos_y =  splitedLine[y_index].trimmed().toDouble() * SCALE_FACTORS.y();
        node->text_w = splitedLine[4].trimmed().toDouble() * SCALE_FACTORS.x();
        node->text_h = splitedLine[5].trimmed().toDouble() * SCALE_FACTORS.y();
        m_cdata->min_x = qMin<double>(m_cdata->min_x, node->pos_x);
        m_cdata->min_y = qMin<double>(m_cdata->min_y, node->pos_y);
        max_text_w = qMax(max_text_w, node->text_w);
        max_text_h = qMax(max_text_h, node->text_h);
      }
    } else if (splitedLine[0] == "edge") {
      // multiInsert because a node can have several childs
      m_cdata->edges.insertMulti(splitedLine[1], splitedLine[2]);
    } else if (splitedLine[0] == "stop") {
      break;
    }
  }

  qfile.close();

  if (m_graphLayout == ngrt4n::NeatoLayout) {
    m_cdata->min_x -= (max_text_w * 0.6);
    m_cdata->min_y -= (max_text_h * 0.6);
  }

  const double MAP_BORDER_HEIGHT = 50.0;
  const double MAP_BORDER_WIDTH = 200;

  m_cdata->min_x = qAbs(m_cdata->min_x) + MAP_BORDER_WIDTH;
  m_cdata->min_y = qAbs(m_cdata->min_y) + MAP_BORDER_HEIGHT;
  m_cdata->map_width += m_cdata->min_x;
  m_cdata->map_height += m_cdata->min_y;

  return 0;
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




