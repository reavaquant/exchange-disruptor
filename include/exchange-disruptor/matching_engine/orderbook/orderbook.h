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

    const Book& getBidBook() const &;
    const Book& getBidBook() const && = delete;
    const Book& getAskBook() const &;
    const Book& getAskBook() const && = delete;
    const std::string& getSymbol() const &;
    const std::string& getSymbol() const && = delete;

    std::optional<RejectReason> addLimit(const Order& order); 
    std::optional<RejectReason> cancelOrder(uint64_t orderId, uint64_t clientId);

    Order* peekBid();
    const Order* peekBid() const;
    Order* peekAsk();
    const Order* peekAsk() const;
    std::optional<int64_t> topBidPrice() const;
    std::optional<int64_t> topAskPrice() const;
    bool consumeBid();
    bool consumeAsk();

    bool hasOrderId(uint64_t orderId) const;

private:
    std::string _symbol;
    Book _bidBook;
    Book _askBook;
    std::unordered_map<uint64_t, BookSide> _orderIdToBookSide; // orderId -> book side for quick cancellation
};

#endif // ORDERBOOK_H
