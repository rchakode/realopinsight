#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP
#include "WebUtils.hpp"
#include <Wt/WValidator>
#include <Wt/WIntValidator>
#include <Wt/WString>


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
      return Wt::WValidator::Result(Wt::WValidator::Valid);

    return Wt::WValidator::Result(Wt::WValidator::Invalid, QObject::tr("Invalid %1 URI").arg(m_schemeBase).toStdString());
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
      return Wt::WValidator::Result(Wt::WValidator::Valid);
    return Wt::WValidator::Result(Wt::WValidator::Invalid, QObject::tr("Bad hostname/IP address").toStdString());
  }
};

class PortValidator: public Wt::WIntValidator
{
public:
  PortValidator(Wt::WObject* parent = 0)
    : Wt::WIntValidator(parent)
  {
    setRange(1, 65535);
  }
};


#endif // VALIDATOR_HPP
