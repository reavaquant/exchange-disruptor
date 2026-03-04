#ifndef SERVER_H
#define SERVER_H

#include <cstdint>
#include <string>
#include <boost/asio.hpp>
#include "protocol/codec.h"
#include "matching_engine/matching_engine.h"


enum class IPVersion { IPv4, IPv6 };

class Server : public std::enable_shared_from_this<Server> {
public:
    using pointer = std::shared_ptr<Server>;
    static pointer create(IPVersion ipv, uint16_t port, std::string symbol);
    ~Server();

    int run();
private:
    Server(IPVersion ipv, uint16_t port, std::string symbol);
    void doAccept();

    Codec _codec;
    MatchingEngine _matchingEngine;

    IPVersion _ipv;
    uint16_t _port;

    boost::asio::io_context _ioContext;
    boost::asio::ip::tcp::acceptor _acceptor;

    std::string _symbol;
};

#endif // SERVER_H