#include "core/ZmqHelper.hpp"
#include "core/MonitorBroker.hpp"
#include <sstream>
#include <iostream>

const int TIMEOUT = 2500 ;
const int NUM_RETRIES = 3 ;

ZmqHelper::ZmqHelper() {}

zmq::socket_t * ZmqHelper::initCliChannel(zmq::context_t & context, const std::string & uri) {

    pingSocket(context, uri) ;
    return createCliSocket(context, uri);
}

zmq::socket_t * ZmqHelper::createCliSocket(zmq::context_t & context, const std::string & uri) {
	std::cout << "I: connecting to server…" << std::endl;
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

void ZmqHelper::pingSocket(zmq::context_t & context, const std::string & uri) {

    zmq::socket_t * client = createCliSocket(context, uri);

    int retriesLeft = NUM_RETRIES;
    while (retriesLeft) {

        std::stringstream msg("PING");
        sendFromSocket(*client, msg.str());

        sleep (1);

        bool expectReply = true;
        while (expectReply) {
            zmq::pollitem_t items[] = { { *client, 0, ZMQ_POLLIN, 0 } };
            zmq::poll (&items[0], 1, TIMEOUT * 1000);

            if (items[0].revents & ZMQ_POLLIN) {
                std::string reply = recvFromSocket(*client);
                if(reply=="ALIVE") {
                    std::cout << "I: connexion etablished" << std::endl;
                    delete client;
                    return ;
                } else {
                    std::cout << "E: Unexpected response : " << reply << "" << std::endl;
                }
            }
            else
                if (--retriesLeft == 0) {
                    std::cout << "E: server seems to be offline, abandoning" << std::endl;
                    expectReply = false;

                    exit(1);
                }
                else {
                    std::cout << retriesLeft<<std::endl;
                    std::cout << "W: no response from server, retrying…" << std::endl;
                    delete client;
                    client = createCliSocket(context);
                    sendFromSocket(*client, msg.str());
                }
        }
    }
    delete client;
}
