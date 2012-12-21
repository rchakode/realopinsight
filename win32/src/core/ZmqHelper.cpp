#include "core/ZmqHelper.hpp"
#include "core/MonitorBroker.hpp"
#include "client/Utils.hpp"
#include <sstream>
#include <iostream>

const int TIMEOUT = 15000 ;
const int NUM_RETRIES = 3 ;

ZmqHelper::ZmqHelper() {}

zmq::socket_t * ZmqHelper::initCliChannel(zmq::context_t & context, const std::string & uri, string & srvVer) {

    zmq::socket_t * socket = NULL;
    if (pingServer(context, uri, srvVer)) {
        socket = createCliSocket(context, uri);
    }
    return socket;
}

void
ZmqHelper::endCliChannel(zmq::socket_t* & socket) {
    if(socket) {
        socket->close();
        delete socket ;
    }
}

zmq::socket_t * ZmqHelper::createCliSocket(zmq::context_t & context, const std::string & uri) {
    std::cout << "INFO: Connecting to server…" << std::endl;
    zmq::socket_t * client = new zmq::socket_t(context, ZMQ_REQ);
    client->connect(uri.c_str());

    //  Configure socket to not wait at close time
    int linger = 0;
    client->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));

    return client;
}

bool
ZmqHelper::sendFromSocket(zmq::socket_t & socket, const std::string & mcontent) {

    zmq::message_t message(mcontent.size());
    memcpy(message.data(), mcontent.data(), mcontent.size());
    bool rc = socket.send(message);

    return (rc);
}

std::string
ZmqHelper::recvFromSocket(zmq::socket_t & socket) {

    zmq::message_t message;
    socket.recv(&message);
    return std::string(static_cast<char*>(message.data()), message.size());
}

bool ZmqHelper::pingServer(zmq::context_t & context, const std::string & uri, string & srvVer) {

    zmq::socket_t * client = createCliSocket(context, uri);

    int retriesLeft = NUM_RETRIES;
    while (retriesLeft) {

        std::stringstream msg("PING");
        sendFromSocket(*client, msg.str());

        Utils::delay(1);

        bool expectReply = true;
        while (expectReply) {
            zmq::pollitem_t items[] = { { *client, 0, ZMQ_POLLIN, 0 } };
            zmq::poll (&items[0], 1, TIMEOUT * 1000);

            if (items[0].revents & ZMQ_POLLIN) {
                std::string reply = recvFromSocket(*client);
                size_t pos = reply.find(":");
                string respType = reply.substr(0, pos);
                if(respType == "ALIVE") {
                    srvVer = reply.substr(pos+1, string::npos);
                    std::cout << "INFO: Connection etablished with server " << srvVer <<"\n";
                    delete client;
                    return true;
                } else {
                    std::cout << "ERROR: Weird response from the server\n";
                }
            }
            else {
                if (--retriesLeft == 0) {
                    std::cout << "ERROR: Server seems to be offline, abandoning\n";
                    expectReply = false;
                }
                else {
                    std::cout << "WARNING: No response from server, retrying…\n";
                    delete client;
                    client = createCliSocket(context);
                    sendFromSocket(*client, msg.str());
                }
            }
        }
    }
    srvVer="0.0.0";
    delete client;

    return false ;
}
