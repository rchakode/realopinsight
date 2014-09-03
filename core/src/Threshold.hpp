#ifndef THRESHOLD_HPP
#define THRESHOLD_HPP

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QDebug>



class Threshold
{
public:
  Threshold(const QString& th);
  Threshold(double weight, int sevIn, int sevOut);
  void parseThreshold(const QString& th);
  double weight(void) const {return m_weight;}
  int sevIn(void) const {return m_sevIn;}
  int sevOut(void) const {return m_sevOut;}
  bool isValid(void) { return isValidWeight(m_weight) && isValidSeverity(m_sevIn) && isValidSeverity(m_sevOut); }
  double toValidWeight(double val) { return isValidWeight(val) ? val : -1.0; }
  int toValidSeverity(int val) { return isValidSeverity(val) ? val : -1; }
  bool isValidWeight(double val) const {return val >= 0.0 && val <= 1;}
  bool isValidSeverity(double val) const {return val >= 0 && val <= 5;}
  QString toString(void) const {return ""; /*TODO*/}
  QString data(void) const {return ""; /*TODO*/}

private:
  double m_weight;
  int m_sevIn;
  int m_sevOut;
};

#endif // THRESHOLD_HPP
