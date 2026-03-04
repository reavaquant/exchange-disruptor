#include "server/server.h"
#include "server/connection.h"
#include <iostream>

Server::Server(IPVersion ipv, uint16_t port, std::string symbol)
    : _matchingEngine(std::move(symbol)),
      _ipv(ipv),
      _port(port),
      _acceptor(_ioContext, boost::asio::ip::tcp::endpoint(ipv == IPVersion::IPv4 ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(), port)) {}

Server::pointer Server::create(IPVersion ipv, uint16_t port, std::string symbol) {
    return Server::pointer(new Server(ipv, port, std::move(symbol)));
}

Server::~Server() = default;

void Server::doAccept() {
    auto self(shared_from_this());
    Connection::pointer connection = Connection::create(_ioContext, _codec, _matchingEngine);
    _acceptor.async_accept(connection->socket(), [self, connection](const boost::system::error_code& error) {
        if (!error) {
            connection->start();
            self->doAccept();
        } else if (error != boost::asio::error::operation_aborted) {
            // self->doAccept(); // gerer autrement sinon boucle serre CPU 
        }
    });
}

int Server::run() {
    try {
        doAccept();
        _ioContext.run();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

