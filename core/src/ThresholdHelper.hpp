/*
 * Threshold.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 06-09-2014                                                  #
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

#ifndef THRESHOLD_HPP
#define THRESHOLD_HPP

#include "Base.hpp"
#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QVector>


class ThresholdHelper
{
public:
  ThresholdHelper(const QString& th);
  ThresholdHelper(double weight, int sevIn, int sevOut);
  void parseThreshold(const QString& th);
  double weight(void) const {return m_weight;}
  int sevIn(void) const {return m_sevIn;}
  int sevOut(void) const {return m_sevOut;}
  ThresholdT toThreshold(void) const;
  bool isValid(void) { return isValidWeight(m_weight) && Severity(m_sevIn).isValid() && Severity(m_sevOut).isValid(); }
  static double toValidWeight(double val) { return isValidWeight(val) ? val : -1.0; }
  static int toValidSeverity(int val) { return Severity(val).isValid() ? val : -1; }
  static bool isValidWeight(double val) {return val >= 0.0 && val <= 1.0;}
  QString toString(void);
  QString data(void);
  static QString listToData(const QVector<ThresholdT>& ths);
  static QVector<ThresholdT> dataToList(const QString& data);

private:
  double m_weight;
  int m_sevIn;
  int m_sevOut;
};

#endif // THRESHOLD_HPP
