#include "Base.hpp"
#include "StatusAggregator.hpp"
#include <cfloat>


StatusAggregator::StatusAggregator(void)
{
  resetData();
}

void StatusAggregator::resetData(void)
{
  m_severityWeightsMap.clear();
  m_thresholdExceededMsg.clear();
  m_statusRatios.clear();
  m_count = 0;
  m_essentialCount = 0;
  m_nonEssentialTotalWeight = 0;
  m_minSeverity = 0;
  m_maxSeverity = 0;
  m_maxEssential = 0;

  m_severityWeightsMap[ngrt4n::Normal] = 0.0;
  m_severityWeightsMap[ngrt4n::Minor] = 0.0;
  m_severityWeightsMap[ngrt4n::Major] = 0.0;
  m_severityWeightsMap[ngrt4n::Critical] = 0.0;
  m_severityWeightsMap[ngrt4n::Unknown] = 0.0;

  m_statusRatios[ngrt4n::Normal] = 0.0;
  m_statusRatios[ngrt4n::Minor] = 0.0;
  m_statusRatios[ngrt4n::Major] = 0.0;
  m_statusRatios[ngrt4n::Critical] = 0.0;
  m_statusRatios[ngrt4n::Unset] = 0.0;

}

void StatusAggregator::addSeverity(int value, double weight)
{
  if (! Severity(value).isValid())
    value = ngrt4n::Unknown;

  if (weight != 0) {
    m_minSeverity = qMin(m_minSeverity, value);
    m_maxSeverity = qMax(m_maxSeverity, value);
    if (weight == ngrt4n::WEIGHT_MAX) {
      m_essentialCount += 1;
      m_maxEssential = qMax(m_maxEssential, value);
    } else {
      m_severityWeightsMap[value] += weight;
      m_nonEssentialTotalWeight += weight;
    }
  }
  updateThresholds();
  ++m_count;
}

void StatusAggregator::addThresholdLimit(QVector<ThresholdT>& thresholdsLimits, const ThresholdT& th)
{
  thresholdsLimits.push_back(th);
  std::sort(thresholdsLimits.begin(), thresholdsLimits.end(), ThresholdLessThanFnt());
}


void StatusAggregator::updateThresholds(void)
{
  if (m_nonEssentialTotalWeight > 0) {
    for (int sev : m_severityWeightsMap.keys()) m_statusRatios[sev] = m_severityWeightsMap[sev] / m_nonEssentialTotalWeight;
  } else {
    for (int sev : m_severityWeightsMap.keys()) m_statusRatios[sev] = DBL_MAX;
  }
}

QString StatusAggregator::toDetailsString(void)
{
  return QObject::tr("Unknown: %1\%; "
                     "Critical: %2\%; "
                     "Major: %3\%; "
                     "Minor: %4\%; "
                     "Normal: %5\%;"
                     "\nComments: %6")
      .arg(QString::number(qRound(100 * m_statusRatios[ngrt4n::Unknown])))
      .arg(QString::number(qRound(100 * m_statusRatios[ngrt4n::Critical])))
      .arg(QString::number(qRound(100 * m_statusRatios[ngrt4n::Major])))
      .arg(QString::number(qRound(100 * m_statusRatios[ngrt4n::Minor])))
      .arg(QString::number(qRound(100 * m_statusRatios[ngrt4n::Normal])))
      .arg(m_thresholdExceededMsg.isEmpty()? "-" : m_thresholdExceededMsg);
}


int StatusAggregator::aggregate(int crule, const QVector<ThresholdT>& thresholdsLimits)
{
  m_thresholdExceededMsg.clear();
  if (m_statusRatios.isEmpty())
    return ngrt4n::Normal;

  int result = ngrt4n::Unknown;
  switch (crule) {
  case CalcRules::Average:
    result = weightedAverage();
    break;
  case CalcRules::WeightedAverageWithThresholds:
    result = weightedAverageWithThresholds(thresholdsLimits);
    break;
  case CalcRules::Worst:
  default:
    result = m_maxSeverity;
    break;
  }

  if (result == m_maxEssential && m_maxEssential != ngrt4n::Normal) {
    QString msg = QObject::tr("Status impacted by problems on essential services");
    if (m_thresholdExceededMsg.isEmpty())
      m_thresholdExceededMsg = msg;
    else
      m_thresholdExceededMsg.append("\n\t").append(msg);
  }
  return result;
}


int StatusAggregator::propagate(int sev, int prule)
{
  qint8 result = static_cast<ngrt4n::SeverityT>(sev);
  Severity sevHelper(static_cast<ngrt4n::SeverityT>(sev));
  switch (prule) {
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

int StatusAggregator::weightedAverage(void)
{
  double severityScore = 0;
  double weightSum = 0;
  for(int sev : m_severityWeightsMap.keys()) {
    double weight = m_severityWeightsMap[sev];
    if (weight > 0) {
      severityScore += weight * static_cast<double>(sev);
      weightSum += weight * ngrt4n::WEIGHT_UNIT;
    }
  }
  return qMax(qRound(severityScore / weightSum), m_maxEssential);
}

int StatusAggregator::weightedAverageWithThresholds(const QVector<ThresholdT>& thresholdsLimits)
{
  int thresholdReached = -1;
  int index = thresholdsLimits.size() - 1;

  while (index >= 0 && thresholdReached == -1) {
    ThresholdT th = thresholdsLimits[index];
    QMap<int, double>::iterator thvalue = m_statusRatios.find(th.sev_in);
    if (thvalue != m_statusRatios.end() && (*thvalue >= th.weight)) {
      thresholdReached = thresholdsLimits[index].sev_out;
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
