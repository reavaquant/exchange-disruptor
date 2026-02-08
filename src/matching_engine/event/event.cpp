#include "matching_engine/event/event.h"

Event::Event(uint64_t clientId, uint64_t orderId) : _clientId(clientId), _orderId(orderId) {}

const uint64_t& Event::getClientId() const { return _clientId; }
const uint64_t& Event::getOrderId() const { return _orderId; }
