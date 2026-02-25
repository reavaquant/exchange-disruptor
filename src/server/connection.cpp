#include "server/connection.h"
#include "matching_engine/matching_engine.h"


Connection::Connection(boost::asio::io_context& ioContext, Codec& codec, MatchingEngine& matchingEngine) : _socket(ioContext), _codec(codec), _matchingEngine(matchingEngine) {}

Connection::pointer Connection::create(boost::asio::io_context& ioContext, Codec& codec, MatchingEngine& matchingEngine) {
    return Connection::pointer(new Connection(ioContext, codec, matchingEngine));
}

void Connection::start() {
    // Start reading from the socket or writing to it as needed
    doRead();
}

void Connection::stop() {
    // Stop reading from the socket and handle error_code
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
                for (const auto& msg : messages) {
                    std::unique_ptr<Command> cmd;
                    try {
                        cmd = _codec.decodeCommand(msg);
                    } catch (...) {
                        stop();
                        return;
                    }
                    if (cmd == nullptr) {
                        stop();
                        return;
                    }
                    // process dans IN_BUS de disruptor apres
                    std::vector<std::unique_ptr<Event>> events = _matchingEngine.process(*cmd);
                    for (const auto& event : events) {
                        std::vector<uint8_t> encodedEvent = _codec.encodeEvent(*event);
                        std::vector<uint8_t> framedEvent = _framer.frame(encodedEvent);
                        // _writeQueue.push_back(framedEvent);
                    }
                }
                doRead();
            } else {
                stop();
            }
        });
}

void Connection::doWrite() {
    // Write data to the socket
}
