#include "MainWebWindow.hpp"

Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
  Wt::WApplication* app = new MainWebWindow(env, "config"); //FIXME:set config file
  app->setTwoPhaseRenderingThreshold(0);
  app->setTitle("NGRT4N Dashboard");
  app->useStyleSheet(Wt::WApplication::appRoot() + "resources/styles.css");
  app->messageResourceBundle().use(Wt::WApplication::appRoot() + "resources/messages");
  app->setLocale("en");
  app->refresh();
  return app;
}


int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, &createApplication);
}
