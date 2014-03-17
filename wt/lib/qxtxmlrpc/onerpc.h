#include <QCoreApplication>
#include "qxtxmlrpccall.h"
#include "qxtxmlrpcclient.h"


class OneRpcClient : public QxtXmlRpcClient
{
  Q_OBJECT

public:
  OneRpcClient();

  virtual ~OneRpcClient();
  void retrieveAllHostInfo(void);

public slots:
  void handleFinished(void);
  void handleError(QNetworkReply::NetworkError error);

private:
  QxtXmlRpcCall* mreply;
};

