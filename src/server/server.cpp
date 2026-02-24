#include "server/server.h"
#include <iostream>

Server::Server(IPVersion ipv, uint16_t port) : _ipv(ipv), _port(port), _acceptor(_ioContext, boost::asio::ip::tcp::endpoint(ipv == IPVersion::IPv4 ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(), port)) {
    doAccept();
}

Server::~Server() = default;

void Server::doAccept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(_ioContext);
    _acceptor.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
        if (!error) {
            // Handle the new connection (e.g., start reading from the socket) with tcpConnection class
        }
        doAccept(); // Accept the next connection
    });
}

int Server::run() {
    try
    {
        doAccept();
        _ioContext.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }
    return 0;
}
