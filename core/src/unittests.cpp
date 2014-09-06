#include "SeverityManager.hpp"
#include <QCoreApplication>
#include <QtTest/QTest>

class TestSeverAggregation : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void testAddSeverity(void);
  void testAverage(void);
  void testWeighted(void);
  void testWorst(void);
};

void TestSeverAggregation::testAddSeverity(void)
{
  QMap<int, ThresholdT> thresholdLimits;
  thresholdLimits.insert(ngrt4n::Major, {0.5, ngrt4n::Major, ngrt4n::Critical});
  SeverityManager sm(thresholdLimits);

  sm.addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);
  sm.addSeverity(ngrt4n::Major, 0);
  sm.addSeverity(ngrt4n::Critical, ngrt4n::WEIGHT_UNIT);
  sm.addSeverity(ngrt4n::Unknown, ngrt4n::WEIGHT_UNIT);

  QCOMPARE(sm.count(), 4);
  QCOMPARE(sm.totalWeight(), 3 * ngrt4n::WEIGHT_UNIT);
  QCOMPARE(sm.minSev(), Severity(ngrt4n::Normal).value());
  QCOMPARE(sm.maxSev(), Severity(ngrt4n::Unknown).value());

  QCOMPARE(sm.aggregatedSeverity(CalcRules::Worst), Severity(ngrt4n::Unknown).value());
  QCOMPARE(sm.aggregatedSeverity(CalcRules::Average), Severity(ngrt4n::Critical).value());
}

void TestSeverAggregation::testAverage(void)
{
  QMap<int, ThresholdT> thresholdLimits;
  thresholdLimits.insert(ngrt4n::Major, {0.5, ngrt4n::Major, ngrt4n::Critical});
  SeverityManager sm(thresholdLimits);
  sm.addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);

  QCOMPARE(1, 1);
}

void TestSeverAggregation::testWeighted(void)
{
 QCOMPARE(1, 1);
}

void TestSeverAggregation::testWorst(void)
{

  QCOMPARE(1, 1);
}

QTEST_MAIN(TestSeverAggregation)
#include "unittests.moc"

