#include "WebMainUI.hpp"
#include <Wt/WBootstrapTheme>
#include <Wt/WServer>

Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
  WebMainUI* webApp = new WebMainUI(env);
  webApp->setTwoPhaseRenderingThreshold(0);
  webApp->useStyleSheet("/resources/css/ngrt4n.css");
  webApp->messageResourceBundle().use(webApp->docRoot() + "/resources/i18n/messages");
  webApp->setTheme(new Wt::WBootstrapTheme());
  webApp->requireJQuery("/resources/js/jquery-1.10.2.min.js");
  webApp->showLoginHome();
  return webApp;
}


int main(int argc, char **argv)
{

  try {
    Wt::WServer server(argv[0]);

    server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
    server.addEntryPoint(Wt::Application, &createApplication);

    DbSession::configureAuth();

    if (server.start()) {
      Wt::WServer::waitForShutdown();
      server.stop();
    }
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() <"\n";
  } catch (Wt::Dbo::Exception &e) {
    std::cerr << "Dbo exception: " << e.what() <<"\n";
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() <<"\n";
  }

  QApplication* qtApp = new QApplication(argc, argv);
  //  return Wt::WRun(argc, argv, &createApplication) | qtApp->exec();
  return qtApp->exec();
}
