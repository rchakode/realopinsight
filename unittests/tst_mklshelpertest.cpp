#include <QString>
#include <QtTest>
#include "MkLsHelper.hpp"

CheckListCstIterT check;
MkLsHelper mkhelper("ubs-1", 50000);

class MkLsHelperTest : public QObject
{
  Q_OBJECT
  
public:
  MkLsHelperTest();
  ~MkLsHelperTest();
  
private Q_SLOTS:
  void testCase1();
  void testBadRecvingDataType();
  void testCaseService();
  void testBadHostBadPort();
  void testLoadHostData();
};

MkLsHelperTest::MkLsHelperTest()
{
}

MkLsHelperTest::~MkLsHelperTest()
{
}

void MkLsHelperTest::testCase1()
{
  QVERIFY(true == mkhelper.connectToService());
  mkhelper.clearData();
  QVERIFY(true == mkhelper.requestData("localhost", MkLsHelper::Host));
  QVERIFY(true == mkhelper.recvData(MkLsHelper::Host));
  QVERIFY(true == mkhelper.findCheck("localhost", check));
  QVERIFY(false == mkhelper.findCheck("localhost/PING", check));

  mkhelper.disconnectFromService();
}

void MkLsHelperTest::testBadRecvingDataType()
{
  QVERIFY(true == mkhelper.connectToService());
  mkhelper.clearData();
  QVERIFY(true == mkhelper.requestData("localhost", MkLsHelper::Service));
  QVERIFY(false == mkhelper.recvData(MkLsHelper::Host));
  QVERIFY(false == mkhelper.findCheck("localhost", check));

  mkhelper.disconnectFromService();
}

void MkLsHelperTest::testCaseService()
{
  QVERIFY(true == mkhelper.connectToService());
  mkhelper.clearData();
  QVERIFY(true == mkhelper.requestData("localhost", MkLsHelper::Service));
  QVERIFY(true == mkhelper.recvData(MkLsHelper::Service));
  QVERIFY(true == mkhelper.findCheck("localhost/Load", check));

  mkhelper.disconnectFromService();
}


void MkLsHelperTest::testLoadHostData()
{
  mkhelper.setHost("ubs-1");
  mkhelper.setPort(50000);
  QVERIFY(true == mkhelper.connectToService());
  QVERIFY(true == mkhelper.loadHostData("localhost"));
  QVERIFY(true == mkhelper.findCheck("localhost", check));
  QVERIFY(true == mkhelper.findCheck("localhost/Load", check));
  QVERIFY(false == mkhelper.findCheck("ubs/Load", check));
  mkhelper.disconnectFromService();
}


void MkLsHelperTest::testBadHostBadPort()
{
  mkhelper.setHost("localhost");
  mkhelper.setPort(50000);
  QEXPECT_FAIL("", "Connection on bad host failed", Continue);
  QVERIFY(false == mkhelper.connectToService());
  mkhelper.disconnectFromService();

  mkhelper.setHost("localhast");
  QEXPECT_FAIL("", "Connection on bad host failed", Continue);
  QVERIFY(false == mkhelper.connectToService());
  mkhelper.disconnectFromService();
}

QTEST_MAIN(MkLsHelperTest)
//QTEST_APPLESS_MAIN(MkLsHelperTest)

#include "tst_mklshelpertest.moc"
