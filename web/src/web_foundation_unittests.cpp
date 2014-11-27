
#include <QtTest>
#include <QObject>

class NotificationTest : public QObject
{
  Q_OBJECT

public:
  NotificationTest();
  ~NotificationTest();

private Q_SLOTS:
  void testAddNotification(void);
  void testAcknowledgeAllUserViewNotifications(void);
  void fecthAllActiveNotifications(void);
  void fecthLastViewActiveNotification(void);
};

NotificationTest::NotificationTest()
{

}

NotificationTest::~NotificationTest()
{

}

void NotificationTest::testAddNotification(void)
{

}

void NotificationTest::testAcknowledgeAllUserViewNotifications(void)
{

}

void NotificationTest::fecthAllActiveNotifications(void)
{

}

void NotificationTest::fecthLastViewActiveNotification(void)
{

}


QTEST_MAIN(NotificationTest)
