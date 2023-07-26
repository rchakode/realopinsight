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
#include "K8sHelper.hpp"
#include <QObject>
#include <QtXml>
#include <iostream>
#include <cassert>


Parser::Parser(CoreDataT* _cdata, int _parsingMode, DbSession* dbSession)
  : m_cdata(_cdata),
    m_parsingMode(_parsingMode),
    m_dbSession(dbSession)
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

int Parser::processRenderingData(void)
{
  fixupVisilityAndDependenciesGraph();
  saveCoordinatesFile();
  return computeCoordinates();
}

std::pair<int, QString> Parser::parse(const QString& viewFile)
{
  if (! m_cdata) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("Parser cdata is null"));
  }

  m_cdata->clear();
  QDomDocument xmlDoc;
  QDomElement xmlRoot;
  QFile file(viewFile);
  if (! file.open(QIODevice::ReadOnly|QIODevice::Text)) {
    file.close();
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("Unable to open the file %1").arg(viewFile));
  }

  if (! xmlDoc.setContent(&file)) {
    file.close();
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("Error while parsing the file %1").arg(viewFile));
  }

  file.close(); // The content of the file is already in memory

  xmlRoot = xmlDoc.documentElement();
  m_cdata->monitor = static_cast<qint8>(xmlRoot.attribute("monitor").toInt());
  m_cdata->format_version = xmlRoot.attribute("compat").toDouble();

  QDomNodeList xmlNodes = xmlRoot.elementsByTagName("Service");

  if (m_cdata->monitor != MonitorT::Any) {
    return loadDynamicViewByGroup(xmlNodes, *m_cdata);
  }

  qint32 xmlNodeCount = xmlNodes.size();
  for (qint32 nodeIndex = 0; nodeIndex < xmlNodeCount; ++nodeIndex) {
    QDomElement xmlNode = xmlNodes.item(nodeIndex).toElement();
    NodeT node;
    node.parents.clear();
    node.monitored = false;
    node.id = xmlNode.attribute("id").trimmed();
    node.sev = ngrt4n::Unknown;
    node.sev_prop = ngrt4n::Unknown;
    node.sev_crule = xmlNode.attribute("statusCalcRule").toInt();
    node.sev_prule = xmlNode.attribute("statusPropRule").toInt();
    node.icon = xmlNode.firstChildElement("Icon").text().trimmed();
    node.name = ngrt4n::decodeXml( xmlNode.firstChildElement("Name").text().trimmed() );
    node.description = ngrt4n::decodeXml( xmlNode.firstChildElement("Description").text().trimmed() );
    node.alarm_msg = ngrt4n::decodeXml( xmlNode.firstChildElement("AlarmMsg").text().trimmed() );
    node.notification_msg = ngrt4n::decodeXml( xmlNode.firstChildElement("NotificationMsg").text().trimmed() );
    node.child_nodes = ngrt4n::decodeXml( xmlNode.firstChildElement("SubServices").text().trimmed() );
    node.weight = (m_cdata->format_version >= 3.1) ? xmlNode.attribute("weight").toDouble() : ngrt4n::WEIGHT_UNIT;

    if (node.sev_crule == CalcRules::WeightedAverageWithThresholds) {
      QString thdata = xmlNode.firstChildElement("Thresholds").text().trimmed();
      node.thresholdLimits = ThresholdHelper::dataToList(thdata);
      std::sort(node.thresholdLimits.begin(), node.thresholdLimits.end(), ThresholdLessThanFnt());
    }

    node.check.status = -1;
    if (node.icon.isEmpty()) {
      node.icon = ngrt4n::DEFAULT_ICON;
    }

    node.type = xmlNode.attribute("type").toInt();
    switch(node.type) {
      case NodeType::ITService:
        insertITServiceNode(node);
        break;
      case NodeType::BusinessService:
      case NodeType::ExternalService:
      default:
        m_cdata->bpnodes.insert(node.id, node);
        break;
    }
  }

  // set nodes' parents
  for (const auto& bpnode: m_cdata->bpnodes) {
    for (const auto& childId: bpnode.child_nodes.split(ngrt4n::CHILD_Q_SEP)) {
      NodeListT::Iterator childRef;
      if (ngrt4n::findNode(m_cdata, childId, childRef)) {
        childRef->parents.insert(bpnode.id);
      }
    }
  }

  return std::make_pair(ngrt4n::RcSuccess, "");
}


std::pair<int, QString> Parser::loadDynamicViewByGroup(QDomNodeList& inXmlDomNodes, CoreDataT& outCData)
{
  if (inXmlDomNodes.size() != 1) {
    return std::make_pair(ngrt4n::RcParseError, QObject::tr("unexpected number of items for dynamic view: %1").arg(inXmlDomNodes.size()));
  }

  QDomElement xmlRootNode = inXmlDomNodes.item(0).toElement();
  QString sourceId = xmlRootNode.attribute("id").trimmed();
  QString monitoredGroup = xmlRootNode.firstChildElement("Name").text().trimmed();

  outCData.sources.insert(sourceId);

  auto sourceFound = m_dbSession->findSourceById(sourceId);
  if (! sourceFound.first) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("failed loading source settings on %1").arg(sourceId));
  }

  if (sourceFound.second.mon_type == MonitorT::Kubernetes) {
    auto k8sNsFound = K8sHelper(sourceFound.second.mon_url, sourceFound.second.verify_ssl_peer, sourceFound.second.auth)
                     .loadNamespaceView(monitoredGroup, outCData);
    if (k8sNsFound.second != ngrt4n::RcSuccess) {
      auto m_lastErrorMsg = QObject::tr("%1: %2").arg(sourceFound.second.id, k8sNsFound.first);
      return std::make_pair(k8sNsFound.second, m_lastErrorMsg);
    }
  } else {
    auto viewLoaded = ngrt4n::loadDynamicViewByGroup(sourceFound.second, monitoredGroup, outCData);
    if (viewLoaded.first == ngrt4n::RcSuccess) {
      ngrt4n::fixupDependencies(outCData);
    }
    return viewLoaded;
  }

  return std::make_pair(ngrt4n::RcSuccess, "");
}


QString Parser::escapeLabel4Graphviz(const QString& label)
{
  QString rwLabel = label;
  return rwLabel.replace("'", " ").replace("-", " ").replace("\"", " ").replace(' ', '#').replace(';', '_').replace('&', '_').replace('$', '_');
}

QString Parser::escapeId4Graphviz(const QString& id)
{
  QString rwId = id;
  return rwId.replace("'", " ").replace("-", "_").replace("\"", "_").replace(' ', '_').replace('#', '_');
}



void Parser::fixupVisilityAndDependenciesGraph(void)
{
  m_dotContent = "\n";

  for (auto&& bpnode:  m_cdata->bpnodes) {
    bpnode.visibility = ngrt4n::Visible|ngrt4n::Expanded;
    auto bpnodeGraphId = escapeId4Graphviz(bpnode.id);
    m_dotContent.insert(0, QString("\t%1[label=\"%2\"];\n").arg(bpnodeGraphId, escapeLabel4Graphviz(bpnode.name)));
    bindGraphDependencies(bpnode);
  }

  // Set IT service nodes' labels
  for (auto&& cnode: m_cdata->cnodes) {
    cnode.visibility = ngrt4n::Visible;
    m_dotContent.insert(0, QString("\t%1[label=\"%2\"];\n").arg(escapeId4Graphviz(cnode.id), escapeLabel4Graphviz(cnode.name)));
    bindGraphDependencies(cnode);
  }
}

void Parser::bindGraphDependencies(const NodeT& node)
{
  m_dotContent.append( QString("\tedge [len=%1]\n").arg(NEATO_EDGE_LENGTH) );

  auto nodeGraphId = escapeId4Graphviz(node.id);
  for (const auto& parentId: node.parents) {
    NodeListT::Iterator parentRef;
    if (ngrt4n::findNode(m_cdata, parentId, parentRef)) {
      m_dotContent.append(QString("\t%1--%2\n").arg(escapeId4Graphviz(parentId), nodeGraphId));
    } else {
      if (node.id != ngrt4n::ROOT_ID) {
        qDebug() << QObject::tr("Failed to find parent-child dependency '%1' => %2").arg(parentId, node.id);
      }
    }
  }
}


void Parser::saveCoordinatesFile(void)
{
  m_dotFile = QDir::tempPath()%"/realopinsight-gen-"%QTime().currentTime().toString("hhmmsszzz")%".dot";
  m_plainFile = m_dotFile % ".plain";
  QFile file(m_dotFile);
  if (! file.open(QIODevice::WriteOnly|QIODevice::Text)) {
    m_lastErrorMsg = QObject::tr("Unable into write file %1").arg(m_dotFile);
    file.close();
    exit(1);
  }
  QTextStream fstream(&file);
  fstream << "strict graph\n{\n node[shape=plaintext]\n"
          << m_dotContent
          << "}";
  file.close();
}

int Parser::computeCoordinates(void)
{
  QProcess process;
  QStringList arguments = QStringList() << "-Tplain"<< "-o" << m_plainFile << m_dotFile;

  int exitCode = -2;
  SettingFactory settings;
  switch (settings.getGraphLayout()) {
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
    return ngrt4n::RcGenericFailure;
  }

  QFile qfile(m_plainFile);
  if (! qfile.open(QFile::ReadOnly)) {
    m_lastErrorMsg = QObject::tr("Failed to open file: %1").arg(m_plainFile);
    return ngrt4n::RcGenericFailure;
  }

  //start parsing
  QTextStream coordFileStream(& qfile);
  QString line;
  if(static_cast<void>(line = coordFileStream.readLine(0)), line.isNull()) {
    m_lastErrorMsg = QObject::tr("Failed to read file: %1").arg(m_plainFile);
    return ngrt4n::RcGenericFailure;
  }

  QRegExp regexSep("[ ]+");
  QStringList splitedLine = line.split (regexSep);
  if (splitedLine.size() != 4 || splitedLine[0] != "graph") {
    m_lastErrorMsg = QObject::tr("Invalid graphviz entry: %1").arg(line);
    return ngrt4n::RcGenericFailure;
  }

  const ScaleFactors SCALE_FACTORS(settings.getGraphLayout());
  m_cdata->graph_mode = static_cast<qint8>(settings.getGraphLayout());
  auto maxWidthRaw = splitedLine[2].trimmed().toDouble();
  m_cdata->map_width = maxWidthRaw * SCALE_FACTORS.x() + NEATO_X_TRANSLATION_FACTOR * maxWidthRaw;
  m_cdata->map_height = splitedLine[3].trimmed().toDouble() * SCALE_FACTORS.y();
  m_cdata->min_x = 0;
  m_cdata->min_y = 0;
  double max_text_w = 0;
  double max_text_h = 0;

  int x_index = 2;
  int y_index = 3;

  while (static_cast<void>(line = coordFileStream.readLine(0)), ! line.isNull()) {
    splitedLine = line.split (regexSep);
    if (splitedLine[0] == "node") {
      NodeListT::Iterator node;
      QString nid = splitedLine[1].trimmed();
      if (ngrt4n::findNode(m_cdata, nid, node)) {
        auto posXRaw = splitedLine[x_index].trimmed().toDouble();
        node->pos_x =  posXRaw * SCALE_FACTORS.x() + NEATO_X_TRANSLATION_FACTOR * posXRaw;
        node->pos_y = splitedLine[y_index].trimmed().toDouble() * SCALE_FACTORS.y();

        node->text_w = splitedLine[4].trimmed().toDouble() * SCALE_FACTORS.x();
        node->text_h = splitedLine[5].trimmed().toDouble() * SCALE_FACTORS.y();

        m_cdata->min_x = qMin<double>(m_cdata->min_x, node->pos_x);
        m_cdata->min_y = qMin<double>(m_cdata->min_y, node->pos_y);

        max_text_w = qMax(max_text_w, node->text_w);
        max_text_h = qMax(max_text_h, node->text_h);
      }
    } else if (splitedLine[0] == "edge") {
      // multiInsert because a node can have several children
      m_cdata->edges.insert(splitedLine[1], splitedLine[2]);
    } else if (splitedLine[0] == "stop") {
      break;
    }
  }

  qfile.close();

  if (settings.getGraphLayout() == ngrt4n::NeatoLayout) {
    m_cdata->min_x -= (max_text_w * 0.6);
    m_cdata->min_y -= (max_text_h * 0.6);
  }

  const double MAP_BORDER_HEIGHT = 50.0;
  const double MAP_BORDER_WIDTH = 200;

  m_cdata->min_x = qAbs(m_cdata->min_x) + MAP_BORDER_WIDTH;
  m_cdata->min_y = qAbs(m_cdata->min_y) + MAP_BORDER_HEIGHT;
  m_cdata->map_width += m_cdata->min_x;
  m_cdata->map_height += m_cdata->min_y;

  return ngrt4n::RcSuccess;
}


void Parser::insertITServiceNode(NodeT& node)
{
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




