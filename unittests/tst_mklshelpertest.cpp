#include <QString>
#include <QtTest>
#include "MkLsHelper.hpp"

CheckListCstIterT check;
MkLsHelper mkhelper("ubs-1", "50000");

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
};

MkLsHelperTest::MkLsHelperTest()
{
}

MkLsHelperTest::~MkLsHelperTest()
{
}

void MkLsHelperTest::testCase1()
{
  QVERIFY2(true == mkhelper.connect(), "testCaseService Test connection");

  QVERIFY2(true == mkhelper.requestData("localhost", MkLsHelper::Host), "Request Host Data");
  QVERIFY2(true == mkhelper.recvData(MkLsHelper::Host), "Recv Host Data");
  QVERIFY2(true == mkhelper.findCheck("localhost", check), "Find host check");
  QVERIFY2(false == mkhelper.findCheck("localhost/PING", check), "Find host check");

  mkhelper.disconnectSocket();
}

void MkLsHelperTest::testBadRecvingDataType()
{
  QVERIFY2(true == mkhelper.connect(), "testCaseService Test connection");

  QVERIFY2(true == mkhelper.requestData("localhost", MkLsHelper::Service), "testBadRecvingDataType Request Service Data");
  QVERIFY2(false == mkhelper.recvData(MkLsHelper::Host), "testBadRecvingDataType Recv Service Data");
  QVERIFY2(false == mkhelper.findCheck("localhost", check), "testBadRecvingDataType Find Service Check");

  mkhelper.disconnectSocket();
}

void MkLsHelperTest::testCaseService()
{
  QVERIFY2(true == mkhelper.connect(), "testCaseService Test connection");

  QVERIFY2(true == mkhelper.requestData("localhost", MkLsHelper::Service), "testCaseService Request Service Data");
  QVERIFY2(true == mkhelper.recvData(MkLsHelper::Service), "testCaseService Recv Service Data");
  QVERIFY2(true == mkhelper.findCheck("localhost/Load", check), "testCaseService Find Service Check");

  mkhelper.disconnectSocket();
}

QTEST_APPLESS_MAIN(MkLsHelperTest)

#include "tst_mklshelpertest.moc"
