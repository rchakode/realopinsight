#include "WebMainUI.hpp"
#include <Wt/WBootstrapTheme>

Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
  WebMainUI* ui = new WebMainUI(env); //FIXME:set config file
  ui->setTwoPhaseRenderingThreshold(0);
  ui->useStyleSheet(Wt::WApplication::appRoot() + "resources/css/ngrt4n.css");
  ui->messageResourceBundle().use(Wt::WApplication::appRoot() + "resources/i18n/messages");
  ui->setTheme(new Wt::WBootstrapTheme());
  ui->requireJQuery("resources/js/jquery-1.10.2.min.js");
  ui->showHome();
  return ui;
}


int main(int argc, char **argv)
{
  QApplication* qtApp = new QApplication(argc, argv);
  return Wt::WRun(argc, argv, &createApplication) | qtApp->exec();
}
