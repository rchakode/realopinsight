
#include <QtTest>
#include <QObject>
#include "dbo/DbSession.hpp"

class NotificationTest : public QObject
{
  Q_OBJECT

public:
  NotificationTest();
  ~NotificationTest();

private Q_SLOTS:
  void testAddNotification(void);
  void testAcknowledgeNotification(void);
  void testCloseNotification(void);
  void testFecthAssignedUserEmails(void);
  void testFlushNotification(void);


private:
  DbSession m_dbSession;
};
