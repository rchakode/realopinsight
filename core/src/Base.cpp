/*
 * Base.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update: 18-09-2014                                                  #
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

const QString NodeType::ITServiceText = QObject::tr("IT Service");
const QString NodeType::BusinessServiceText = QObject::tr("Business Service");
const QString NodeType::ExternalServiceText = QObject::tr("External Service");
const QString NodeType::K8sNamespaceServiceText = QObject::tr("Kubernetes Namespace Service");

QString PropRules::toString(void) const
{
  switch( static_cast<PropRulesT>(m_rule) ) {
    case Unchanged: return QObject::tr("Unchanged");
    case Decreased: return QObject::tr("Decreased");
    case Increased: return QObject::tr("Increased");
  }
  return QObject::tr("Unchanged");
}


QString CalcRules::toString(void) const
{
  QString result = QObject::tr("Default");
  switch (m_rule) {
    case Average:
      result = QObject::tr("Weighted Average");
      break;
    case WeightedAverageWithThresholds:
      result = QObject::tr("Weighted Average With Thresholds");
      break;
    case Worst:
    default:
      result = QObject::tr("Worst Severity");
      break;
  }
  return result;
}


QString NodeType::toString(int type)
{
  QString result = "";
  switch (type) {
    case ITService:
      result = ITServiceText;
      break;
    case BusinessService:
      result = BusinessServiceText;
      break;
    case ExternalService:
      result = ExternalServiceText;
      break;
    case K8sNamespaceService:
      result = K8sNamespaceServiceText;
      break;
    default:
      result = QObject::tr("Undefined Service Type");
      break;
  }

  return result;
}


int NodeType::toInt(const QString& strType)
{

  int result = -1;
  if (strType == ITServiceText)
    result = ITService;
  else if (strType == ExternalServiceText)
    result = ExternalService;
  else if (strType == BusinessServiceText)
    result = BusinessService;

  return result;
}


QString Severity::toString(void) const
{
  switch(m_sev) {
    case ngrt4n::Normal:
      return QObject::tr("Normal");
      break;
    case ngrt4n::Minor:
      return QObject::tr("Minor");
      break;
    case ngrt4n::Major:
      return QObject::tr("Major");
      break;
    case ngrt4n::Critical:
      return QObject::tr("Critical");
      break;
    default:
      break;
  }
  return QObject::tr("Unknown");
}

std::string Severity::toStdString(void) const
{
  return toString().toStdString();
}


Severity Severity::operator *(Severity& sev) const
{
  switch(m_sev) {
    case ngrt4n::Critical:
      return Severity(m_sev);
      break;
    case ngrt4n::Normal:
      return sev;
      break;
    case ngrt4n::Minor:
      if(sev.m_sev == ngrt4n::Critical ||
         sev.m_sev == ngrt4n::Major ||
         sev.m_sev == ngrt4n::Unknown)
        return sev;
      return Severity(m_sev);
      break;
    case ngrt4n::Major:
      if(sev.m_sev == ngrt4n::Critical ||
         sev.m_sev == ngrt4n::Unknown)
        return sev;
      return Severity(m_sev);
      break;
    default:
      // MonitorBroker::CRITICITY_UNKNOWN
      if(sev.m_sev == ngrt4n::Critical)
        return sev;
      break;
  }  //end switch
  return Severity(ngrt4n::Unknown);
}

Severity Severity::operator / (Severity& st) const
{
  if(m_sev == st.m_sev)
    return st;

  if(m_sev == ngrt4n::Critical ||
     st.m_sev == ngrt4n::Critical)
    return Severity(ngrt4n::Critical);

  if(m_sev == ngrt4n::Unknown ||
     st.m_sev == ngrt4n::Unknown)
    return Severity(ngrt4n::Unknown);

  if(m_sev == ngrt4n::Major ||
     st.m_sev == ngrt4n::Major)
    return Severity(ngrt4n::Major);

  if(m_sev == ngrt4n::Minor ||
     st.m_sev == ngrt4n::Minor)
    return Severity(ngrt4n::Minor);

  return Severity(ngrt4n::Normal);
}


Severity Severity::operator ++()
{
  switch(m_sev) {
    case ngrt4n::Minor:
      return Severity(ngrt4n::Major);
      break;
    case ngrt4n::Major:
      return Severity(ngrt4n::Critical);
      break;
    default://leave as is
      break;
  }
  return Severity(m_sev);
}


Severity Severity::operator--()
{
  switch(m_sev) {
    case ngrt4n::Critical:
      return Severity(ngrt4n::Major);
      break;
    case ngrt4n::Major:
      return Severity(ngrt4n::Minor);
      break;
    default: //leave as is
      break;
  }
  return Severity(m_sev);
}


QString NodeT::toThresholdsString(void) const
{
  QString result = "";
  if (sev_crule == CalcRules::WeightedAverageWithThresholds) {
    Q_FOREACH(const ThresholdT& th, thresholdLimits) {
      result.append(QString("%1\% of %2 => %3; ").arg(QString::number(100 * th.weight),
                                                      Severity(th.sev_in).toString(),
                                                      Severity(th.sev_out).toString()));
    }
    return result;
  }
  return "-";
}

QString NodeT::toString(void) const
{
  QString result = QObject::tr("Service: %1"
                               "\nDescription: %2"
                               "\nSeverity: %3"
                               "\nProp. Rule: %4"
                               "\nWeight: %5"
                               "\nCalc. Rule: %6"
                               "\nThresholds: %9"   // the param  %9 will be filled if details required
                               ).arg(name,
                                     description.isEmpty()? "-" : const_cast<QString&>(description).replace("\n", " "),
                                     Severity(sev).toString(),
                                     PropRules(sev_prule).toString(),
                                     (weight == ngrt4n::WEIGHT_MAX)? QObject::tr("Essential") : QString::number(weight),
                                     CalcRules(sev_crule).toString(),
                                     toThresholdsString());

  if (type == NodeType::ITService) {
    result.append(QObject::tr("\nHost: %1"
                              "\nGroups: %2"
                              "\nData Point: %3"
                              "\nRaw Output: %4"
                              "\nOther Details: %5").arg(QString::fromStdString(check.host).replace("\n", " "),
                                                         (check.host_groups.empty())? "-" : QString::fromStdString(check.host_groups),
                                                         child_nodes.isEmpty()? "-" : child_nodes,
                                                         QString::fromStdString(check.alarm_msg),
                                                         actual_msg));
  } else {
    result.append(QObject::tr("\nAlarm Message: %1").arg(actual_msg));
  }

  return result;
}
