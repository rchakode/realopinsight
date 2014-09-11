#include "SeverityAggregator.hpp"
#include <QCoreApplication>
#include <QtTest/QTest>

class TestSeverityAggregation : public QObject
{
  Q_OBJECT

public:
  TestSeverityAggregation(void)
  {
    m_severityAggregator = new SeverityAggregator(QVector<ThresholdT>());
  }

  ~TestSeverityAggregation()
  {
    delete m_severityAggregator;
  }

private Q_SLOTS:
  void testAddSeverity(void);
  void testAverage(void);
  void testWeighted1(void);
  void testWeighted2(void);
  void testWeighted3(void);
  void testWeighted4(void);
  void testWorst(void);

private:
  SeverityAggregator* m_severityAggregator;
};

void TestSeverityAggregation::testAddSeverity(void)
{
  m_severityAggregator->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Major, 0);
  m_severityAggregator->addSeverity(ngrt4n::Minor, 3 * ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Major, 2 * ngrt4n::WEIGHT_UNIT);

  QCOMPARE(m_severityAggregator->count(), 4);
  QCOMPARE(m_severityAggregator->totalWeight(), 6 * ngrt4n::WEIGHT_UNIT);
  QCOMPARE(m_severityAggregator->minSev(), Severity(ngrt4n::Normal).value());
  QCOMPARE(m_severityAggregator->maxSev(), Severity(ngrt4n::Major).value());
}

void TestSeverityAggregation::testAverage(void)
{
  QCOMPARE(m_severityAggregator->aggregate(CalcRules::Average), static_cast<int>(ngrt4n::Minor));
}

void TestSeverityAggregation::testWeighted1(void)
{
  m_severityAggregator->displayWeight();
  m_severityAggregator->addThresholdLimit({0.5, ngrt4n::Major, ngrt4n::Critical});
  QCOMPARE(m_severityAggregator->weightedAverageWithThresholds(), m_severityAggregator->weightedAverage());

  m_severityAggregator->addThresholdLimit({0.5, ngrt4n::Minor, ngrt4n::Major});
  QCOMPARE(m_severityAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Major));

  m_severityAggregator->addThresholdLimit({0.3, ngrt4n::Major, ngrt4n::Critical});
  QCOMPARE(m_severityAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Critical));
}

void TestSeverityAggregation::testWeighted2(void)
{
  m_severityAggregator->reset();
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);

  m_severityAggregator->displayWeight();

  m_severityAggregator->addThresholdLimit({0.75, ngrt4n::Minor, ngrt4n::Major});
  QCOMPARE(m_severityAggregator->weightedAverageWithThresholds(), m_severityAggregator->weightedAverage());
  QCOMPARE(m_severityAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Minor));

  m_severityAggregator->addThresholdLimit({0.6, ngrt4n::Minor, ngrt4n::Major});
  QCOMPARE(m_severityAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Major));

  m_severityAggregator->addThresholdLimit({0.5, ngrt4n::Major, ngrt4n::Critical});
  QCOMPARE(m_severityAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Major));
}



void TestSeverityAggregation::testWeighted3(void)
{
  m_severityAggregator->reset();
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);

  m_severityAggregator->displayWeight();

  m_severityAggregator->addThresholdLimit({0.8, ngrt4n::Minor, ngrt4n::Critical});
  m_severityAggregator->addThresholdLimit({0.6, ngrt4n::Minor, ngrt4n::Major});
  qDebug()<< m_severityAggregator->thresholdExceededMsg();
  QCOMPARE(m_severityAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Critical));
}

void TestSeverityAggregation::testWeighted4(void)
{
  m_severityAggregator->reset();
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Critical, ngrt4n::WEIGHT_MAX);

  m_severityAggregator->displayWeight();

  m_severityAggregator->addThresholdLimit({0.8, ngrt4n::Minor, ngrt4n::Major});
  qDebug()<< m_severityAggregator->thresholdExceededMsg();
  QCOMPARE(m_severityAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Critical));
}


void TestSeverityAggregation::testWorst(void)
{
  m_severityAggregator->reset();
  m_severityAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Major, 3 * ngrt4n::WEIGHT_UNIT);
  m_severityAggregator->addSeverity(ngrt4n::Unset, 3 * ngrt4n::WEIGHT_UNIT);
  QCOMPARE(m_severityAggregator->aggregate(CalcRules::Worst), static_cast<int>(ngrt4n::Unknown));
}

QTEST_MAIN(TestSeverityAggregation)
#include "unittests.moc"

