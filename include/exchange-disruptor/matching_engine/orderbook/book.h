#ifndef BOOK_H
#define BOOK_H

#include "order.h"
#include <map>
#include <unordered_map>
#include <list>

enum class BookSide { Bid, Ask };

class Book {
public:
    Book(BookSide side) : _side(side) {}
    void addOrder(const Order& order);
    void removeExecutedOrder(uint64_t orderId);
    void cancelOrder(uint64_t orderId);
    std::list<Order> getTopLevels(int n) const; // Get top n levels of the book

private:
    using Orders = std::list<Order>;
    using Levels = std::map<int64_t, Orders>;

    using LevelIt = Levels::iterator;
    using OrderIt = Orders::iterator;

    struct Locator {
        LevelIt levelIt; 
        OrderIt orderIt; 
    };

    BookSide _side; 
    Levels _book; // price -> list of orders at that price
    std::unordered_map<uint64_t, Locator> _orderIdMap; // orderId -> iterators for quick access
};

#endif // BOOK_H
