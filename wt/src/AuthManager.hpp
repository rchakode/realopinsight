#ifndef AUTHWIDGET_HPP
#define AUTHWIDGET_HPP


#include <Wt/Auth/AuthWidget>
#include <Wt/WContainerWidget>
#include <Wt/Auth/Login>

class DbSession;
class WebMainUI;

class AuthManager : public Wt::Auth::AuthWidget
{
public:
  AuthManager(DbSession* dbSession);
  virtual ~AuthManager(void){}
  DbSession* session(void) {return m_dbSession;}
  void logout(void);
  bool isLogged(void);

protected:
  virtual void createLoggedInView(void) { handleAuthentication();}
  virtual void create(void);

private:
  Wt::Auth::Login m_loginObject;
  DbSession* m_dbSession;
  WebMainUI* m_mainUI;
  bool m_logged;

  void handleAuthentication(void);
};

#endif // AUTHWIDGET_HPP
