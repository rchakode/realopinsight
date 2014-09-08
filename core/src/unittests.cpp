#include "SeverityManager.hpp"
#include <QCoreApplication>
#include <QtTest/QTest>

class TestSeverAggregation : public QObject
{
  Q_OBJECT

public:
  TestSeverAggregation(void)
  {
    m_severityManager = new SeverityManager(QVector<ThresholdT>());
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
  SeverityManager* m_severityManager;
};

void TestSeverAggregation::testAddSeverity(void)
{
  m_severityManager->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Major, 0);
  m_severityManager->addSeverity(ngrt4n::Minor, 3 * ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Major, 2 * ngrt4n::WEIGHT_UNIT);

  QCOMPARE(m_severityManager->count(), 4);
  QCOMPARE(m_severityManager->totalWeight(), 6 * ngrt4n::WEIGHT_UNIT);
  QCOMPARE(m_severityManager->minSev(), Severity(ngrt4n::Normal).value());
  QCOMPARE(m_severityManager->maxSev(), Severity(ngrt4n::Major).value());
}

void TestSeverAggregation::testAverage(void)
{
  QCOMPARE(m_severityManager->aggregatedSeverity(CalcRules::Average), static_cast<int>(ngrt4n::Minor));
}

void TestSeverAggregation::testWeighted(void)
{
  m_severityManager->displayWeight();
  m_severityManager->addThresholdLimit({0.5, ngrt4n::Major, ngrt4n::Critical});
  QCOMPARE(m_severityManager->weightedSeverity(), m_severityManager->averageSeverity());

  m_severityManager->addThresholdLimit({0.5, ngrt4n::Minor, ngrt4n::Major});
  QCOMPARE(m_severityManager->weightedSeverity(), static_cast<int>(ngrt4n::Major));

  m_severityManager->addThresholdLimit({0.3, ngrt4n::Major, ngrt4n::Critical});
  QCOMPARE(m_severityManager->weightedSeverity(), static_cast<int>(ngrt4n::Critical));

//  m_severityManager->addSeverity(ngrt4n::Critical, ngrt4n::WEIGHT_UNIT);
//  m_severityManager->addSeverity(ngrt4n::Unknown, ngrt4n::WEIGHT_UNIT);
}

void TestSeverAggregation::testWorst(void)
{
  QCOMPARE(m_severityManager->aggregatedSeverity(CalcRules::Worst), Severity(ngrt4n::Major).value());
}

QTEST_MAIN(TestSeverAggregation)
#include "unittests.moc"

