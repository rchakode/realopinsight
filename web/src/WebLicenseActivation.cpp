#include "WebLicenseActivation.hpp"
#include "WebUtils.hpp"

WebLicenseActivation::WebLicenseActivation()
  : WebPreferencesBase(),
    Wt::WTemplate(Wt::WString::tr("license-activation-form.tpl"))
{
  bindWidget("activation-key-field", m_activationKeyField = new Wt::WLineEdit());
  bindWidget("activate-button", m_activeBtn = new Wt::WPushButton(Q_TR("Activate")));
  m_activeBtn->clicked().connect(this, &WebLicenseActivation::saveActivationKey);
}

void WebLicenseActivation::saveActivationKey(void)
{
  setKeyValue(Settings::ACTIVATION_LICENSE_KEY, m_activationKeyField->text().toUTF8().c_str());
  sync();
}

QString WebLicenseActivation::genLicenseKey(const QString& hostid, const QString& hostname, const QString& version)
{
  QCryptographicHash hasher(QCryptographicHash::Md5);
  hasher.addData(hostid.toLatin1());
  hasher.addData(hostname.toLatin1());
  hasher.addData(version.toLatin1());
  QString key = "";
  QByteArray md5 = hasher.result().toHex().toUpper();
  for (int i = 0; i < md5.length(); ++i) {
    if ((i != 0) && (i % 4 == 0))
      key.append("-");
    key += md5[i];
  }
  return key;
}

bool WebLicenseActivation::isActivated(const QString& version)
{
  return checkLicenseKey(value(Settings::ACTIVATION_LICENSE_KEY, ""), version);
}

bool WebLicenseActivation::checkLicenseKey(const QString& key, const QString& version)
{
  return isValidKey(key, getHostId(), getHostName(), version);
}


bool WebLicenseActivation::isValidKey(const QString& key, const QString& hostid, const QString& hostname, const QString& version)
{
  return key == genLicenseKey(hostid, hostname, version);
}


QString WebLicenseActivation::getHostName(void)
{
  char hostname[255];
  size_t hostnameLen = 0;
  if (gethostname(hostname, hostnameLen) != 0) {
    hostnameLen = 0;
    qDebug() << "Can't get hostname: gethostname failed";
  }

  return std::string(hostname, hostnameLen).c_str();
}

QString WebLicenseActivation::getHostId(void)
{
  return QString::number(gethostid());
}
