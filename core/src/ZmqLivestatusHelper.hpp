#ifndef ZMQLIVESTATUSHELPER_HPP
#define ZMQLIVESTATUSHELPER_HPP
#include "Base.hpp"

class ZmqLivestatusHelper
{
public:
  ZmqLivestatusHelper();

  int
  loadChecks(const SourceT& srcInfo, const QString& host, ChecksT& checks);

private:
};

#endif // ZMQLIVESTATUSHELPER_HPP
