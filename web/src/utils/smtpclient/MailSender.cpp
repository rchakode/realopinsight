#include "MailSender.hpp"
#include "WebUtils.hpp"

MailSender::MailSender(const QString& smtpHost,
                       int port,
                       const QString& username,
                       const QString& password,
                       bool useStartSsl)
  : QxtSmtp(),
    m_host(smtpHost),
    m_port(port)
{
  addEvents();
  setUsername(username.toAscii());
  setPassword(password.toAscii());
  setStartTlsDisabled(! useStartSsl);
}

int MailSender::send(const QString& sender,
                     const QStringList& recipients,
                     const QString& subject,
                     const QString& body)
{
  connectToSecureHost(m_host, m_port);

  if (m_eventSynchonizer.exec() != 0)
    return -1;

  QxtMailMessage message;

  message.setExtraHeader("From", sender);
  message.setExtraHeader("MIME-Version", "1.0");
  message.setExtraHeader("Content-Type", "text/html; charset=ISO-8859-1");

  Q_FOREACH(const QString& recipient, recipients) {
    message.addRecipient(recipient);
  }

  message.setSender(QString("%1 <%2>").arg(APP_NAME, sender));
  message.setSubject(subject);
  message.setBody(body);

  int messageId = QxtSmtp::send(message);
  m_spool.insert(messageId, message);
  int exitCode = m_eventSynchonizer.exec();

  disconnectFromHost();
  return exitCode;
}


void MailSender::connected(void)
{
  m_eventSynchonizer.exit(0);
}

void MailSender::handleConnectionFailed(const QByteArray& msg)
{
  m_lastError = tr("[MailSender] connection error: %1").arg(QString(msg)).toStdString();
  m_eventSynchonizer.exit(-1);
}

void MailSender::handleMailFailed(int mailID, int errorCode, const QByteArray& msg)
{
  m_spool.remove(mailID);
  m_lastError = tr("[MailSender] Sending error %1: %2. Mail Id: %3")
                .arg(QString::number(errorCode), msg, QString::number(mailID));
  m_eventSynchonizer.exit(-1);
}

void MailSender::handleSenderRejected(int mailID, const QString& address, const QByteArray & msg)
{
  m_spool.remove(mailID);
  m_lastError = tr("[MailSender] Mail sender has been rejected. Address: %1. Message %2, Mail Id: %3")
                .arg(address, QString(msg), QString::number(mailID));
  m_eventSynchonizer.exit(-1);
}

void MailSender::handleMailSent(int mailID)
{
  m_spool.remove(mailID);
  m_lastError = tr("[MailSender] Mail sent. Mail Id: %3").arg(QString::number(mailID)).toStdString();
  m_eventSynchonizer.exit(0);
}


void MailSender::addEvents(void)
{
  connect(this, SIGNAL(connectionFailed(const QByteArray&)), this, SLOT(handleConnectionFailed(const QByteArray&)));
  connect(this, SIGNAL(authenticationFailed(const QByteArray&)), this, SLOT(handleConnectionFailed(const QByteArray&)));
  connect(this, SIGNAL(encryptionFailed(const QByteArray&)), this, SLOT(handleConnectionFailed(const QByteArray&)));
  connect(this, SIGNAL(mailFailed(int, int, const QByteArray&)), this, SLOT(handleMailFailed(int, int, const QByteArray&)));
  connect(this, SIGNAL(senderRejected(int, const QString&, const QByteArray&)), this, SLOT(handleSenderRejected(int, const QString&, const QByteArray&)));
  connect(this, SIGNAL(recipientRejected(int, const QString&, const QByteArray&)), this, SLOT(handleSenderRejected(int, const QString&, const QByteArray&)));
  connect(this, SIGNAL(mailSent(int)), this, SLOT(handleMailSent(int)));
}
