
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
