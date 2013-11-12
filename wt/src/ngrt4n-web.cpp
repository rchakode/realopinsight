#include "MainWebUI.hpp"

Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
  MainWebUI* app = new MainWebUI(env, "config"); //FIXME:set config file
  app->setTwoPhaseRenderingThreshold(0);
  app->setTitle("NGRT4N Dashboard");
  app->useStyleSheet(Wt::WApplication::appRoot() + "resources/styles.css");
  app->messageResourceBundle().use(Wt::WApplication::appRoot() + "resources/messages");
  app->setLocale("en");
  app->render();
  app->refresh();
  return app;
}


int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, &createApplication);
}
