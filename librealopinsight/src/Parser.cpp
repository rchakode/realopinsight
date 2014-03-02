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

#include "global.hpp"
#include "Parser.hpp"
#include "utilsClient.hpp"
#include <QObject>

using namespace std;

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
  QString graphContent ="";
  QDomDocument xmlDoc;
  QDomElement xmlRoot;

  QFile file(m_config);
  if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
    m_lastError = QObject::tr("Unable to open the file %1").arg(m_config);
    Q_EMIT errorOccurred(m_lastError);
    file.close();
    return false;
  }

  if (!xmlDoc.setContent(&file)) {
    file.close();
    m_lastError = QObject::tr("Error while parsing the file %1").arg(m_config);
    Q_EMIT errorOccurred(m_lastError);
    return false;
  }
  file.close(); // The content of the file is already in memory

  xmlRoot = xmlDoc.documentElement();
  m_cdata->monitor = xmlRoot.attribute("monitor").toInt();
  QDomNodeList services = xmlRoot.elementsByTagName("Service");

  NodeT node;
  qint32 serviceCount = services.length();
  for (qint32 srv = 0; srv < serviceCount; ++srv) {
    QDomElement service = services.item(srv).toElement();
    node.id = service.attribute("id").trimmed();
    node.parent = "";
    node.monitored = false;
    node.type = service.attribute("type").toInt();
    node.severity = ngrt4n::Unknown;
    node.sev_crule = service.attribute("statusCalcRule").toInt();
    node.sev_prule = service.attribute("statusPropRule").toInt();
    node.icon = service.firstChildElement("Icon").text().trimmed();
    node.name = service.firstChildElement("Name").text().trimmed();
    node.description = service.firstChildElement("Description").text().trimmed();
    node.alarm_msg = service.firstChildElement("AlarmMsg").text().trimmed();
    node.notification_msg = service.firstChildElement("NotificationMsg").text().trimmed();
    node.child_nodes = service.firstChildElement("SubServices").text().trimmed();

    if (node.icon.isEmpty()) {
      node.icon = ngrt4n::DEFAULT_ICON;
    }
    if (node.type == NodeType::AlarmNode) {
      node.visibility = ngrt4n::Visible;
      StringPairT info = ngrt4n::splitHostCheckInfo(node.child_nodes);
      m_cdata->hosts[info.first] << info.second;

      QString srcid = ngrt4n::getSourceIdFromStr(info.first);

      if (srcid.isEmpty()) {
        QString srcid = ngrt4n::sourceId(0);
        if (console) {
          node.child_nodes = ngrt4n::realCheckId(srcid, node.child_nodes);
        }
        m_cdata->sources.insert(srcid);
      } else {
        m_cdata->sources.insert(srcid);
      }
      m_cdata->cnodes.insert(node.id, node);
    } else { // means a BP node
      node.visibility = ngrt4n::Visible | ngrt4n::Expanded;
      m_cdata->bpnodes.insert(node.id, node);
    }
  }

  m_cdata->root = m_cdata->bpnodes.find(ngrt4n::ROOT_ID);
  if (m_cdata->root == m_cdata->bpnodes.end()) {
    Q_EMIT errorOccurred(tr("The configuration is not valid, there is no root service !"));
    return false;
  }

  updateNodeHierachy(graphContent);
  graphContent = m_dotHeader + graphContent;
  graphContent += m_dotFooter;
  saveCoordinatesFile(graphContent);

  return computeNodeCoordinates();
}

void Parser::updateNodeHierachy(QString& _graphContent)
{
  _graphContent = "\n";
  for (NodeListT::ConstIterator node = m_cdata->bpnodes.begin();
       node != m_cdata->bpnodes.end(); ++node)
  {
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

  for (NodeListT::ConstIterator node = m_cdata->cnodes.begin(), end = m_cdata->cnodes.end();
       node != end; ++node)
  {
    QString nname = node->name;
    _graphContent = "\t"%node->id%"[label=\""%nname.replace(' ', '#')%"\"];\n"%_graphContent;
  }
}

void Parser::saveCoordinatesFile(const QString& _content)
{
  m_dotFile = QDir::tempPath()%"/graphviz-"%QTime().currentTime().toString("hhmmsszzz")%".dot";
  QFile file(m_dotFile);
  if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
    m_lastError = QObject::tr("Unable into write the file %1").arg(m_dotFile);
    Q_EMIT errorOccurred(m_lastError);
    file.close();
    exit(1);
  }
  QTextStream fstream(&file);
  fstream << _content;
  file.close();
}

bool Parser::computeNodeCoordinates(void)
{
  bool error = false;
  auto process = std::unique_ptr<QProcess>(new QProcess());
  QString plainDotFile = m_dotFile%".plain";
  QStringList arguments = QStringList() << "-Tplain"<< "-o" << plainDotFile << m_dotFile;
  int exitCode = process->execute("dot", arguments);
  process->waitForFinished(60000);
  if (!exitCode) {
    computeNodeCoordinates(plainDotFile);
  } else {
    m_lastError = QObject::tr("The graph engine exited with the code %1").arg(exitCode);
    Q_EMIT errorOccurred(m_lastError);
    error = true;
  }
  process.reset(NULL);

  return ! error;
}

void Parser::computeNodeCoordinates(const QString& _plainDot)
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
      m_cdata->map_width = splitedLine[2].trimmed().toFloat() * XSCAL_FACTOR;
      m_cdata->map_height = splitedLine[3].trimmed().toFloat() * YSCAL_FACTOR;
    }

    while (line = coodFileStream.readLine(0), ! line.isNull()) {
      splitedLine = line.split (regexSep);
      if (splitedLine[0] == "node") {
        NodeListT::Iterator node;
        QString nid = splitedLine[1].trimmed();
        if (ngrt4n::findNode(m_cdata->bpnodes, m_cdata->cnodes, nid, node)) {
          node->pos_x = splitedLine[2].trimmed().toFloat() * XSCAL_FACTOR;
          node->pos_y = m_cdata->map_height - splitedLine[3].trimmed().toFloat() * YSCAL_FACTOR;
        }
      } else if (splitedLine[0] == "edge") {
        m_cdata->edges.insertMulti(splitedLine[1], splitedLine[2]);
      } else if (splitedLine[0] == "stop") {
        break;
      }
    }
    qfile.close();
  }
}
