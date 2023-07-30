#include "MailSender.hpp"
#include "WebUtils.hpp"

MailSender::MailSender(const QString& smtpHost,
                       int port,
                       const QString& username,
                       const QString& password,
                       bool useStartSsl)
  : QxtSmtp(),
    m_host(smtpHost),
    m_port(port),
    m_connected(false)
{
  addEvents();
  setUsername(username.toLatin1());
  setPassword(password.toLatin1());
  setStartTlsDisabled(! useStartSsl);
}

int MailSender::send(const QString& sender,
                     const QStringList& recipients,
                     const QString& subject,
                     const QString& body)
{
  if (! m_connected) {
    connectToHost(m_host, m_port);
    if (m_eventSynchronizer.exec() != 0)
      return -1;
  }

  QxtMailMessage message;

  message.setExtraHeader("From", QString("%1 <%2>").arg(APP_NAME, sender));
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
  int exitCode = m_eventSynchronizer.exec();

  return exitCode;
}


void MailSender::handleConnected(void)
{
  m_connected = true;
  m_eventSynchronizer.exit(0);
}

void MailSender::handleConnectionFailed(const QByteArray& msg)
{
  m_connected = false;
  m_lastError = tr("SMTP connection failed: %1").arg(QString(msg));
  m_eventSynchronizer.exit(-1);
}

void MailSender::handleMailFailed(int mailID, int errorCode, const QByteArray& msg)
{
  m_spool.remove(mailID);
  m_lastError = tr("SMTP sending failed (code: %1): %2. Mail Id: %3"
                   ).arg(QString::number(errorCode),
                         msg,
                         QString::number(mailID));
  m_eventSynchronizer.exit(-1);
}

void MailSender::handleSenderRejected(int mailID, const QString& address, const QByteArray & msg)
{
  m_spool.remove(mailID);
  m_lastError = tr("SMTP rejected mail sender."
                   " Message: %1. Address: %2."
                   ).arg(QString(msg), address);
  m_eventSynchronizer.exit(-1);
}

void MailSender::handleMailSent(int mailID)
{
  m_lastError = tr("Email successfully sent to %1"
                   ).arg(m_spool[mailID].recipients().join(","));

  m_spool.remove(mailID);
  m_eventSynchronizer.exit(0);
}


void MailSender::addEvents(void)
{
  connect(this, SIGNAL(connected()), this, SLOT(handleConnected()));
  connect(this, SIGNAL(connectionFailed(const QByteArray&)), this, SLOT(handleConnectionFailed(const QByteArray&)));
  connect(this, SIGNAL(authenticationFailed(const QByteArray&)), this, SLOT(handleConnectionFailed(const QByteArray&)));
  connect(this, SIGNAL(encryptionFailed(const QByteArray&)), this, SLOT(handleConnectionFailed(const QByteArray&)));
  connect(this, SIGNAL(mailFailed(int, int, const QByteArray&)), this, SLOT(handleMailFailed(int, int, const QByteArray&)));
  connect(this, SIGNAL(senderRejected(int, const QString&, const QByteArray&)), this, SLOT(handleSenderRejected(int, const QString&, const QByteArray&)));
  connect(this, SIGNAL(recipientRejected(int, const QString&, const QByteArray&)), this, SLOT(handleSenderRejected(int, const QString&, const QByteArray&)));
  connect(this, SIGNAL(mailSent(int)), this, SLOT(handleMailSent(int)));
}
