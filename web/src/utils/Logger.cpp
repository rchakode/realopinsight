#include "Logger.hpp"
#include <QDateTime>
#include "WebUtils.hpp"

Logger::Logger(int module)
  : m_module(module)
{
}

Logger::~Logger(){}


void Logger::log(const std::string& logLevel, const std::string& msg)
{
  QString module("CORE");
  if(m_module == ReportdLogger) {
    module = "REPORT";
  }
  auto logEntry = QString("%1 [%2][%3] %4").arg(QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:s"),
                                                module,
                                                logLevel.c_str(),
                                                msg.c_str());
  std::cout << logEntry.toStdString() << std::endl;
}

