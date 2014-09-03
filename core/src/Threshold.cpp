#include "Threshold.hpp"

Threshold::Threshold(const QString& th)
  : m_weight(-1.0),
    m_sevIn(-1.0),
    m_sevOut(-1.0)
{
  parseThreshold(th);
}

Threshold::Threshold(double weight, int sevIn, int sevOut)
  : m_weight(weight),
    m_sevIn(sevIn),
    m_sevOut(sevOut)
{
}

void Threshold::parseThreshold(const QString& th)
{
  m_weight = -1.0;
  m_sevIn = -1.0;
  m_sevOut = -1.0;

  QStringList l1 = th.split("=");
  if (l1.size() != 2)
    return;

  QStringList l2 = l1[0].split("x");
  if (l2.size() != 2)
    return;

  bool ok;
  int sev;
  m_weight = toValidWeight(l2[0].toDouble());
  if (sev = l2[1].toInt(&ok), ok)
    m_sevIn = toValidSeverity(sev);

  if (sev = l1[1].toInt(&ok), ok)
    m_sevOut = toValidSeverity(sev);
}
