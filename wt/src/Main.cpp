#include "include/Ngrt4nDashboard.hpp"

WApplication *createApplication(const WEnvironment& env)
{
  Wt::WApplication *app = new Ngrt4nDashboard(env);
  app->setTwoPhaseRenderingThreshold(0);
  app->setTitle("NGRT4N Dashboard");
  app->useStyleSheet(WApplication::appRoot() + "resources/styles.css");
  app->messageResourceBundle().use(WApplication::appRoot() + "resources/messages");
  app->setLocale("en") ;
  app->refresh();
  return app;
}


int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, &createApplication);
}
