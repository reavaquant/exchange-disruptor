#include "client/client.h"
#include <exception>
#include <iostream>

Client::Client(boost::asio::io_context& ioContext, const std::string& host, uint16_t port, Codec& codec)
    : _socket(ioContext), _resolver(ioContext), _host(host), _port(port), _codec(codec) {}

Client::pointer Client::create(boost::asio::io_context& ioContext, const std::string& host, uint16_t port, Codec& codec) {
    pointer client(new Client(ioContext, host, port, codec));
    client->start();
    return client;
}

void Client::start() {
    _stopped = false;

    auto self = shared_from_this();
    _resolver.async_resolve(_host, std::to_string(_port),
        [this, self](const boost::system::error_code& ec,
                     const boost::asio::ip::tcp::resolver::results_type& endpoints) {
            if (ec) {
                std::cerr << "Failed to resolve host: " << ec.message() << std::endl;
                stop();
                return;
            }
            connect(endpoints);
        });
}

void Client::connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) {
    auto self(shared_from_this());
    boost::asio::async_connect(_socket, endpoints,
        [this, self](boost::system::error_code ec, const boost::asio::ip::tcp::endpoint& /*endpoint*/) {
            if (!ec) {
                _connected = true;
                asyncRead();

                if (!_writeQueue.empty() && !_writeInProgress) {
                    _writeInProgress = true;
                    asyncWrite();
                }
                return;
            }
            std::cerr << "Failed to connect: " << ec.message() << std::endl;
            stop();
        });
}

void Client::stop() {
    auto self = shared_from_this();
    boost::asio::dispatch(_socket.get_executor(), [this, self]() {
        stopImpl();
    });
}


void Client::stopImpl() {
    _stopped = true;
    _connected = false;
    _writeInProgress = false;
    _writeQueue.clear();

    _resolver.cancel();

    if (!_socket.is_open()) {
        return;
    }

    boost::system::error_code ec;
    _socket.cancel(ec);
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    _socket.close(ec);
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
    std::cout << "Received event: " << static_cast<int>(event.getType())
              << " for clientId: " << event.getClientId()
              << " orderId: " << event.getOrderId() << std::endl;
}

void Client::enqueueWrite(std::vector<uint8_t> payload) {
    if (_stopped) return;
    if (_writeQueue.size() >= 1024) {
        stop();
        return;
    }

    _writeQueue.push_back(std::move(payload));

    if (_connected && !_writeInProgress) {
        _writeInProgress = true;
        asyncWrite();
    }
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

void Client::post(std::unique_ptr<Command> cmd) {
    if (!cmd) {
        return;
    }

    auto self = shared_from_this();
    boost::asio::post(_socket.get_executor(),
        [this, self, cmd = std::move(cmd)]() mutable {
            if (_stopped) {
                return;
            }
            try {
                auto bytes = _codec.encodeCommand(*cmd);
                auto framed = _framer.frame(bytes);
                enqueueWrite(std::move(framed));
            } catch (const std::exception& ex) {
                std::cerr << "Failed to encode command: " << ex.what() << std::endl;
                stopImpl();
            } catch (...) {
                std::cerr << "Failed to encode command: unknown error" << std::endl;
                stopImpl();
            }
        });
}
