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


class Parser : public QObject
{
    Q_OBJECT
  public:
    static const int ParsingModeEditor = 0;
    static const int ParsingModeDashboard = 1;
    static const int ParsingModeExternalService = 2;

  public:
    Parser(const QString& _descriptionFile, CoreDataT* _cdata, int _parsingMode, int _graphLayout);
    virtual ~Parser();
    int process(void);
    int parse(void);
    int computeCoordinates(void);
    QString dotContent(void) const {return m_dotContent;}
    QString dotFile(void) const { return m_dotFile; }
    QString lastErrorMsg(void) const {return m_lastErrorMsg;}

  public Q_SLOTS :
    void handleErrorOccurred(QString msg) { Q_EMIT errorOccurred(msg);}

  Q_SIGNALS:
    void errorOccurred(QString msg);


  private:
    static const QString m_dotHeader;
    static const QString m_dotFooter;
    QString m_dotContent;
    QString m_dotFile;
    QString m_plainFile;
    QString m_descriptionFile;
    CoreDataT* m_cdata;
    QString m_lastErrorMsg;
    int m_parsingMode;
    int m_graphLayout;

    void buildDotContentAndCheckDependencies(void);
    void saveCoordinatesFile(void);
    static QString espacedNodeLabel(const QString& rawLabel);
    void insertITServiceNode(NodeT& node);
    void insertBusinessServiceNode(NodeT& node);
    void insertExternalServiceNode(NodeT& node);
};

#endif /* SNAVPARSESVCONFIG_H_ */
