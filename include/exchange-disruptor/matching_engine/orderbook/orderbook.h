#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "book.h"

class OrderBook {
public:
    OrderBook(std::string symbol);

    const Book& getBidBook() const;
    const Book& getAskBook() const;
    std::string getSymbol() const;

    bool addLimit(const Order& order);
    bool cancelOrder(uint64_t orderId, uint64_t clientId);
    bool purgeTop();

private:
    Book _bidBook;
    Book _askBook;
    std::string _symbol;
};


#endif // ORDERBOOK_H