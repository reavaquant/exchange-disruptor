#ifndef SERVER_H
#define SERVER_H

#include <cstdint>
#include <boost/asio.hpp>


enum class IPVersion { IPv4, IPv6 };

class Server {
public:
    Server(IPVersion ipv, uint16_t port);
    ~Server();

    int run();
private:
    IPVersion _ipv;
    uint16_t _port;

    boost::asio::io_context _ioContext;
    boost::asio::ip::tcp::acceptor _acceptor;

    void doAccept();
};

#endif // SERVER_H