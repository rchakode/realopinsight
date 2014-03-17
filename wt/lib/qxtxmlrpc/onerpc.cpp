#include <QCoreApplication>
#include <QUrl>
#include <QDebug>
#include "qxtxmlrpccall.h"
#include "qxtxmlrpcclient.h"
#include "onerpc.h"

OneRpcClient::OneRpcClient(): QxtXmlRpcClient()
{
  setServiceUrl(QUrl("http://localhost:2633/RPC2"));
}

OneRpcClient::~OneRpcClient() {}


void OneRpcClient::handleFinished(void)
{
  qDebug()<<mreply->result();
}


void OneRpcClient::handleError(QNetworkReply::NetworkError error)
{
  qDebug() <<"error:"<< error;
}

void OneRpcClient::retrieveAllHostInfo(void)
{
  QVariantList rpcArgs;
  rpcArgs.push_back("oneadmin:oneadmin");
  mreply = call("one.hostpool.info", rpcArgs);
  connect(mreply, SIGNAL(finished()), this, SLOT(handleFinished()));
  connect(mreply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
}

int main(int argc, char *argv[])
{
  //    Q_INIT_RESOURCE(application);
  QCoreApplication app(argc, argv);
  OneRpcClient rpcClient;
  rpcClient.retrieveAllHostInfo();
  return app.exec();
}
