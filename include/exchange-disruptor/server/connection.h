#ifndef CONNECTION_H
#define CONNECTION_H

#include <boost/asio.hpp>

class Connection : public std::enable_shared_from_this<Connection> {
public:
    typedef std::shared_ptr<Connection> pointer;

    static pointer create(boost::asio::io_context& ioContext);

    void start();
    void stop();
    boost::asio::ip::tcp::socket& socket();

private:
    explicit Connection(boost::asio::io_context& ioContext);

    void doReadHeader();
    void doReadBody();
    void doWrite();

    boost::asio::ip::tcp::socket _socket;
    // ajout d'un truc qui doit lire ou ecrire pour command ou event
};

#endif // CONNECTION_H