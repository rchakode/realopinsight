#include "GuiUtils.hpp"
#include "utilsClient.hpp"
#include "StatsLegend.hpp"

QSplashScreen* utils::infoScreen(const QString & msg) {
  QSplashScreen* screen = new QSplashScreen(QPixmap(":/images/built-in/loading-screen.png"));
  screen->showMessage(msg, Qt::AlignJustify|Qt::AlignVCenter);
  screen->show();
  return screen;
}

QColor utils::severityQColor(const int& _criticity)
{
  QColor color(utils::COLOR_UNKNOWN);
  switch (static_cast<utils::SeverityT>(_criticity)) {
    case utils::Normal:
      color = utils::COLOR_NORMAL;
      break;
    case utils::Minor:
      color = utils::COLOR_MINOR;
      break;
    case utils::Major:
      color = utils::COLOR_MAJOR;
      break;
    case utils::Critical:
      color = utils::COLOR_CRITICAL;
      break;
    default:
      break;
  }
  return color;
}

void utils::alert(const QString& msg)
{
  QMessageBox::warning(0, QObject::tr("%1 - Warning").arg(APP_NAME), msg, QMessageBox::Yes);
}

QIcon utils::severityIcon(int _severity)
{
  return QIcon(":/"+utils::getIconPath(_severity));
}
