
#include "WQApplication"
#include "WebUtils.hpp"
#include "AuthManager.hpp"
#include "DbSession.hpp"
#include "WebMainUI.hpp"
#include <Wt/WBootstrapTheme>
#include <Wt/WServer>
#include <Wt/WEnvironment>


class WebApp : public Wt::WQApplication
{
public:
  WebApp(const Wt::WEnvironment& env)
    : WQApplication(env, true) {}

protected:
  virtual void create()
  {
    setTwoPhaseRenderingThreshold(0);
    useStyleSheet("/resources/css/ngrt4n.css");
    useStyleSheet("/resources/css/font-awesome.min.css");
    messageResourceBundle().use(docRoot() + "/resources/i18n/messages");
    setTheme(new Wt::WBootstrapTheme());
    requireJQuery("/resources/js/jquery-1.10.2.min.js");
    m_dbSession = new DbSession();
    root()->setId("wrapper");
    root()->addWidget(new AuthManager(m_dbSession));
  }

  virtual void destroy()
  {
    delete m_dbSession;
  }

private:
  DbSession* m_dbSession;
};

Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
  return new WebApp(env);
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
    LOG("error", e.what());
  } catch (Wt::Dbo::Exception &e) {
    LOG("error", e.what());
  } catch (std::exception &e) {
    LOG("error", e.what());
  }
  return qtApp->exec();
}
