#ifndef MAILSENDER_HPP
#define MAILSENDER_HPP
#include "qxtsmtp.h"
#include <QEventLoop>

class MailSender : public QxtSmtp
{
  Q_OBJECT

public:
  MailSender(const QString& smtpHost,
             int port,
             const QString& username,
             const QString& password,
             bool disableSsl);

  int send(const QString& sender,
           const QStringList& recipients,
           const QString& subject,
           const QString& body);

protected Q_SLOTS:
  void handleConnectionFailed(const QByteArray& msg);
  void handleMailFailed(int mailID, int errorCode, const QByteArray& msg);
  void handleSenderRejected(int mailID, const QString& address, const QByteArray & msg);
  void handleMailSent(int mailID);

private:
  void addEvents(void);

  QEventLoop m_eventSynchonizer;
  QMap<int, QxtMailMessage> m_spool;
};
#endif // MAILSENDER_HPP
