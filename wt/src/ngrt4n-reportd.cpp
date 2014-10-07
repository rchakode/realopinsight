
#include "Applications.hpp"
#include <Wt/WApplication>
#include "QosCollector.hpp"


ReportCollectorApp* createRealOpInsightWApplication(const Wt::WEnvironment& env)
{
  return new ReportCollectorApp(env);
}


int main(int argc, char **argv)
{
  RealOpInsightQApp qtApp (argc, argv);

  try {
    DbSession dbSession;
    QosCollector biCollector("", &dbSession);
  } catch (dbo::Exception& ex){
    std::cerr << QObject::tr("[FATAL] %1").arg(ex.what()).toStdString();
    exit(1);
  } catch (std::exception &ex) {
    std::cerr << QObject::tr("[FATAL] %1").arg(ex.what()).toStdString();
    exit(1);
  }
  return qtApp.exec();
}
