#include "matching_engine/orderbook/orderbook.h"

OrderBook::OrderBook(std::string symbol) : _bidBook(BookSide::Bid), _askBook(BookSide::Ask), _symbol(symbol) {}

const Book& OrderBook::getBidBook() const & { return _bidBook; }
const Book& OrderBook::getAskBook() const & { return _askBook; }
const std::string& OrderBook::getSymbol() const & { return _symbol; }

std::optional<RejectReason> OrderBook::addLimit(const Order& order) {
    if (order.getSide() == Side::Buy) {
        auto rejectReason = _bidBook.addLimit(order);
        if (!rejectReason) {
            _orderIdToBookSide[order.getOrderId()] = BookSide::Bid;
        }
        return rejectReason;
    } else {
        auto rejectReason = _askBook.addLimit(order);
        if (!rejectReason) {
            _orderIdToBookSide[order.getOrderId()] = BookSide::Ask;
        }
        return rejectReason;
    }
}

std::optional<RejectReason> OrderBook::cancelOrder(uint64_t orderId, uint64_t clientId) {
    auto it = _orderIdToBookSide.find(orderId);
    if (it == _orderIdToBookSide.end()) {
        return RejectReason::UnknownOrderId; // Order ID not found
    }
    BookSide side = it->second;
    if (side == BookSide::Bid) {
        auto rejectReason = _bidBook.cancelOrder(orderId, clientId);
        if (!rejectReason) {
            _orderIdToBookSide.erase(it);
        }
        return rejectReason;
    } else {
        auto rejectReason = _askBook.cancelOrder(orderId, clientId);
        if (!rejectReason) {
            _orderIdToBookSide.erase(it);
        }
        return rejectReason;
    }
}
