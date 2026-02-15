#include "matching_engine/orderbook/orderbook.h"

OrderBook::OrderBook(std::string symbol) : _symbol(symbol), _bidBook(BookSide::Bid), _askBook(BookSide::Ask) {}

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

Order* OrderBook::peekBid() {
    return _bidBook.peek();
}

const Order* OrderBook::peekBid() const {
    return _bidBook.peek();
}

Order* OrderBook::peekAsk() {
    return _askBook.peek();
}

const Order* OrderBook::peekAsk() const {
    return _askBook.peek();
}

std::optional<int64_t> OrderBook::topBidPrice() const {
    const Order* topBid = _bidBook.peek();
    if (topBid) {
        return topBid->getPrice();
    }
    return std::nullopt;
}

std::optional<int64_t> OrderBook::topAskPrice() const {
    const Order* topAsk = _askBook.peek();
    if (topAsk) {
        return topAsk->getPrice();
    }
    return std::nullopt;
}

bool OrderBook::consumeBid() {
    auto topBid = _bidBook.peek();
    if (topBid == nullptr) {
        return false;
    }
    uint64_t orderId = topBid->getOrderId();
    bool purged = _bidBook.consume();
    if (purged) {
        _orderIdToBookSide.erase(orderId);
    }
    return purged;
}

bool OrderBook::consumeAsk() {
    auto topAsk = _askBook.peek();
    if (topAsk == nullptr) {
        return false;
    }
    uint64_t orderId = topAsk->getOrderId();
    bool purged = _askBook.consume();
    if (purged) {
        _orderIdToBookSide.erase(orderId);
    }
    return purged;
}

bool OrderBook::hasOrderId(uint64_t orderId) const {
    return _orderIdToBookSide.find(orderId) != _orderIdToBookSide.end();
}