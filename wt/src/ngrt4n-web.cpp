#include "WebMainUI.hpp"
#include <Wt/WBootstrapTheme>

Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
  WebMainUI* webApp = new WebMainUI(env);
  webApp->setTwoPhaseRenderingThreshold(0);
  webApp->useStyleSheet(Wt::WApplication::appRoot() + "resources/css/ngrt4n.css");
  webApp->messageResourceBundle().use(Wt::WApplication::appRoot() + "resources/i18n/messages");
  webApp->setTheme(new Wt::WBootstrapTheme());
  webApp->requireJQuery("resources/js/jquery-1.10.2.min.js");
  webApp->showHome();
  return webApp;
}


int main(int argc, char **argv)
{
  QApplication* qtApp = new QApplication(argc, argv);
  return Wt::WRun(argc, argv, &createApplication) | qtApp->exec();
}
