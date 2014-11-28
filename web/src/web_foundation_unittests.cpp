
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
  user1.email = QString("%1@example.com").arg(TEST_USER1.c_str()).toStdString();
  user1.role = DboUser::OpRole;
  user1.registrationDate = QDateTime::currentDateTime().toString().toStdString();

  DboView view1;
  view1.name = TEST_VIEW1;
  view1.service_count = 30;
  view1.path = "/dev/null";

  dbSession.addUser(user1);
  dbSession.addView(view1);
  dbSession.assignView(TEST_USER1, TEST_VIEW1);
}

NotificationTest::~NotificationTest()
{
  dbSession.acknowledgeAllActiveNotifications(TEST_USER1, "");
  dbSession.deleteUser(TEST_USER1);
  dbSession.deleteUser(TEST_VIEW1);
}

void NotificationTest::testAddNotification(void)
{
  NotificationListT notifications;
  QCOMPARE(0, dbSession.addNotification(TEST_VIEW1, ngrt4n::Minor));
  QCOMPARE(1, dbSession.fetchActiveNotifications(notifications, TEST_VIEW1));
}

void NotificationTest::testAcknowledgeAllUserViewNotifications(void)
{
  NotificationListT notifications;
  QCOMPARE(0, dbSession.acknowledgeAllActiveNotifications(TEST_USER1, ""));
  QCOMPARE(0, dbSession.fetchActiveNotifications(notifications, TEST_VIEW1));
}

void NotificationTest::testFecthActiveNotifications(void)
{
  NotificationListT notifications;
  QCOMPARE(0, dbSession.acknowledgeAllActiveNotifications("admin", ""));
  QCOMPARE(0, dbSession.addNotification(TEST_VIEW1, ngrt4n::Minor));
  QCOMPARE(0, dbSession.addNotification(TEST_VIEW1, ngrt4n::Major));
  QCOMPARE(0, dbSession.addNotification(TEST_VIEW1, ngrt4n::Critical));
  QCOMPARE(3, dbSession.fetchActiveNotifications(notifications, TEST_VIEW1));
}



void NotificationTest::testFecthAssignedUserEmails(void)
{
  QStringList emails;
  QCOMPARE(1, dbSession.fetchAssignedUserEmails(emails, TEST_VIEW1));
  QCOMPARE(0, dbSession.fetchAssignedUserEmails(emails, "admin"));
}

QTEST_MAIN(NotificationTest)
