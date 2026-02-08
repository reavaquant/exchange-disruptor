#ifndef BOOK_H
#define BOOK_H

#include "matching_engine/order/order.h"
#include "matching_engine/enum.h"
#include <map>
#include <list>
#include <optional>
#include <unordered_map>

enum class BookSide { Bid, Ask };

class Book {
public:
    Book(BookSide side);

    std::optional<RejectReason> addLimit(const Order& order);
    std::optional<RejectReason> cancelOrder(uint64_t orderId, uint64_t clientId);

    bool empty() const;
    Order* topOrder();
    const Order* topOrder() const;

    bool purgeTop();
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
