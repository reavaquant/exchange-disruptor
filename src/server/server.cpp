#include "server/server.h"

Server::Server(IPVersion ipv, uint16_t port) : _ipv(ipv), _port(port), _acceptor(_ioContext, boost::asio::ip::tcp::endpoint(ipv == IPVersion::IPv4 ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(), port)) {}

Server::~Server() = default;
