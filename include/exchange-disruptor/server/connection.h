#ifndef CONNECTION_H
#define CONNECTION_H

#include <boost/asio.hpp>
#include <array>
#include <stdint.h>
#include <deque>
#include <vector>
#include "protocol/framer.h"

class Connection : public std::enable_shared_from_this<Connection> {
public:
    typedef std::shared_ptr<Connection> pointer;

    static pointer create(boost::asio::io_context& ioContext);

    void start();
    void stop();
    boost::asio::ip::tcp::socket& socket();

private:
    explicit Connection(boost::asio::io_context& ioContext);

    void doRead();
    void doWrite();

    boost::asio::ip::tcp::socket _socket;
    std::array<uint8_t, 4096> _buffer; // reading data
    Framer _framer; // message framing
    std::deque<std::vector<uint8_t>> _writeQueue; // outgoing payloads queue
};

#endif // CONNECTION_H