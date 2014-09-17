#include "StatusAggregator.hpp"
#include <QCoreApplication>
#include <QtTest/QTest>

class TestStatusAggregation : public QObject
{
  Q_OBJECT

public:
  TestStatusAggregation(void)
  {
    m_StatusAggregator = new StatusAggregator(QVector<ThresholdT>());
  }

  ~TestStatusAggregation()
  {
    delete m_StatusAggregator;
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
  StatusAggregator* m_StatusAggregator;
};

void TestStatusAggregation::testAddSeverity(void)
{
  m_StatusAggregator->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Major, 0);
  m_StatusAggregator->addSeverity(ngrt4n::Minor, 3 * ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Major, 2 * ngrt4n::WEIGHT_UNIT);

  QCOMPARE(m_StatusAggregator->count(), 4);
  QCOMPARE(m_StatusAggregator->totalWeight(), 6 * ngrt4n::WEIGHT_UNIT);
  QCOMPARE(m_StatusAggregator->minSev(), Severity(ngrt4n::Normal).value());
  QCOMPARE(m_StatusAggregator->maxSev(), Severity(ngrt4n::Major).value());
}

void TestStatusAggregation::testAverage(void)
{
  QCOMPARE(m_StatusAggregator->aggregate(CalcRules::Average), static_cast<int>(ngrt4n::Minor));
}

void TestStatusAggregation::testWeighted1(void)
{
  m_StatusAggregator->displayWeight();
  m_StatusAggregator->addThresholdLimit({0.5, ngrt4n::Major, ngrt4n::Critical});
  QCOMPARE(m_StatusAggregator->weightedAverageWithThresholds(), m_StatusAggregator->weightedAverage());

  m_StatusAggregator->addThresholdLimit({0.5, ngrt4n::Minor, ngrt4n::Major});
  QCOMPARE(m_StatusAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Major));

  m_StatusAggregator->addThresholdLimit({0.3, ngrt4n::Major, ngrt4n::Critical});
  QCOMPARE(m_StatusAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Critical));
}

void TestStatusAggregation::testWeighted2(void)
{
  m_StatusAggregator->resetData();
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);

  m_StatusAggregator->displayWeight();

  m_StatusAggregator->addThresholdLimit({0.75, ngrt4n::Minor, ngrt4n::Major});
  QCOMPARE(m_StatusAggregator->weightedAverageWithThresholds(), m_StatusAggregator->weightedAverage());
  QCOMPARE(m_StatusAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Minor));

  m_StatusAggregator->addThresholdLimit({0.6, ngrt4n::Minor, ngrt4n::Major});
  QCOMPARE(m_StatusAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Major));

  m_StatusAggregator->addThresholdLimit({0.5, ngrt4n::Major, ngrt4n::Critical});
  QCOMPARE(m_StatusAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Major));
}



void TestStatusAggregation::testWeighted3(void)
{
  m_StatusAggregator->resetData();
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Normal, ngrt4n::WEIGHT_UNIT);

  m_StatusAggregator->displayWeight();

  m_StatusAggregator->addThresholdLimit({0.8, ngrt4n::Minor, ngrt4n::Critical});
  m_StatusAggregator->addThresholdLimit({0.6, ngrt4n::Minor, ngrt4n::Major});
  qDebug()<< m_StatusAggregator->thresholdExceededMsg();
  QCOMPARE(m_StatusAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Critical));
}

void TestStatusAggregation::testWeighted4(void)
{
  m_StatusAggregator->resetData();
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Critical, ngrt4n::WEIGHT_MAX);

  m_StatusAggregator->displayWeight();

  m_StatusAggregator->addThresholdLimit({0.8, ngrt4n::Minor, ngrt4n::Major});
  qDebug()<< m_StatusAggregator->thresholdExceededMsg();
  QCOMPARE(m_StatusAggregator->weightedAverageWithThresholds(), static_cast<int>(ngrt4n::Critical));
}


void TestStatusAggregation::testWorst(void)
{
  m_StatusAggregator->resetData();
  m_StatusAggregator->addSeverity(ngrt4n::Minor, ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Major, 3 * ngrt4n::WEIGHT_UNIT);
  m_StatusAggregator->addSeverity(ngrt4n::Unset, 3 * ngrt4n::WEIGHT_UNIT);
  QCOMPARE(m_StatusAggregator->aggregate(CalcRules::Worst), static_cast<int>(ngrt4n::Unknown));
}

QTEST_MAIN(TestStatusAggregation)
#include "unittests.moc"

