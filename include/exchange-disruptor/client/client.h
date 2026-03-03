#ifndef CLIENT_H
#define CLIENT_H

#include <deque>
#include <boost/asio.hpp>
#include "protocol/codec.h"
#include "protocol/framer.h"


class Client : public std::enable_shared_from_this<Client> {
public:
    Client(boost::asio::io_context& ioContext, const std::string& host, uint16_t port, Codec& codec);

    void stop();
    boost::asio::ip::tcp::socket& socket();
    void post(const Command& cmd);

private:
    void connect(const boost::asio::ip::tcp::resolver::results_type& endpoints);
    void asyncRead();
    void handleEvent(const Event& event);
    void asyncWrite();

    boost::asio::ip::tcp::socket _socket;
    boost::asio::io_context& _ioContext;
    std::string _host;
    uint16_t _port;
    Framer _framer;
    Codec _codec;

    std::array<uint8_t, 4096> _buffer;
    std::deque<std::vector<uint8_t>> _writeQueue; // outgoing payloads queue
    bool _writeInProgress = false;
};

#endif // CLIENT_H