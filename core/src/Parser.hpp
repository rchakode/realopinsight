/*
 * ParseSVConfig.hpp
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


class Parser : public QObject
{
  Q_OBJECT
public:
  Parser(const QString& _config, CoreDataT* _cdata);
  virtual ~Parser();

  bool process(bool console);
  QString dotFile(void) const { return m_dotFile; }
  QString lastError(void) const {return m_lastError;}

Q_SIGNALS:
  void errorOccurred(QString msg);

private:
  static const QString m_dotHeader;
  static const QString m_dotFooter;
  QString m_dotFile;
  QString m_config;
  CoreDataT* m_cdata;
  QString m_lastError;

  void updateNodeHierachy(QString& _graphContent);
  void saveCoordinatesFile(const QString& _content);
  bool computeNodeCoordinates(void);
  void computeNodeCoordinates(const QString& dotfile);
};

#endif /* SNAVPARSESVCONFIG_H_ */
