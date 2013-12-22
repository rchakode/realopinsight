#ifndef AUTHWIDGET_HPP
#define AUTHWIDGET_HPP


#include <Wt/Auth/AuthWidget>
#include <Wt/WContainerWidget>

class DbSession;

class AuthManager : public Wt::Auth::AuthWidget
{
public:
  AuthManager(DbSession* dbSession);
  virtual ~AuthManager(void){}
  DbSession* session(void) {return m_dbSession;}
  void logout(void);
  bool isLogged(void);

protected:
  virtual void createLoggedInView(void) {}

private:
  DbSession* m_dbSession;

  void handleAuthentication(void);
};

#endif // AUTHWIDGET_HPP
