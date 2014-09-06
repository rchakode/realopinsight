#ifndef SEVERITYAGGREGATOR_HPP
#define SEVERITYAGGREGATOR_HPP
#include "Base.hpp"

class SeverityManager
{
public:
  SeverityManager(QMap<int, ThresholdT> thresholdLimits);
  void reset(void);
  void addSeverity(int aggregatedSeverity, double weight);
  QString thresholdsToString(void);
  void updateThresholds(void) { Q_FOREACH(int sev, m_weights.keys()) m_thresholds[sev] = m_weights[sev] / m_totalWeight;}
  int aggregatedSeverity(int crule);
  int averageSeverity(void);
  int weightedSeverity(void);
  int minSev(void) const {return m_minSev;}
  int maxSev(void) const {return m_maxSev;}
  int count(void) const {return m_count;}
  double totalWeight(void) const {return m_totalWeight;}
private:
  int m_count;
  double m_totalWeight;
  int m_minSev;
  int m_maxSev;
  int m_maxEssentialSev;
  QMap<int, double> m_weights;
  QMap<int, double> m_thresholds;
  QMap<int, ThresholdT> m_thresholdsLimits;
};


#endif // SEVERITYAGGREGATOR_HPP
