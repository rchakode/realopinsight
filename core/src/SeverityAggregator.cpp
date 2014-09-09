#include "Base.hpp"
#include "SeverityAggregator.hpp"


SeverityAggregator::SeverityAggregator(const QVector<ThresholdT>& thresholdLimits)
  : m_count(0),
    m_totalWeight(0),
    m_maxSev(ngrt4n::Normal),
    m_maxEssentialSev(ngrt4n::Normal),
    m_thresholdsLimits(thresholdLimits)
{
  reset();
}

void SeverityAggregator::reset(void)
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

void SeverityAggregator::addSeverity(int value, double weight)
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

void SeverityAggregator::addThresholdLimit(const ThresholdT& th)
{
  m_thresholdsLimits.push_back(th);
  qSort(m_thresholdsLimits.begin(), m_thresholdsLimits.end(), ThresholdLessthanFnt());
}

QString SeverityAggregator::toString(void)
{
  return QObject::tr(
        "Unknown: %1\%; Critical: %2\%; Major: %3\%; Minor: %4\%; Normal: %5\%; ")
      .arg(QString::number(100 * m_thresholds[ngrt4n::Unknown]))
      .arg(QString::number(100 * m_thresholds[ngrt4n::Critical]))
      .arg(QString::number(100 * m_thresholds[ngrt4n::Major]))
      .arg(QString::number(100 * m_thresholds[ngrt4n::Minor]))
      .arg(QString::number(100 * m_thresholds[ngrt4n::Normal]));
}


int SeverityAggregator::aggregate(int crule)
{
  m_thresholdExceededMsg.clear();
  if (m_thresholds.isEmpty())
    return ngrt4n::Normal;

  int result = ngrt4n::Unknown;
  switch (crule) {
  case CalcRules::Average:
    result = weightedAverage();
    break;
  case CalcRules::Weighted:
    result = thresholdAverage();
    break;
  case CalcRules::Worst:
  default:
    result = m_maxSev;
    break;
  }
  return result;
}


int SeverityAggregator::propagate(int sev, int prule)
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

int SeverityAggregator::weightedAverage(void)
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

int SeverityAggregator::thresholdAverage(void)
{
  int thresholdReached = -1;
  int index = m_thresholdsLimits.size() - 1;

  while (index >= 0 && thresholdReached == -1) {
    ThresholdT th = m_thresholdsLimits[index];
    QMap<int, double>::iterator thvalue = m_thresholds.find(th.sev_in);
    if (thvalue != m_thresholds.end() && (*thvalue >= th.weight)) {
      thresholdReached = m_thresholdsLimits[index].sev_out;
      m_thresholdExceededMsg = QObject::tr("%1 events exceeded %2\% and set to %3").arg(Severity(th.sev_in).toString(),
                                                                                        QString::number(100 * th.weight),
                                                                                        Severity(th.sev_out).toString()
                                                                                        );
    }
    --index;
  }

  if (thresholdReached != -1)
    return qMax(thresholdReached, weightedAverage());

  return weightedAverage();
}
