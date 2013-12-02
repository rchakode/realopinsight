#include "WebMainUI.hpp"
#include <Wt/WBootstrapTheme>

Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
  WebMainUI* ui = new WebMainUI(env, "examples/small_hosting_platform.nag.ngrt4n.xml"); //FIXME:set config file
  ui->setTwoPhaseRenderingThreshold(0);
  ui->setTitle(ui->getConfig().toStdString());
  ui->setTheme(new Wt::WBootstrapTheme());
  ui->requireJQuery("resources/js/jquery-1.10.2.min.js");
  //ui->setCssTheme("polished");
  ui->useStyleSheet(Wt::WApplication::appRoot() + "resources/css/ngrt4n.css");
  ui->render();
  return ui;
}


int main(int argc, char **argv)
{
  QApplication* qtApp = new QApplication(argc, argv);
  return Wt::WRun(argc, argv, &createApplication) | qtApp->exec();
}
