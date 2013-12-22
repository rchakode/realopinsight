#include "AuthManager.hpp"
#include "DbSession.hpp"
#include "WebMainUI.hpp"
#include <Wt/WBootstrapTheme>
#include <Wt/WServer>

Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
  Wt::WApplication* webApp = new Wt::WApplication(env);
  webApp->setTwoPhaseRenderingThreshold(0);
  webApp->useStyleSheet("/resources/css/ngrt4n.css");
  webApp->messageResourceBundle().use(webApp->docRoot() + "/resources/i18n/messages");
  webApp->setTheme(new Wt::WBootstrapTheme());
  webApp->requireJQuery("/resources/js/jquery-1.10.2.min.js");
  DbSession* dbSession = new DbSession(true);
  webApp->root()->setId("wrapper");
  webApp->root()->addWidget(new AuthManager(dbSession));
  return webApp;
}


int main(int argc, char **argv)
{
  QApplication* qtApp = new QApplication(argc, argv);
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
    Wt::log("error")<<"[realopinsight] "<< e.what();
  } catch (Wt::Dbo::Exception &e) {
    Wt::log("error")<<"[realopinsight][dbo] "<< e.what();
  } catch (std::exception &e) {
    Wt::log("error")<<"[realopinsight] "<< e.what();
  }
  return qtApp->exec();
}
