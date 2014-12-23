/*
 * Threshold.cpp
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

#include "ThresholdHelper.hpp"
#include "Base.hpp"

ThresholdHelper::ThresholdHelper(const QString& th)
  : m_weight(-1.0),
    m_sevIn(-1.0),
    m_sevOut(-1.0)
{
  parseThreshold(th);
}

ThresholdHelper::ThresholdHelper(double weight, int sevIn, int sevOut)
  : m_weight( ThresholdHelper::toValidWeight(weight) ),
    m_sevIn( ThresholdHelper::toValidSeverity(sevIn) ),
    m_sevOut( ThresholdHelper::toValidSeverity(sevOut) )
{
}

void ThresholdHelper::parseThreshold(const QString& th)
{
  m_weight = -1.0;
  m_sevIn = -1.0;
  m_sevOut = -1.0;

  QStringList l1 = th.split("=");
  if (l1.size() != 2)
    return;

  QStringList l2 = l1[0].split("x");
  if (l2.size() != 2)
    return;

  bool ok;
  int sev;
  m_weight = toValidWeight(l2[0].toDouble());
  if (sev = l2[1].toInt(&ok), ok)
    m_sevIn = toValidSeverity(sev);

  if (sev = l1[1].toInt(&ok), ok)
    m_sevOut = toValidSeverity(sev);
}


ThresholdT ThresholdHelper::toThreshold(void) const
{
  ThresholdT th;
  th.weight = m_weight;
  th.sev_in = m_sevIn;
  th.sev_out = m_sevOut;
  return th;
}

QString ThresholdHelper::toString(void)
{
  if (isValid())
    return QObject::tr("Set %1\% %2 To %3")
        .arg(QString::number(m_weight * 100), Severity(m_sevIn).toString(), Severity(m_sevOut).toString());
  return QString();
}

QString ThresholdHelper::data(void)
{
  if (isValid())
    return QString("%1x%2=%3"
                   ).arg(QString::number(m_weight), QString::number(m_sevIn), QString::number(m_sevOut));

  return QString();
}


QString ThresholdHelper::listToData(const QVector<ThresholdT>& thresholds)
{
  QString result = "";
  Q_FOREACH(const ThresholdT& th, thresholds) {
    if (! result.isEmpty())
      result.append( ngrt4n::CHILD_SEP.c_str() );

    result.append( ThresholdHelper(th.weight, th.sev_in, th.sev_out).data() );
  }

  return result;
}


QVector<ThresholdT> ThresholdHelper::dataToList(const QString& data)
{
  QVector<ThresholdT> result;
  Q_FOREACH(const QString& entry, data.split(ngrt4n::CHILD_SEP.c_str()))
    result.push_back(ThresholdHelper(entry).toThreshold());

  return result;
}
