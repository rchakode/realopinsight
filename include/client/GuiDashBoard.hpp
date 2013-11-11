#include <QString>
#include "Base.hpp"
#include "Chart.hpp"
#include "Parser.hpp"
#include "WebKit.hpp"
#include "MsgConsole.hpp"
#include "GraphView.hpp"
#include "SvNavigatorTree.hpp"
#include "Preferences.hpp"
#include "ZmqSocket.hpp"
#include "ZbxHelper.hpp"
#include "ZnsHelper.hpp"
#include "DashboardBase.hpp"

class QScriptValueIterator;
class QSystemTrayIcon;

class GuiDashboard : public QMainWindow, public DashboardBase
{
  Q_OBJECT

public:
  GuiDashboard(const qint32& _userRole = Auth::OpUserRole,
              const QString& _config = "",
              QWidget* = 0);
  virtual ~GuiDashboard();

  static StringMapT propRules();
  static StringMapT calcRules();
  void initSettings(void);
  virtual void load(const QString& _file);
  virtual void resizeDashboard(void);

public slots:
  virtual void handleShowAbout(void);
  virtual void handleShowOnlineResources(void);
  virtual void handleChangeMonitoringSettingsAction(void);
  virtual void handleChangePasswordAction(void);
  virtual void expandNode(const QString& _nodeId, const bool& _expand, const qint32& _level);
  virtual void centerGraphOnNode(const QString& _nodeId);
  virtual void filterNodeRelatedMsg(void);
  virtual void filterNodeRelatedMsg(const QString& _nodeId);
  virtual void handleTabChanged(int index);
  virtual void handleHideChart(void);
  virtual void toggleFullScreen(bool _toggled);
  virtual void toggleTroubleView(bool _toggled);
  virtual void toggleIncreaseMsgFont(bool _toggled);

signals:
  void hasToBeUpdate(QString);
  void sortEventConsole(void);

protected:
  virtual void closeEvent(QCloseEvent *);
  virtual void contextMenuEvent(QContextMenuEvent *);
  virtual void timerEvent(QTimerEvent *);
  virtual void showEvent(QShowEvent *);


protected:
  enum {
    ConsoleTab=0,
    BrowserTab=1
  };
  std::shared_ptr<Chart> m_chart;
  MsgConsole* m_filteredMsgConsole;
  QSplitter* m_mainSplitter;
  QSplitter* m_rightSplitter;
  QTabWidget* m_viewPanel;
  WebKit* m_browser;
  GraphView* m_map;
  SvNavigatorTree* m_tree;
  MsgConsole* m_msgConsole;
  QMenu* m_contextMenu;
  MenuListT m_menus;
  SubMenuListT m_subMenus;
  SubMenuListT m_contextMenuList;
  QSystemTrayIcon* m_trayIcon;
  QComboBox* m_bxSourceSelection;

  void updateTrayInfo(const NodeT& _node);
  QTabWidget* newMsgConsole();
  void changeBrowserUrl(const QString& sid, const QString& url, const QString& icon);

  void updateDashboardOnError(const SourceT& src, const QString& msg);
  void setBrowserSourceSelectionBx(void);
  virtual void updateDashboardOnError(const SourceT& src, const QString& msg);
  virtual void addEvents(void) = 0;
  virtual void loadMenus(void) = 0;
  virtual void unloadMenus(void) = 0;
  virtual void updateNavTreeItemStatus(const NodeListT::iterator& _node, const QString& _tip);
  virtual void updateNavTreeItemStatus(const NodeT& _node, const QString& _tip) = 0;
  virtual void updateDashboard(const NodeT & _node) = 0;
  virtual void finalizeUpdate(const SourceT& src) = 0;
  virtual void updateStatusBar(const QString& msg);
  virtual void changeBrowserUrl(const QString& sid, const QString& url, const QString& icon);
  virtual void updateDashboard(NodeListT::iterator& _node);
};

#endif /* GUIDASHBOOARD_HPP */