#include "Base.hpp"
#include "SeverityManager.hpp"


SeverityManager::SeverityManager(QMap<int, ThresholdT> thresholdLimits)
  : m_count(0),
    m_totalWeight(0),
    m_maxSev(ngrt4n::Normal),
    m_maxEssentialSev(ngrt4n::Normal),
    m_thresholdsLimits(thresholdLimits)
{
  reset();
}

void SeverityManager::reset(void)
{
  m_weights.clear();
  m_thresholds.clear();
  m_count = 0;
  m_totalWeight = 0;

  m_weights[ngrt4n::Normal] = 0.0;
  m_weights[ngrt4n::Minor] = 0.0;
  m_weights[ngrt4n::Major] = 0.0;
  m_weights[ngrt4n::Critical] = 0.0;
  m_weights[ngrt4n::Unknown] = 0.0;

  m_thresholds[ngrt4n::Normal] = 0.0;
  m_thresholds[ngrt4n::Minor] = 0.0;
  m_thresholds[ngrt4n::Major] = 0.0;
  m_thresholds[ngrt4n::Critical] = 0.0;
  m_thresholds[ngrt4n::Unset] = 0.0;

}

void SeverityManager::addSeverity(int value, double weight)
{
  if (! Severity(value).isValid())
    m_weights[ngrt4n::Unknown] += weight;

  m_totalWeight += weight;
  m_weights[value] += weight;
  m_minSev =qMin(m_minSev, value);
  m_maxSev = qMax(m_maxSev, value);
  if (weight == ngrt4n::WEIGHT_MAX)
    m_maxEssentialSev = qMax(m_maxEssentialSev, value);

  updateThresholds();
  ++m_count;
}

QString SeverityManager::thresholdsToString(void)
{
  return QObject::tr(
        "Unknown  : %1 \%\n"
        "Critical : %2 \%\n"
        "Major    : %3 \%\n"
        "Minor    : %4 \%\n"
        "Normal   : %5 \%\n"
        )
      .arg(QString::number(m_thresholds[ngrt4n::Unknown]))
      .arg(QString::number(m_thresholds[ngrt4n::Critical]))
      .arg(QString::number(m_thresholds[ngrt4n::Major]))
      .arg(QString::number(m_thresholds[ngrt4n::Minor]))
      .arg(QString::number(m_thresholds[ngrt4n::Normal]));
}


int SeverityManager::aggregatedSeverity(int crule)
{
  if (m_thresholds.isEmpty())
    return ngrt4n::Normal;

  int result = ngrt4n::Unknown;
  switch (crule) {
  case CalcRules::Average:
    result = (m_maxEssentialSev > m_maxSev)? m_maxEssentialSev : qMax(m_maxEssentialSev, averageSeverity());
    break;
  case CalcRules::Weighted:
    result = weightedSeverity();
    break;
  case CalcRules::Worst:
  default:
    result = m_maxSev;
    break;
  }
  return result;
}

int SeverityManager::averageSeverity(void)
{
  double sumSev = 0;
  double count = 0;
  Q_FOREACH(int sev, m_weights.keys()) {
    double weight = m_weights[sev];
    if (weight > 0) {
      sumSev += weight * static_cast<int>(sev);
      count += weight * ngrt4n::WEIGHT_UNIT;
    }
  }
  return qCeil(sumSev / count);
}

int SeverityManager::weightedSeverity(void)
{
  for (int sev = static_cast<int>(ngrt4n::Unknown); sev >= static_cast<int>(ngrt4n::Normal); --sev) {
    QMap<int, ThresholdT>::ConstIterator thlimit = m_thresholdsLimits.find(sev);
    if (thlimit != m_thresholdsLimits.end()) {
      if (thlimit->weight > 0.0 && m_thresholds[sev] >= thlimit->weight)
        return thlimit->sev_out;
    }
  }

  return averageSeverity();
}
