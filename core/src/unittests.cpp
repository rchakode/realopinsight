#include "SeverityManager.hpp"
#include <QCoreApplication>
#include <QtTest/QTest>

class TestSeverityAggregation : public QObject
{
  Q_OBJECT

public:
  TestSeverityAggregation(void)
  {
    m_severityManager = new SeverityManager(QVector<ThresholdT>());
  }

  ~TestSeverityAggregation()
  {
    delete m_severityManager;
  }

private Q_SLOTS:
  void testAddSeverity(void);
  void testAverage(void);
  void testWeighted1(void);
  void testWeighted2(void);
  void testWeighted3(void);
  void testWorst(void);

private:
  SeverityManager* m_severityManager;
};

void TestSeverityAggregation::testAddSeverity(void)
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

void TestSeverityAggregation::testAverage(void)
{
  QCOMPARE(m_severityManager->aggregatedSeverity(CalcRules::Average), static_cast<int>(ngrt4n::Minor));
}

void TestSeverityAggregation::testWeighted1(void)
{
  m_severityManager->displayWeight();
  m_severityManager->addThresholdLimit({0.5, ngrt4n::Major, ngrt4n::Critical});
  QCOMPARE(m_severityManager->weightedSeverity(), m_severityManager->averageSeverity());

  m_severityManager->addThresholdLimit({0.5, ngrt4n::Minor, ngrt4n::Major});
  QCOMPARE(m_severityManager->weightedSeverity(), static_cast<int>(ngrt4n::Major));

  m_severityManager->addThresholdLimit({0.3, ngrt4n::Major, ngrt4n::Critical});
  QCOMPARE(m_severityManager->weightedSeverity(), static_cast<int>(ngrt4n::Critical));
}

void TestSeverityAggregation::testWeighted2(void)
{
  m_severityManager->reset();
  m_severityManager->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);

  m_severityManager->displayWeight();

  m_severityManager->addThresholdLimit({0.75, ngrt4n::Minor, ngrt4n::Major});
  QCOMPARE(m_severityManager->weightedSeverity(), m_severityManager->averageSeverity());
  QCOMPARE(m_severityManager->weightedSeverity(), static_cast<int>(ngrt4n::Minor));

  m_severityManager->addThresholdLimit({0.6, ngrt4n::Minor, ngrt4n::Major});
  QCOMPARE(m_severityManager->weightedSeverity(), static_cast<int>(ngrt4n::Major));

  m_severityManager->addThresholdLimit({0.5, ngrt4n::Major, ngrt4n::Critical});
  QCOMPARE(m_severityManager->weightedSeverity(), static_cast<int>(ngrt4n::Major));
}



void TestSeverityAggregation::testWeighted3(void)
{
  m_severityManager->reset();
  m_severityManager->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);

  m_severityManager->displayWeight();

  m_severityManager->addThresholdLimit({0.6, ngrt4n::Minor, ngrt4n::Major});
  m_severityManager->addThresholdLimit({0.8, ngrt4n::Minor, ngrt4n::Critical});
  QCOMPARE(m_severityManager->weightedSeverity(), static_cast<int>(ngrt4n::Critical));
}

void TestSeverityAggregation::testWorst(void)
{
  m_severityManager->reset();
  m_severityManager->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Major, 3 * ngrt4n::WEIGHT_UNIT);
  m_severityManager->addSeverity(ngrt4n::Unset, 3 * ngrt4n::WEIGHT_UNIT);
  QCOMPARE(m_severityManager->aggregatedSeverity(CalcRules::Worst), static_cast<int>(ngrt4n::Unknown));
}

QTEST_MAIN(TestSeverityAggregation)
#include "unittests.moc"

