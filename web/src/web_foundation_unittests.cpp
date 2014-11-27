
#include "web_foundation_unittests.hpp"
#include "dbo/DbSession.hpp"
#include "Base.hpp"

DbSession dbSession;
namespace {
  const std::string TEST_USER1 = "test_user1";
  const std::string TEST_VIEW1 = "view1";
}

NotificationTest::NotificationTest()
{
  DboUser user1;
  user1.username = TEST_USER1;
  user1.password = TEST_USER1;
  user1.firstname = TEST_USER1;
  user1.lastname = TEST_USER1;
  user1.role = DboUser::OpRole;
  user1.registrationDate = QDateTime::currentDateTime().toString().toStdString();
  dbSession.addUser(user1);


  DboView view1;
  view1.name = TEST_VIEW1;
  view1.service_count = 30;
  view1.path = "/dev/null";
  dbSession.addView(view1);

  dbSession.assignView(TEST_USER1, TEST_VIEW1);
}

NotificationTest::~NotificationTest()
{
}

void NotificationTest::testAddNotification(void)
{
  QCOMPARE(0, dbSession.addNotification(TEST_VIEW1, ngrt4n::Minor));
}

void NotificationTest::testAcknowledgeAllUserViewNotifications(void)
{
  QCOMPARE(false, true);
}

void NotificationTest::fecthAllActiveNotifications(void)
{
  QCOMPARE(false, true);
}

void NotificationTest::fecthLastViewActiveNotification(void)
{
  QCOMPARE(false, true);
}


QTEST_MAIN(NotificationTest)
