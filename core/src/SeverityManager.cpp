#include "Base.hpp"
#include "SeverityManager.hpp"


SeverityManager::SeverityManager(const QVector<ThresholdT>& thresholdLimits)
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
  m_thresholdsLimits.clear();
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
    value = ngrt4n::Unknown;

  m_totalWeight += weight;
  m_weights[value] += weight;
  m_minSev =qMin(m_minSev, value);
  m_maxSev = qMax(m_maxSev, value);
  if (weight == ngrt4n::WEIGHT_MAX)
    m_maxEssentialSev = qMax(m_maxEssentialSev, value);

  updateThresholds();
  ++m_count;
}

void SeverityManager::addThresholdLimit(const ThresholdT& th)
{
  m_thresholdsLimits.push_back(th);
  qSort(m_thresholdsLimits.begin(), m_thresholdsLimits.end(), ThresholdLessthanFnt());
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
    result = averageSeverity();
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


int SeverityManager::propagatedSeverity(int sev, int prule)
{
  qint8 result = static_cast<ngrt4n::SeverityT>(sev);
  Severity sevHelper(static_cast<ngrt4n::SeverityT>(sev));
  switch(prule) {
  case PropRules::Increased:
    result = (++sevHelper).value();
    break;
  case PropRules::Decreased:
    result = (--sevHelper).value();
    break;
  default:
    break;
  }
  return result;
}

int SeverityManager::averageSeverity(void)
{
  double severityScore = 0;
  double weightSum = 0;
  Q_FOREACH(int sev, m_weights.keys()) {
    double weight = m_weights[sev];
    if (weight > 0) {
      severityScore += weight * static_cast<double>(sev);
      weightSum += weight * ngrt4n::WEIGHT_UNIT;
    }
  }
  return qRound(severityScore / weightSum);
}

int SeverityManager::weightedSeverity(void)
{
  int thresholdReached = -1;
  int index = m_thresholdsLimits.size() - 1;

  while (index >= 0 && thresholdReached == -1) {
    QMap<int, double>::iterator th = m_thresholds.find(m_thresholdsLimits[index].sev_in);
    if (th != m_thresholds.end() && (*th >= m_thresholdsLimits[index].weight))
      thresholdReached = m_thresholdsLimits[index].sev_out;

    --index;
  }

  if (thresholdReached != -1)
    return qMax(thresholdReached, averageSeverity());

  return averageSeverity();
}
