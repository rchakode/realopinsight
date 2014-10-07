
#include "Applications.hpp"
#include <Wt/WApplication>
#include <Wt/WServer>


ReportCollectorApp* createQosCollectorApplication(const Wt::WEnvironment& env)
{
  return new ReportCollectorApp(env);
}


int main(int argc, char **argv)
{
  RealOpInsightQApp qtApp (argc, argv);

  try {
    std::string configurationFile = "/opt/realopinsight/etc/wt_config.xml";
    Wt::WServer server(argv[0], configurationFile);
    server.setServerConfiguration(argc, argv);
    server.addEntryPoint(Wt::Application, &createQosCollectorApplication, "", "favicon.ico");

    if (server.start()) {

      Wt::WServer::waitForShutdown();
      server.stop();
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
