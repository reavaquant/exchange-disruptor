#include "server/connection.h"

Connection::Connection(boost::asio::io_context& ioContext) : _socket(ioContext) {}

Connection::pointer Connection::create(boost::asio::io_context& ioContext) {
    return Connection::pointer(new Connection(ioContext));
}

void Connection::start() {
    // Start reading from the socket or writing to it as needed
}

void Connection::stop() {
    // Stop reading from the socket or writing to it as needed
}

boost::asio::ip::tcp::socket& Connection::socket() {
    return _socket;
}

void Connection::doReadHeader() {
    // Read the header from the socket
}

void Connection::doReadBody() {
    // Read the body from the socket
}

void Connection::doWrite() {
    // Write data to the socket
}