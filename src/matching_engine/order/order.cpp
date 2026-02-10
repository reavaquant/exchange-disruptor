#include "matching_engine/order/order.h"
#include "matching_engine/reject_exception.h"

Order::Order(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, int64_t price, int64_t qtyRemaining) : _clientId(clientId), _orderId(orderId), _symbol(symbol), _side(side), _price(price), _qtyRemaining(qtyRemaining) {}

uint64_t Order::getClientId() const { return _clientId; }
uint64_t Order::getOrderId() const { return _orderId; }
const std::string& Order::getSymbol() const & { return _symbol; }
Side Order::getSide() const { return _side; }
int64_t Order::getPrice() const { return _price; }
int64_t Order::getQtyRemaining() const { return _qtyRemaining; }

int64_t Order::qtyDecrease(int64_t execQty) {
    _qtyRemaining -= execQty;
    return _qtyRemaining;
}
