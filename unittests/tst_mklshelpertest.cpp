#include <QString>
#include "LsHelper.hpp"

ChecksT check;
LsHelper mkhelper("ubs-1", 50000);

class LsHelperTest : public QObject
{
  Q_OBJECT
  
public:
  LsHelperTest();
  ~LsHelperTest();
  
private Q_SLOTS:
  void testCase1();
  void testBadReceivingDataType();
  void testCaseService();
  void testBadHostBadPort();
  void testLoadHostData();
};

LsHelperTest::LsHelperTest()
{
}

LsHelperTest::~LsHelperTest()
{
}

void LsHelperTest::testCase1()
{
  QVERIFY(true == mkhelper.connectToService());
  mkhelper.clearData();
  QVERIFY(true == mkhelper.requestData("localhost", LsHelper::Host));
  QVERIFY(true == mkhelper.recvData(LsHelper::Host));
  QVERIFY(true == mkhelper.findCheck("localhost", check));
  QVERIFY(false == mkhelper.findCheck("localhost/PING", check));

  mkhelper.disconnectFromService();
}

void LsHelperTest::testBadReceivingDataType()
{
  QVERIFY(true == mkhelper.connectToService());
  mkhelper.clearData();
  QVERIFY(true == mkhelper.requestData("localhost", LsHelper::Service));
  QVERIFY(false == mkhelper.recvData(LsHelper::Host));
  QVERIFY(false == mkhelper.findCheck("localhost", check));

  mkhelper.disconnectFromService();
}

void LsHelperTest::testCaseService()
{
  QVERIFY(true == mkhelper.connectToService());
  mkhelper.clearData();
  QVERIFY(true == mkhelper.requestData("localhost", LsHelper::Service));
  QVERIFY(true == mkhelper.recvData(LsHelper::Service));
  QVERIFY(true == mkhelper.findCheck("localhost/Load", check));

  mkhelper.disconnectFromService();
}


void LsHelperTest::testLoadHostData()
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


void LsHelperTest::testBadHostBadPort()
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

QTEST_MAIN(LsHelperTest)
//QTEST_APPLESS_MAIN(LsHelperTest)

#include "tst_LsHelpertest.moc"
