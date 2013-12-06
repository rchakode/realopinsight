#include "WebMainUI.hpp"
#include <Wt/WBootstrapTheme>

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
  QApplication* qtApp = new QApplication(argc, argv);
  return Wt::WRun(argc, argv, &createApplication) | qtApp->exec();
}
