#include "matching_engine/order/order.h"

Order::Order(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, int64_t price, int64_t qtyRemaining) : _clientId(clientId), _orderId(orderId), _symbol(symbol), _side(side), _price(price), _qtyRemaining(qtyRemaining) {
    if (price <= 0) {
        throw std::invalid_argument("Price must be greater than 0");
    }
    if (qtyRemaining <= 0) {
        throw std::invalid_argument("Quantity must be greater than 0");
    }
    if (side != Side::Buy && side != Side::Sell) {
        throw std::invalid_argument("Invalid side");
    }
}

uint64_t Order::getClientId() const { return _clientId; }
uint64_t Order::getOrderId() const { return _orderId; }
const std::string& Order::getSymbol() const & { return _symbol; }
Side Order::getSide() const { return _side; }
int64_t Order::getPrice() const { return _price; }
int64_t Order::getQtyRemaining() const { return _qtyRemaining; }
bool Order::isFilled() const { return _isFilled; }

int64_t Order::qtyDecrease(int64_t execQty) {
    if (execQty <= 0) {
        throw std::invalid_argument("Executed quantity must be greater than 0");
    }
    if (execQty > _qtyRemaining) {
        throw std::invalid_argument("Executed quantity cannot be greater than remaining quantity");
    }
    _qtyRemaining -= execQty;
    if (_qtyRemaining == 0) {
        _isFilled = true;
    }
    return _qtyRemaining;
}
