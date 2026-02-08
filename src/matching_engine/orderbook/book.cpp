#include "book.h"

Book::Book(BookSide side) : _side(side) {}

bool Book::addLimit(const Order& order) {
    if (order.getPrice() <= 0 || order.getQtyRemaining() <= 0) {
        return false; // Invalid price or quantity
    }
    if ((order.getSide() == Side::Buy && _side != BookSide::Bid) || (order.getSide() == Side::Sell && _side != BookSide::Ask)) {
        return false; // Order side does not match book side
    }
    if (_orderIdMap.find(order.getOrderId()) != _orderIdMap.end()) { return false; } // Duplicate order ID
    auto [levelIt, created] = _book.emplace(order.getPrice(), std::list<Order>{});
    levelIt->second.push_back(order);
    _orderIdMap[order.getOrderId()] = {levelIt, std::prev(levelIt->second.end())};
    return true;
}

bool Book::cancelOrder(uint64_t orderId, uint64_t clientId) { 
    auto it = _orderIdMap.find(orderId);
    if (it == _orderIdMap.end()) {
        return false; // Order ID not found
    }
    Locator locator = it->second;
    if (locator.orderIt->getClientId() != clientId) {
        return false; // 
    }
    _orderIdMap.erase(it);
    locator.levelIt->second.erase(locator.orderIt);
    if (locator.levelIt->second.empty()) {
        _book.erase(locator.levelIt);
    }
    return true;
}

bool Book::empty() const {
    return _book.empty();
}

Order* Book::topOrder() {
    if (_book.empty()) {
        return nullptr;
    }
    if (_side == BookSide::Bid) {
        return &_book.rbegin()->second.front();
    } else {
        return &_book.begin()->second.front();
    }
}

const Order* Book::topOrder() const {
    if (_book.empty()) {
        return nullptr; // Return nullptr if book is empty to avoid throwing exceptions in const context
    }
    if (_side == BookSide::Bid) {
        return &_book.rbegin()->second.front();
    } else {
        return &_book.begin()->second.front();
    }
}

bool Book::purgeTop() {
    if (_book.empty()) {
        return false; // Book is empty, nothing to purge
    }
    auto levelIt = (_side == BookSide::Bid) ? std::prev(_book.end()) : _book.begin();
    const auto& topOrder = levelIt->second.front(); // top order reference for order ID retrieval and no copies
    _orderIdMap.erase(topOrder.getOrderId());
    levelIt->second.pop_front();
    if (levelIt->second.empty()) {
        _book.erase(levelIt);
    }
    return true;
}