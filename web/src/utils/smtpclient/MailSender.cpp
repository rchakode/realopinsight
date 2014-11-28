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
                      const QStringList& recipients,
                      const QString& subject,
                      const QString& body)
{
  QxtMailMessage message;

  message.setExtraHeader("From", sender);
  message.setExtraHeader("MIME-Version", "1.0");
  message.setExtraHeader("Content-Type", "text/html; charset=ISO-8859-1");

  Q_FOREACH(const QString& recipient, recipients) {
    message.addRecipient(recipient);
  }

  message.setSender(sender);
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
