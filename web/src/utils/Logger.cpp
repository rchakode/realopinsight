#include "Logger.hpp"
#include <QDateTime>
#include "WebUtils.hpp"

const std::string Logger::m_coreSemaphoreName = "realopinsight_core_log_sem";
const std::string Logger::m_reportdSemaphoreName = "realopinsight_reportd_log_sem";

Logger::Logger(int module, const std::string& logdir)
  : m_module(module)
{
  m_coreSemaphore = ngrt4n::createSemaphoreOrDie(m_coreSemaphoreName);
  m_reportdSemaphore = ngrt4n::createSemaphoreOrDie(m_reportdSemaphoreName);
  m_coreLogPath = QString("%1/realopinsight.log").arg(logdir.c_str()).toStdString();
  m_monitorLogPath = QString("%1/realopinsight-reportd.log").arg(logdir.c_str()).toStdString();
  m_coreLoggingStream.open(m_coreLogPath.c_str(), std::ios::out|std::ios::app);
  monitorLoggingStream.open(m_monitorLogPath.c_str(), std::ios::out|std::ios::app);
}


Logger::~Logger()
{
  ngrt4n::releaseSemaphore(m_coreSemaphore);
  ngrt4n::releaseSemaphore(m_reportdSemaphore);
  m_coreLoggingStream.close();
  monitorLoggingStream.close();
}


void Logger::log(const std::string& logLevel, const std::string& msg)
{
  std::string logEntry = QString("%1 [%2] %3")
      .arg(QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:s"),
           logLevel.c_str(),msg.c_str()).toStdString();
  if(m_module == ReportdLogger) {
    sem_wait(m_reportdSemaphore);
    monitorLoggingStream << logEntry << std::endl;
    sem_post(m_reportdSemaphore);
  } else {
    sem_wait(m_coreSemaphore);
    m_coreLoggingStream << logEntry << std::endl;
    sem_post(m_coreSemaphore);
  }
}

