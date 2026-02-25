#include "server/connection.h"

Connection::Connection(boost::asio::io_context& ioContext, Codec& codec) : _socket(ioContext), _codec(codec) {}

Connection::pointer Connection::create(boost::asio::io_context& ioContext, Codec& codec) {
    return Connection::pointer(std::make_shared<Connection>(ioContext, codec));
}

void Connection::start() {
    // Start reading from the socket or writing to it as needed
    doRead();
}

void Connection::stop() {
    // Stop reading from the socket or writing to it as needed
}

boost::asio::ip::tcp::socket& Connection::socket() {
    return _socket;
}

void Connection::doRead() { // Read data from the socket
    auto self(shared_from_this());
    _socket.async_read_some(boost::asio::buffer(_buffer),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::vector<std::vector<uint8_t>> messages = _framer.consume(std::span<const uint8_t>(_buffer.data(), length));
                (void)messages; // Process the data read from the socket
                // TODO: iterate on messages, decode them, and push responses to _writeQueue (next will be IN_BUS of disruptor)
                for (const auto& msg : messages) {
                    std::unique_ptr<Command> cmd = _codec.decodeCommand(msg);
                }
                doRead();
            } else {
                // TODO: Handle error (e.g., close the connection)
            }
        });
}

void Connection::doWrite() {
    // Write data to the socket
}
