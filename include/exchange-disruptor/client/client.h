#ifndef CLIENT_H
#define CLIENT_H

#include <array>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "protocol/codec.h"
#include "protocol/framer.h"

class Client : public std::enable_shared_from_this<Client> {
public:
    using pointer = std::shared_ptr<Client>;
    static pointer create(boost::asio::io_context& ioContext, const std::string& host, uint16_t port, Codec& codec);

    void stop();
    void post(std::unique_ptr<Command> cmd);

private:
    explicit Client(boost::asio::io_context& ioContext, const std::string& host, uint16_t port, Codec& codec);

    void start();
    void stopImpl();
    void connect(const boost::asio::ip::tcp::resolver::results_type& endpoints);
    void asyncRead();
    void handleEvent(const Event& event);
    void enqueueWrite(std::vector<uint8_t> payload);
    void asyncWrite();

    boost::asio::ip::tcp::socket _socket;
    boost::asio::ip::tcp::resolver _resolver;

    std::array<uint8_t, 4096> _buffer;
    std::deque<std::vector<uint8_t>> _writeQueue;
    bool _writeInProgress = false;
    bool _connected = false;
    bool _stopped = false;

    std::string _host;
    uint16_t _port;

    Codec _codec;
    Framer _framer;
};

#endif // CLIENT_H
