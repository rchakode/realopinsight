#include "Logger.hpp"
#include <QDateTime>
#include <fcntl.h>
#include "WebUtils.hpp"

const std::string Logger::m_coreSemaphoreName = "realopinsight_core_log_sem";
const std::string Logger::m_reportdSemaphoreName = "realopinsight_reportd_log_sem";

Logger::Logger(int module, const std::string& logdir)
  : m_module(module)
{
  m_coreSemaphore = createSemaphoreOrDie(m_coreSemaphoreName);
  m_reportdSemaphore = createSemaphoreOrDie(m_reportdSemaphoreName);
  m_coreLogPath = QString("%1/realopinsight.log").arg(logdir.c_str()).toStdString();
  m_monitorLogPath = QString("%1/realopinsight-reportd.log").arg(logdir.c_str()).toStdString();
  m_coreLoggingStream.open(m_coreLogPath.c_str(), std::ios::out|std::ios::app);
  monitorLoggingStream.open(m_monitorLogPath.c_str(), std::ios::out|std::ios::app);
}


Logger::~Logger()
{
  sem_close(m_coreSemaphore);
  sem_close(m_reportdSemaphore);
  m_coreLoggingStream.close();
  monitorLoggingStream.close();
}



sem_t* Logger::createSemaphoreOrDie(const std::string& name)
{
  sem_t* mysem = sem_open(name.c_str(), O_CREAT, S_IRUSR|S_IWUSR, 1);
  if (mysem == SEM_FAILED) {
    std::string errorMsg = Q_TR("The initialization of the logger semaphore has failed: ");
    switch (errno) {
      case EACCES:
        errorMsg += Q_TR("permission denied to access to the semaphore");
        break;
      default:
        errorMsg += QObject::tr("sem_open returned errno %1")
            .arg(QString::number(errno))
            .toStdString();
        break;
    }
    qFatal("%s", errorMsg.c_str());
    exit(1);
  }
  return mysem;
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

