#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "matching_engine/orderbook/book.h"
#include "matching_engine/enum.h"
#include <string>
#include <optional>
#include <unordered_map>

class OrderBook {
public:
    OrderBook(std::string symbol);

    const Book& getBidBook() const;
    const Book& getAskBook() const;
    const std::string& getSymbol() const;

    std::optional<RejectReason> addLimit(const Order& order); 
    std::optional<RejectReason> cancelOrder(uint64_t orderId, uint64_t clientId);
    bool purgeTop();

private:
    Book _bidBook;
    Book _askBook;
    std::string _symbol;
    std::unordered_map<uint64_t, BookSide> _orderIdToBookSide; // orderId -> book side for quick cancellation
};

#endif // ORDERBOOK_H