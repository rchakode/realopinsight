#include "Logger.hpp"
#include <QDateTime>
#include <fcntl.h>
#include "WebUtils.hpp"

const std::string Logger::m_lockSemaphoreName = "sem_lock_realopinsight_log";

Logger::Logger(int module, const std::string& logdir)
  : m_module(module)
{
  m_lockSemaphore = sem_open(m_lockSemaphoreName.c_str(), O_CREAT, S_IRUSR|S_IWUSR, 1);
  if(m_lockSemaphore == SEM_FAILED) {
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
    qFatal(errorMsg.c_str());
    exit(1);
  }

  m_coreLogPath = QString("%1/realopinsight.log").arg(logdir.c_str()).toStdString();
  m_monitorLogPath = QString("%1/realopinsight-reportd.log").arg(logdir.c_str()).toStdString();
  m_coreLoggingStream.open(m_coreLogPath.c_str(), std::ios::out|std::ios::app);
  monitorLoggingStream.open(m_monitorLogPath.c_str(), std::ios::out|std::ios::app);
}


Logger::~Logger()
{
  m_coreLoggingStream.close();
  monitorLoggingStream.close();
}


void Logger::log(const std::string& logLevel, const std::string& msg)
{
  std::string logEntry = QString("%1[%2]: %3")
      .arg(QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:s"),
           logLevel.c_str(),msg.c_str()).toStdString();
  if(m_module == ReportdLogger) {
    monitorLoggingStream << logEntry << std::endl;
  } else {
    sem_wait(m_lockSemaphore);
    m_coreLoggingStream << logEntry << std::endl;
    sem_post(m_lockSemaphore);
  }
}

