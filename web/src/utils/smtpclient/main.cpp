#include <QCoreApplication>
#include <QHostAddress>
#include "qxtsmtp.h"
#include "qxtmailmessage.h"
#include "MailSender.hpp"

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  MailSender sender("smtp.mail.yahoo.com",
                    465,
                    "rodrigue_in@yahoo.fr",
                    "ch#r0n$3",
                    false);

  sender.send("rodrigue_in@yahoo.fr",
              "rodrigue.chakode@gmail.com",
              "Bonjour depuis la chine",
              "Mon message de bienvenu");

  return a.exec();
}
