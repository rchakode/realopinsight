#include "ZmqLivestatusHelper.hpp"
#include "LsHelper.hpp"
#include "ZmqSocket.hpp"
#include "zmq.h"

ZmqLivestatusHelper::ZmqLivestatusHelper()
{
}

int
ZmqLivestatusHelper::loadChecks(const SourceT& srcInfo, const QString& host, ChecksT& checks)
{
  QString uri = QString("tcp://%1:%2").arg(srcInfo.ls_addr, QString::number(srcInfo.ls_port));
  ZmqSocket zmqHelper(uri.toStdString(), ZMQ_REQ);

  QString data;
  QString result;

  // request host info
  data = QString("%1:%2").arg(srcInfo.auth, QString(LsHelper::prepareRequestData(host, LsHelper::Host)));
  qDebug()<<data;
  zmqHelper.send(data.toStdString());
  result = QString::fromStdString(zmqHelper.recv());

  qDebug() << result <<"\n";


//  LsHelper::parseResult(result, checks);

  // request service info
  data = QString("%1:%2").arg(srcInfo.auth, QString(LsHelper::prepareRequestData(host, LsHelper::Service)));
  zmqHelper.send(data.toStdString());
  result = QString::fromStdString(zmqHelper.recv());

  qDebug() << result <<"\n";

  //LsHelper::parseResult(result, checks);

  return 0;
}
