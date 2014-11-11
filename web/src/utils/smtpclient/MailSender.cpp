#include "MailSender.hpp"


MailSender::MailSender(const QString& smtpHost,
                       int port,
                       const QString& username,
                       const QString& password,
                       bool disableSsl)
  : QxtSmtp()
{
  addEvents();
  setUsername(username.toAscii());
  setPassword(password.toAscii());
  setStartTlsDisabled(disableSsl);
  connectToSecureHost(smtpHost, port);
}

void MailSender::send(const QString& sender,
                      const QString& recipient,
                      const QString& subject,
                      const QString& body)
{
  QxtMailMessage message;
  message.setSender(sender);
  message.addRecipient(recipient);
  message.setSubject(subject);
  message.setBody(body);

  QxtSmtp::send(message);
}

void MailSender::handleConnectionFailed(const QByteArray& msg)
{
  qDebug()<< "connection failed: "<<msg;
}

void MailSender::handleMailFailed(int mailID, int errorCode, const QByteArray& msg)
{
  qDebug()<< "Mail failed: " << mailID  << errorCode << msg;
}

void MailSender::handleSenderRejected(int mailID, const QString& address, const QByteArray & msg)
{
  qDebug()<< "Mail sender rejected: " << mailID  << address << msg;
}

void MailSender::addEvents(void)
{
  connect(this, SIGNAL(connectionFailed(const QByteArray&)),
          this, SLOT(handleConnectionFailed(const QByteArray&)));
  connect(this, SIGNAL(authenticationFailed(const QByteArray&)),
          this, SLOT(handleConnectionFailed(const QByteArray&)));
  connect(this, SIGNAL(encryptionFailed(const QByteArray&)),
          this, SLOT(handleConnectionFailed(const QByteArray&)));
  connect(this, SIGNAL(mailFailed(int, int, const QByteArray&)),
          this, SLOT(handleMailFailed(int, int, const QByteArray&)));
  connect(this, SIGNAL(senderRejected(int, const QString&, const QByteArray&)),
          this, SLOT(handleSenderRejected(int, const QString&, const QByteArray&)));
  connect(this, SIGNAL(recipientRejected(int, const QString&, const QByteArray&)),
          this, SLOT(handleSenderRejected(int, const QString&, const QByteArray&)));
  connect(this, SIGNAL(mailSent(int)), this, SLOT(handleMailSent(int)));
}
