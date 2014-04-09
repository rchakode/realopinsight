
#include <QCoreApplication>
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
    useStyleSheet("resources/css/ngrt4n.css");
    useStyleSheet("resources/css/font-awesome.min.css");
    messageResourceBundle().use("resources/i18n/messages");
    setTheme(new Wt::WBootstrapTheme());
    requireJQuery("resources/js/jquery-1.10.2.min.js");
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

Wt::WApplication* createRealOpInsightWApplication(const Wt::WEnvironment& env)
{
  return new WebApp(env);
}

class RealOpInsightQApp : public QCoreApplication
{
public:
  RealOpInsightQApp(int& argc, char ** argv) : QCoreApplication(argc, argv) { }
  virtual ~RealOpInsightQApp() { }

  virtual bool notify(QObject * receiver, QEvent * event) {
    try {
      return QCoreApplication::notify(receiver, event);
    } catch(std::exception& ex) {
      LOG("fatal", ex.what());
    }
    return false;
  }
};

int main(int argc, char **argv)
{
  RealOpInsightQApp qtApp (argc, argv);
  Q_INIT_RESOURCE(ngrt4n);

  try {
    std::string configurationFile = "/opt/realopinsight/etc/wt_config.xml";
    Wt::WServer server(argv[0], configurationFile);
    server.setServerConfiguration(argc, argv);
    server.addEntryPoint(Wt::Application, &createRealOpInsightWApplication);

    //DbSession::configureAuth();

    if (server.start()) {
      Wt::WServer::waitForShutdown();
      server.stop();
    }
  } catch (std::exception &e) {
    LOG("error", e.what());
  }
  return qtApp.exec();
}
