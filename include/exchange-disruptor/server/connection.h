#ifndef CONNECTION_H
#define CONNECTION_H

#include <boost/asio.hpp>

class Connection : public std::enable_shared_from_this<Connection> {
public:

private:
    boost::asio::ip::tcp::socket _socket;
};

#endif // CONNECTION_H