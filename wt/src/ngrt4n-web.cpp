
#include <QCoreApplication>
#include <Wt/WServer>
#include "WQApplication"
#include "WebUtils.hpp"
#include "AuthManager.hpp"
#include "DbSession.hpp"
#include "WebMainUI.hpp"
#include "Applications.hpp"




Wt::WApplication* createRealOpInsightWApplication(const Wt::WEnvironment& env)
{
  return new WebApp(env);
}


int main(int argc, char **argv)
{
  RealOpInsightQApp qtApp (argc, argv);
  Q_INIT_RESOURCE(ngrt4n);

  try {
    std::string configurationFile = "/opt/realopinsight/etc/wt_config.xml";
    Wt::WServer server(argv[0], configurationFile);
    server.setServerConfiguration(argc, argv);
    server.addEntryPoint(Wt::Application, &createRealOpInsightWApplication, "", "favicon.ico");

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
