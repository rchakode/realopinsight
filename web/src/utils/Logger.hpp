#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "WebUtils.hpp"
#include <string>
#include <fstream>
#include <sstream>

class Logger
{
public:
  enum {
    CoreLogger = 0,
    ReportdLogger = 1
  };
  Logger(int module);
  virtual ~Logger();

  void log(const std::string& logLevel, const std::string& msg);


private:
  int m_module;
};


#endif // LOGGER_HPP
