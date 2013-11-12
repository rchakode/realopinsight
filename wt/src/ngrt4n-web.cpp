#include "WebUI.hpp"

Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
  WebUI* ui = new WebUI(env, "config"); //FIXME:set config file
  ui->setTwoPhaseRenderingThreshold(0);
  ui->setTitle(ui->getConfig().toStdString());
  ui->useStyleSheet(Wt::WApplication::appRoot() + "wt/resources/styles.css");
  ui->render();
  return ui;
}


int main(int argc, char **argv)
{
  QApplication* qtApp = new QApplication(argc, argv);
  return Wt::WRun(argc, argv, &createApplication) | qtApp->exec();
}
