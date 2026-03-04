#ifndef CONNECTION_H
#define CONNECTION_H

#include <boost/asio.hpp>
#include <array>
#include <cstddef>
#include <stdint.h>
#include <deque>
#include <vector>
#include "protocol/framer.h"
#include "protocol/codec.h"
#include "matching_engine/matching_engine.h"

class Connection : public std::enable_shared_from_this<Connection> {
public:
    typedef std::shared_ptr<Connection> pointer;

    static pointer create(boost::asio::io_context& ioContext, Codec& codec, MatchingEngine& matchingEngine);

    void start();
    void stop();
    boost::asio::ip::tcp::socket& socket();

private:
    explicit Connection(boost::asio::io_context& ioContext, Codec& codec, MatchingEngine& matchingEngine);

    void asyncRead();
    void handleCmd(const Command& cmd);
    void enqueueRsp(std::vector<uint8_t> rsp);
    void asyncWrite();

    boost::asio::ip::tcp::socket _socket;
    std::array<uint8_t, 4096> _buffer; 
    Framer _framer;
    Codec& _codec;
    MatchingEngine& _matchingEngine;
    std::deque<std::vector<uint8_t>> _writeQueue; // outgoing payloads queue
    bool _writeInProgress = false;
};

#endif // CONNECTION_H