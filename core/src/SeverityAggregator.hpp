#ifndef SEVERITYAGGREGATOR_HPP
#define SEVERITYAGGREGATOR_HPP
#include "Base.hpp"

class SeverityAggregator
{
public:
  SeverityAggregator(const QVector<ThresholdT>& thresholdLimits);
  void reset(void);
  void reset(const QVector<ThresholdT>& thresholdLimits) {reset(); m_thresholdsLimits = thresholdLimits;}
  void addSeverity(int value, double weight);
  void addThresholdLimit(const ThresholdT& th);
  QString toString(void);
  void updateThresholds(void) { Q_FOREACH(int sev, m_weights.keys()) m_thresholds[sev] = m_weights[sev] / m_totalWeight; }
  int aggregate(int crule);
  static int propagate(int sev, int prule);
  int weightedAverage(void);
  int thresholdAverage(void);
  int minSev(void) const {return m_minSev;}
  int maxSev(void) const {return m_maxSev;}
  int count(void) const {return m_count;}
  double totalWeight(void) const {return m_totalWeight;}
  void displayWeight(void) { Q_FOREACH(int sev, m_weights.keys()) qDebug()<<Severity(sev).toString() <<  m_thresholds[sev]; }
  QString thresholdExceededMsg(void) const {return m_thresholdExceededMsg;}

private:
  int m_count;
  double m_totalWeight;
  int m_minSev;
  int m_maxSev;
  int m_maxEssentialSev;
  QString m_thresholdExceededMsg;
  QMap<int, double> m_weights;
  QMap<int, double> m_thresholds;
  QVector<ThresholdT> m_thresholdsLimits;
};


#endif // SEVERITYAGGREGATOR_HPP
