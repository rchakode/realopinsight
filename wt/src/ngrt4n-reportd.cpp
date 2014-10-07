
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
    WebPreferences preferences;
    dbSession.updateUserList();

    // Preparing qos collectors
    std::vector<QosCollector*> mycollectors;
    for (auto view: dbSession.viewList()) {
      QosCollector* collector =  new QosCollector(view.path.c_str());
      collector->initialize(&preferences);
      mycollectors.push_back(collector);
    }
    // handle monitoring
    while (1) {
      qDebug()<< "Starting monitoring loop";
      for (auto collector: mycollectors) {
        collector->runMonitor();
        dbSession.addQosInfo(collector->qosInfo());
      }

      sleep(5);
    }
    // free up resources
    for (auto collector: mycollectors) {
      delete collector;
    }

  } catch (dbo::Exception& ex){
    std::cerr << QObject::tr("[FATAL] %1").arg(ex.what()).toStdString();
    exit(1);
  } catch (std::exception &ex) {
    std::cerr << QObject::tr("[FATAL] %1").arg(ex.what()).toStdString();
    exit(1);
  }
  return qtApp.exec();
}
