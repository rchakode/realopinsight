#ifndef ZMQHELPER_HPP
#define ZMQHELPER_HPP
#include "zmq.hpp"
#include <string>

class ZmqHelper
{
public:
	ZmqHelper();

    static zmq::socket_t *
    initCliChannel(zmq::context_t & context, const std::string & uri="tcp://localhost:1983");

	static zmq::socket_t *
    createCliSocket(zmq::context_t & context, const std::string & uri="tcp://localhost:1983");

    static void
    pingSocket(zmq::context_t & context, const std::string & uri="tcp://localhost:1983");

	static bool
	sendFromSocket(zmq::socket_t & socket, const std::string & string) ;

	static std::string
	recvFromSocket (zmq::socket_t & socket);

};

#endif // ZMQHELPER_HPP
