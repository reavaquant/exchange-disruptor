#include "server/server.h"

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
