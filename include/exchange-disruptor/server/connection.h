#ifndef CONNECTION_H
#define CONNECTION_H

#include <boost/asio.hpp>

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(boost::asio::ip::tcp::socket socket);
    ~Connection();

    void start();
    void stop();
    boost::asio::ip::tcp::socket& socket();

private:
    boost::asio::ip::tcp::socket _socket;
    // ajout d'un truc qui doit lire ou ecrire pour command ou event
};

#endif // CONNECTION_H