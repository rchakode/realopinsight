/*
 * Parser.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 23-05-2014                                                  #
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

#ifndef SNAVPARSESVCONFIG_H_
#define SNAVPARSESVCONFIG_H_

#include "Base.hpp"
#include "utilsCore.hpp"
#include "SettingFactory.hpp"
#include "DbSession.hpp"
#include <QDomNodeList>


class Parser : public QObject
{
    Q_OBJECT
  public:
    static const int ParsingModeEditor = 0;
    static const int ParsingModeDashboard = 1;
    static const int ParsingModeExternalService = 2;

  public:
    Parser(CoreDataT* _cdata, int _parsingMode, DbSession* dbSession);
    virtual ~Parser();
    int processRenderingData(void);
    std::pair<int, QString> parse(const QString& viewFile);
    int computeCoordinates(void);
    QString dotContent(void) const {return m_dotContent;}
    QString dotFile(void) const { return m_dotFile; }
    QString lastErrorMsg(void) const {return m_lastErrorMsg;}
    static QString escapeLabel4Graphviz(const QString& label);
    static QString escapeId4Graphviz(const QString& id);


  private:
    QString m_dotContent;
    QString m_dotFile;
    QString m_plainFile;
    CoreDataT* m_cdata;
    QString m_lastErrorMsg;
    int m_parsingMode;
    DbSession* m_dbSession;


    void fixupVisibilityAndDependenciesGraph(void);
    void saveCoordinatesFile(void);
    void insertITServiceNode(NodeT& node);
    std::pair<int, QString> loadDynamicViewByGroup(QDomNodeList& inXmlDomNodes, CoreDataT& outCData);
    void bindGraphDependencies(const NodeT& node);
};

#endif /* SNAVPARSESVCONFIG_H_ */
