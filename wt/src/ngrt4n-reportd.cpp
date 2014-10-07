
#include "RealOpInsightQApp.hpp"
#include <Wt/WApplication>
#include "DbSession.hpp"
#include "WQApplication"
#include "BIDataCollector.hpp"

class WebApp : public Wt::WQApplication
{
public:
  WebApp(const Wt::WEnvironment& env)
    : WQApplication(env, true) {}

protected:
  virtual void create()
  {
    m_dbSession = new DbSession();
  }

  virtual void destroy()
  {
    delete m_dbSession;
  }

private:
  DbSession* m_dbSession;
  std::string m_dirroot;
  std::string m_docroot;
};

Wt::WApplication* createRealOpInsightWApplication(const Wt::WEnvironment& env)
{
  return new WebApp(env);
}


int main(int argc, char **argv)
{
  RealOpInsightQApp qtApp (argc, argv);

  try {
    BIDataCollector biCollector("");
  } catch (dbo::Exception& ex){
    std::cerr << QObject::tr("[FATAL] %1").arg(ex.what()).toStdString();
    exit(1);
  } catch (std::exception &ex) {
    std::cerr << QObject::tr("[FATAL] %1").arg(ex.what()).toStdString();
    exit(1);
  }
  return qtApp.exec();
}
