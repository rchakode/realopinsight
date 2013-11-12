#include "MainWebUI.hpp"

Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
  MainWebUI* ui = new MainWebUI(env, "config"); //FIXME:set config file
  ui->setTwoPhaseRenderingThreshold(0);
  ui->setTitle(ui->getConfig().toStdString());
  ui->useStyleSheet(Wt::WApplication::appRoot() + "wt/resources/styles.css");
  ui->messageResourceBundle().use(Wt::WApplication::appRoot() + "wt/resources/messages");
  ui->setLocale("en");
  ui->render();
  return ui;
}


int main(int argc, char **argv)
{
  QApplication* qtApp = new QApplication(argc, argv);
  return Wt::WRun(argc, argv, &createApplication) | qtApp->exec();
}
