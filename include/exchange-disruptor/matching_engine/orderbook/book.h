#ifndef BOOK_H
#define BOOK_H

#include "order.h"
#include <map>
#include <unordered_map>
#include <list>

enum class BookSide { Bid, Ask };

class Book {
public:

private:
    using LevelIt = std::map<int64_t, std::list<Order>>::iterator;
    using OrderIt = std::list<Order>::iterator;
    struct Locator {
        LevelIt levelIt;
        OrderIt orderIt;
    };

    BookSide _side;
    std::map<int64_t, std::list<Order>> _book; // price -> list of orders at that price
    std::unordered_map<uint64_t, Locator> _orderIdMap; // orderId -> iterators for quick access
};

#endif // BOOK_H
