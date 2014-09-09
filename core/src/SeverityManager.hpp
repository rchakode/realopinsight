#ifndef SEVERITYAGGREGATOR_HPP
#define SEVERITYAGGREGATOR_HPP
#include "Base.hpp"

class SeverityManager
{
public:
  SeverityManager(const QVector<ThresholdT>& thresholdLimits);
  void reset(void);
  void reset(const QVector<ThresholdT>& thresholdLimits) {reset(); m_thresholdsLimits = thresholdLimits;}
  void addSeverity(int value, double weight);
  void addThresholdLimit(const ThresholdT& th);
  QString thresholdsText(void);
  void updateThresholds(void) { Q_FOREACH(int sev, m_weights.keys()) m_thresholds[sev] = m_weights[sev] / m_totalWeight; }
  int aggregatedSeverity(int crule);
  static int propagatedSeverity(int sev, int prule);
  int averageSeverity(void);
  int weightedSeverity(void);
  int minSev(void) const {return m_minSev;}
  int maxSev(void) const {return m_maxSev;}
  int count(void) const {return m_count;}
  double totalWeight(void) const {return m_totalWeight;}
  void displayWeight(void) { Q_FOREACH(int sev, m_weights.keys()) qDebug()<<Severity(sev).toString() <<  m_thresholds[sev]; }

private:
  int m_count;
  double m_totalWeight;
  int m_minSev;
  int m_maxSev;
  int m_maxEssentialSev;
  QMap<int, double> m_weights;
  QMap<int, double> m_thresholds;
  QVector<ThresholdT> m_thresholdsLimits;
};


#endif // SEVERITYAGGREGATOR_HPP
