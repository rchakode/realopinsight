#ifndef SEVERITYAGGREGATOR_HPP
#define SEVERITYAGGREGATOR_HPP
#include "Base.hpp"

class SeverityManager
{
public:
  SeverityManager(const QVector<ThresholdT>& thresholdLimits);
  void reset(void);
  void addSeverity(int aggregatedSeverity, double weight);
  void addThresholdLimit(const ThresholdT& th);
  QString thresholdsToString(void);
  void updateThresholds(void) { Q_FOREACH(int sev, m_weights.keys()) m_thresholds[sev] = m_weights[sev] / m_totalWeight;}

  void displayWeight(void) { Q_FOREACH(int sev, m_weights.keys()) qDebug()<<Severity(sev).toString() <<  m_thresholds[sev];}
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
  QVector<ThresholdT> m_thresholdsLimits;
};


#endif // SEVERITYAGGREGATOR_HPP
