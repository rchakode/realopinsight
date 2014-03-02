#ifndef GUIUTILS_HPP
#define GUIUTILS_HPP
#include <QtGui>

struct GNodeT {
  QGraphicsTextItem* label;
  QGraphicsPixmapItem* icon;
  QGraphicsPixmapItem* exp_icon;
  qint32 type;
  bool expand;
};
struct GEdgeT {
  QGraphicsPathItem* edge;
};
typedef QHash<QString, GNodeT> GNodeListT;
typedef QHash<QString, GEdgeT> GEdgeListT;
typedef QHash<QString, QTreeWidgetItem*> GuiTreeItemListT;
typedef QMap<QString, QMenu*> MenuListT;
typedef QMap<QString, QAction*> SubMenuListT;

namespace ngrt4n {
  const QColor COLOR_NORMAL =  Qt::green;
  const QColor COLOR_MINOR = Qt::yellow;
  const QColor COLOR_MAJOR = QColor("#FF8C00");
  const QColor COLOR_CRITICAL = Qt::red;
  const QColor COLOR_UNKNOWN = Qt::lightGray;
  const QColor HIGHLIGHT_COLOR = QColor(255, 255, 200);
  QSplashScreen* infoScreen(const QString & msg="");
  QColor severityQColor(const int& _criticity);
  void alert(const QString & msg);
  QIcon severityIcon(int _severity);
}

#endif // GUIUTILS_HPP
