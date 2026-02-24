#include "server/connection.h"
#include <iostream>

Connection::Connection(boost::asio::ip::tcp::socket socket) : _socket(std::move(socket)) {}

Connection::~Connection() = default;

void Connection::start() {
    // Start reading from the socket or writing to it as needed
}

void Connection::stop() {
    // Stop reading from the socket or writing to it as needed
}

boost::asio::ip::tcp::socket& Connection::socket() {
    return _socket;
}