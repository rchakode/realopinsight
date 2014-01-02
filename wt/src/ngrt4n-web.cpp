#include "WebUtils.hpp"
#include "AuthManager.hpp"
#include "DbSession.hpp"
#include "WebMainUI.hpp"
#include <Wt/WBootstrapTheme>
#include <Wt/WServer>
#include <Wt/WEnvironment>

int argc;
char** argv;
QApplication* qtApp;

Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
  Wt::WApplication* webApp = new Wt::WApplication(env);
  qtApp = new QApplication(argc, argv);
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


int main(int _argc, char **_argv)
{
  argc = _argc;
  argv = _argv;

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
    LOG("error", e.what());
  } catch (Wt::Dbo::Exception &e) {
    LOG("error", e.what());
  } catch (std::exception &e) {
    LOG("error", e.what());
  }
  return qtApp->exec();
}
