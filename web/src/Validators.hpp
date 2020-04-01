#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP
#include "WebUtils.hpp"
#include <Wt/WValidator.h>
#include <Wt/WIntValidator.h>
#include <Wt/WString.h>


class UriValidator : public Wt::WValidator
{
public:
  UriValidator(const QString& schemeBase, bool nopath, Wt::WObject* parent = 0)
    : Wt::WValidator(parent),
      m_schemeBase(schemeBase),
      m_nopath(nopath)
  {
  }

  virtual Wt::WValidator::Result validate(const Wt::WString& input) const
  {
    if (ngrt4n::isValidUri(QString::fromStdString(input.toUTF8()), m_schemeBase, m_nopath))
      return Wt::WValidator::Result(Wt::ValidationState::Valid);

    return Wt::WValidator::Result(Wt::ValidationState::Invalid, QObject::tr("Invalid %1 URI").arg(m_schemeBase).toStdString());
  }

private:
  QString m_schemeBase;
  bool m_nopath;
};


class HostValidator: public Wt::WValidator
{
public:
  HostValidator(Wt::WObject* parent = 0)
    : Wt::WValidator(parent)
  {
  }

  virtual Wt::WValidator::Result validate(const Wt::WString& input) const
  {
    if (ngrt4n::isValidHostAddr(input.toUTF8().c_str()))
      return Wt::WValidator::Result(Wt::ValidationState::Valid);
    return Wt::WValidator::Result(Wt::ValidationState::Invalid, QObject::tr("Bad hostname/IP address").toStdString());
  }
};

class PortValidator: public Wt::WIntValidator
{
public:
  PortValidator()
    : Wt::WIntValidator()
  {
    setRange(1, 65535);
  }
};

class FileExistValidator: public Wt::WValidator
{
public:
  FileExistValidator(Wt::WObject* parent = 0)
    : Wt::WValidator(parent) { }

  virtual Wt::WValidator::Result validate(const Wt::WString& input) const
  {
    QFile file(input.toUTF8().c_str());
    if (file.exists())
      return Wt::WValidator::Result(Wt::ValidationState::Valid);
    return Wt::WValidator::Result(Wt::ValidationState::Invalid, QObject::tr("File not found").toStdString());
  }
};

#endif // VALIDATOR_HPP
