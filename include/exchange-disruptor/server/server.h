#ifndef SERVER_H
#define SERVER_H

#include <cstdint>
#include <boost/asio.hpp>
#include "protocol/codec.h"


enum class IPVersion { IPv4, IPv6 };

class Server : public std::enable_shared_from_this<Server> {
public:
    Server(IPVersion ipv, uint16_t port);
    ~Server();

    int run();
private:
    void doAccept();

    Codec _codec;

    IPVersion _ipv;
    uint16_t _port;

    boost::asio::io_context _ioContext;
    boost::asio::ip::tcp::acceptor _acceptor;
};

#endif // SERVER_H