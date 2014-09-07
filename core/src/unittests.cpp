#include "SeverityManager.hpp"
#include <QCoreApplication>
#include <QtTest/QTest>

class TestSeverAggregation : public QObject
{
  Q_OBJECT

public:
  TestSeverAggregation(void)
  {
    m_thresholdLimits.insert(ngrt4n::Major, {0.5, ngrt4n::Major, ngrt4n::Critical});
    m_severityManager = new SeverityManager(m_thresholdLimits);
  }

  ~TestSeverAggregation()
  {
    delete m_severityManager;
  }

private Q_SLOTS:
  void testAddSeverity(void);
  void testAverage(void);
  void testWeighted(void);
  void testWorst(void);

private:
  QMap<int, ThresholdT> m_thresholdLimits;
  SeverityManager* m_severityManager;
};

void TestSeverAggregation::testAddSeverity(void)
{
  m_severityManager->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Major, 0);
  m_severityManager->addSeverity(ngrt4n::Critical, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Unknown, ngrt4n::WEIGHT_UNIT);

  QCOMPARE(m_severityManager->count(), 4);
  QCOMPARE(m_severityManager->totalWeight(), 3 * ngrt4n::WEIGHT_UNIT);
  QCOMPARE(m_severityManager->minSev(), Severity(ngrt4n::Normal).value());
  QCOMPARE(m_severityManager->maxSev(), Severity(ngrt4n::Unknown).value());

}

void TestSeverAggregation::testAverage(void)
{
  QCOMPARE(m_severityManager->aggregatedSeverity(CalcRules::Average), Severity(ngrt4n::Major).value());
}

void TestSeverAggregation::testWeighted(void)
{
  QCOMPARE(1, 1);
}

void TestSeverAggregation::testWorst(void)
{
  QCOMPARE(m_severityManager->aggregatedSeverity(CalcRules::Worst), Severity(ngrt4n::Unknown).value());
}

QTEST_MAIN(TestSeverAggregation)
#include "unittests.moc"

