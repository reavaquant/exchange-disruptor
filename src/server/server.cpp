#include "server/server.h"
#include "server/connection.h"
#include <iostream>

Server::Server(IPVersion ipv, uint16_t port) : _ipv(ipv), _port(port), _acceptor(_ioContext, boost::asio::ip::tcp::endpoint(ipv == IPVersion::IPv4 ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(), port)) {
    doAccept();
}

Server::~Server() = default;

void Server::doAccept() {
    Connection::pointer connection = Connection::create(_ioContext);
    _acceptor.async_accept(connection->socket(), [this, connection](boost::system::error_code ec) {
        if (!ec) {
            connection->start();
        }
        doAccept();
    });
}

// int Server::run() {
//     try
//     {
//         doAccept();
//         _ioContext.run();
//     }
//     catch(const std::exception& e)
//     {
//         std::cerr << e.what() << '\n';
//         return -1;
//     }
//     return 0;
// }


