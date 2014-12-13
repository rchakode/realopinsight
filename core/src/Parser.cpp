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

const QString Parser::m_dotHeader = "strict graph\n{\n node[shape=plaintext]\n";
const QString Parser::m_dotFooter = "}";


Parser::Parser(const QString& _config, CoreDataT* _cdata)
  : m_config(_config),
    m_cdata(_cdata){}

Parser::~Parser()
{
  QFile dotFile;
  if (dotFile.exists(m_dotFile)) dotFile.remove(m_dotFile);
  if (dotFile.exists(m_dotFile+".plain")) dotFile.remove(m_dotFile+".plain");
  dotFile.close();
}

bool Parser::process(bool console)
{
  QString graphContent = "";
  QDomDocument xmlDoc;
  QDomElement xmlRoot;

  QFile file(m_config);
  if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
    m_lastErrorMsg = QObject::tr("Unable to open the file %1").arg(m_config);
    Q_EMIT errorOccurred(m_lastErrorMsg);
    file.close();
    return false;
  }

  if (!xmlDoc.setContent(&file)) {
    file.close();
    m_lastErrorMsg = QObject::tr("Error while parsing the file %1").arg(m_config);
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

    if (node.type == NodeType::ITService) {
      node.visibility = ngrt4n::Visible;
      StringPairT dataPointInfo = ngrt4n::splitDataPointInfo(node.child_nodes);
      m_cdata->hosts[dataPointInfo.first] << dataPointInfo.second;

      QString srcid = ngrt4n::getSourceIdFromStr(dataPointInfo.first);
      if (srcid.isEmpty()) {
        srcid = ngrt4n::sourceId(0);
        if (console) node.child_nodes = ngrt4n::realCheckId(srcid, node.child_nodes);
      }
      m_cdata->sources.insert(srcid);
      m_cdata->cnodes.insert(node.id, node);
    } else { // i.e. a business service
      node.visibility = ngrt4n::Visible | ngrt4n::Expanded;
      m_cdata->bpnodes.insert(node.id, node);
    }
  }

  updateNodeHierachy(graphContent);
  graphContent = m_dotHeader + graphContent;
  graphContent += m_dotFooter;
  saveCoordinatesFile(graphContent);


  return console ? parseDotResult() : true;
}

void Parser::updateNodeHierachy(QString& _graphContent)
{
  _graphContent = "\n";
  for (NodeListT::ConstIterator node = m_cdata->bpnodes.begin(),
       end = m_cdata->bpnodes.end();
       node != end; ++node) {

    QString nname = node->name;
    _graphContent = "\t"%node->id%"[label=\""%nname.replace(' ', '#')%"\"];\n"%_graphContent;
    if (node->child_nodes != "") {
      QStringList ids = node->child_nodes.split(ngrt4n::CHILD_SEP.c_str());
      Q_FOREACH(const QString& nid, ids) {
        QString nidTrimmed = nid.trimmed();
        auto childNode = m_cdata->cnodes.find(nidTrimmed);
        if (ngrt4n::findNode(m_cdata->bpnodes, m_cdata->cnodes, nidTrimmed, childNode)) {
          childNode->parent = node->id;
          _graphContent += "\t" + node->id%"--"%childNode->id%"\n";
        }
      }
    }
  }

  for (NodeListT::ConstIterator node = m_cdata->cnodes.begin(),
       end = m_cdata->cnodes.end();
       node != end; ++node) {
    QString nname = node->name;
    _graphContent = "\t"%node->id%"[label=\""%nname.replace(' ', '#')%"\"];\n"%_graphContent;
  }
}

void Parser::saveCoordinatesFile(const QString& _content)
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
  fstream << _content;
  file.close();
}

bool Parser::parseDotResult(void)
{
  bool error = false;
  auto process = std::unique_ptr<QProcess>(new QProcess());
  QString plainDotFile = m_dotFile%".plain";
  QStringList arguments = QStringList() << "-Tplain"<< "-o" << plainDotFile << m_dotFile;
  int exitCode = process->execute("dot", arguments);
  process->waitForFinished(60000);
  if (!exitCode) {
    parseDotResult(plainDotFile);
  } else {
    m_lastErrorMsg = QObject::tr("The graph engine exited with the code %1").arg(exitCode);
    Q_EMIT errorOccurred(m_lastErrorMsg);
    error = true;
  }
  process.reset(NULL);

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
