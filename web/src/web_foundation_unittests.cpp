
#include "web_foundation_unittests.hpp"
#include "Base.hpp"

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

  m_dbSession.addUser(user1);
  m_dbSession.addView(view1);
  m_dbSession.assignView(TEST_USER1, TEST_VIEW1);
}

NotificationTest::~NotificationTest()
{
  m_dbSession.deleteUser(TEST_USER1);
  m_dbSession.deleteUser(TEST_VIEW1);
}


void NotificationTest::testFecthAssignedUserEmails(void)
{
  QStringList emails;
	QCOMPARE(1, m_dbSession.listAssignedUsersEmails(emails, TEST_VIEW1));
	QCOMPARE(0, m_dbSession.listAssignedUsersEmails(emails, "admin"));
}

void NotificationTest::testAddNotification(void)
{
  QCOMPARE(ngrt4n::RcSuccess, m_dbSession.addNotification(TEST_VIEW1, ngrt4n::Minor));

  NotificationT notification;
	QCOMPARE(true, m_dbSession.getLastNotificationInfo(notification, TEST_VIEW1));
  QCOMPARE((long)DboNotification::Open, notification.ack_status);
}

void NotificationTest::testAcknowledgeNotification(void)
{
  QCOMPARE(0, m_dbSession.updateNotificationStatus(TEST_USER1, TEST_VIEW1, DboNotification::Acknowledged));

  NotificationT notification;
	QCOMPARE(true, m_dbSession.getLastNotificationInfo(notification, TEST_VIEW1));
  QCOMPARE((long)DboNotification::Acknowledged, notification.ack_status);
}

void NotificationTest::testCloseNotification(void)
{
  QCOMPARE(0, m_dbSession.updateNotificationStatus(TEST_USER1, TEST_VIEW1, DboNotification::Closed));

  NotificationT notification;
	QCOMPARE(true, m_dbSession.getLastNotificationInfo(notification, TEST_VIEW1));
  QCOMPARE((long)DboNotification::Closed, notification.ack_status);
}

void NotificationTest::testFlushNotification(void)
{
	//FIXME: QCOMPARE(0, m_dbSession.countViewRelatedNotifications(TEST_VIEW1));
}

QTEST_MAIN(NotificationTest)
