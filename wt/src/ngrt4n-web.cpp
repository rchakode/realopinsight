
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
#ifdef REALOPINSIGHT_WEB_FASTCGI
    m_dirroot = "";
    m_docroot = "";
#else
    m_dirroot = "/";
    m_docroot = docRoot() +  m_dirroot;
#endif

    setTwoPhaseRenderingThreshold(0);
    useStyleSheet(m_dirroot+"resources/css/ngrt4n.css");
    useStyleSheet(m_dirroot+"resources/css/font-awesome.min.css");
    messageResourceBundle().use(m_docroot+"resources/i18n/messages");
    setTheme(new Wt::WBootstrapTheme());
    requireJQuery(m_dirroot+"resources/js/jquery-1.10.2.min.js");
#ifdef ENABLE_ANALYTICS
    require(m_dirroot+"resources/js/ga.js");
#endif
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
  std::string m_dirroot;
  std::string m_docroot;
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
