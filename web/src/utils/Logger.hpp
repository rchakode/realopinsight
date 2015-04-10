#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <semaphore.h>
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
  static const std::string m_coreSemaphoreName;
  static const std::string m_reportdSemaphoreName;
  Logger(int module, const std::string& logdir);
  virtual ~Logger();

  void log(const std::string& logLevel, const std::string& msg);
  std::string getCoreLogPath(void) const {return m_coreLogPath; }
  sem_t* createSemaphoreOrDie(const std::string& name);


private:
  int m_module;
  std::ofstream m_coreLoggingStream;
  std::ofstream monitorLoggingStream;
  std::string m_coreLogPath;
  std::string m_monitorLogPath;
  sem_t* m_coreSemaphore;
  sem_t* m_reportdSemaphore;
};


#endif // LOGGER_HPP
