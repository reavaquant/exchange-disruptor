#include "server/connection.h"
#include "matching_engine/matching_engine.h"
#include <utility>


Connection::Connection(boost::asio::io_context& ioContext, Codec& codec, MatchingEngine& matchingEngine) : _socket(ioContext), _codec(codec), _matchingEngine(matchingEngine) {}

Connection::pointer Connection::create(boost::asio::io_context& ioContext, Codec& codec, MatchingEngine& matchingEngine) {
    return Connection::pointer(new Connection(ioContext, codec, matchingEngine));
}

void Connection::start() {
    // Start reading from the socket or writing to it as needed
    asyncRead();
}

void Connection::stop() {
    _writeInProgress = false;
    if (!_socket.is_open()) {
        return;
    }

    boost::system::error_code ec;
    _socket.cancel(ec);
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    _socket.close(ec);
    _writeQueue.clear();
}

boost::asio::ip::tcp::socket& Connection::socket() {
    return _socket;
}

void Connection::asyncRead() { // Read data from the socket
    auto self(shared_from_this());
    _socket.async_read_some(boost::asio::buffer(_buffer),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::vector<std::vector<uint8_t>> messages = _framer.consume(std::span<const uint8_t>(_buffer.data(), length));
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
                    handleCmd(*cmd);
                }
                asyncRead();
            } else {
                stop();
            }
        });
}

void Connection::handleCmd(const Command& cmd) {
    // process dans IN_BUS de disruptor apres
    std::vector<std::unique_ptr<Event>> events = _matchingEngine.process(cmd);
    for (const auto& event : events) {
        std::vector<uint8_t> encodedEvent = _codec.encodeEvent(*event);
        std::vector<uint8_t> framedEvent = _framer.frame(encodedEvent);
        enqueueRsp(std::move(framedEvent));
    }
}

void Connection::enqueueRsp(std::vector<uint8_t> rsp) {
    if (!_socket.is_open()) {
        return;
    }
    if (_writeQueue.size() >= 1024) { // Arbitrary limit to prevent unbounded growth
        stop();
        return;
    }

    const bool startWrite = !_writeQueue.empty();
    _writeQueue.push_back(std::move(rsp));
    if (!startWrite) {
        _writeInProgress = true;
        asyncWrite();
    }
}

void Connection::asyncWrite() {
    if (_writeQueue.empty()) {
        _writeInProgress = false;
        return;
    }

    auto self(shared_from_this());
    boost::asio::async_write(_socket, boost::asio::buffer(_writeQueue.front()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                _writeQueue.pop_front();
                if (!_writeQueue.empty()) {
                    asyncWrite();
                } else {
                    _writeInProgress = false;
                }
            } else {
                _writeInProgress = false;
                stop();
            }
        });
}
