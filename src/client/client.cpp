#include "client/client.h"
#include <iostream>

Client::Client(boost::asio::io_context& ioContext, const std::string& host, uint16_t port, Codec& codec) 
    : 
    _socket(ioContext), 
    _ioContext(ioContext), 
    _host(host), 
    _port(port), 
    _codec(codec) {
    boost::asio::ip::tcp::resolver resolver(ioContext);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    connect(endpoints);
}

void Client::connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) {
    boost::asio::async_connect(_socket, endpoints,
        [this](boost::system::error_code ec, const boost::asio::ip::tcp::endpoint& /*endpoint*/) {
            if (!ec) {
                asyncRead();
            }
        });
}

void Client::stop() {
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

boost::asio::ip::tcp::socket& Client::socket() {
    return _socket;
}

void Client::asyncRead() {
    auto self(shared_from_this());
    _socket.async_read_some(boost::asio::buffer(_buffer),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::vector<std::vector<uint8_t>> messages = _framer.consume(std::span<const uint8_t>(_buffer.data(), length));
                for (const auto& msg : messages) {
                    std::unique_ptr<Event> event;
                    try {
                        event = _codec.decodeEvent(msg);
                    } catch (...) {
                        stop();
                        return;
                    }
                    if (event == nullptr) {
                        stop();
                        return;
                    }
                    handleEvent(*event);
                }
                asyncRead();
            } else {
                stop();
            }
        });
}

void Client::handleEvent(const Event& event) {
    // afficher event
    std::cout << "Received event: " << static_cast<int>(event.getType()) << " for clientId: " << event.getClientId() << " orderId: " << event.getOrderId() << std::endl;
}

void Client::asyncWrite() {
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

void Client::post(const Command& cmd) {
    std::vector<uint8_t> encodedCmd = _codec.encodeCommand(cmd);
    std::vector<uint8_t> framedCmd = _framer.frame(encodedCmd);
    _writeQueue.push_back(std::move(framedCmd));
    if (!_writeInProgress) {
        _writeInProgress = true;
        asyncWrite();
    }
}