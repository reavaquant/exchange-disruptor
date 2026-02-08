#include "matching_engine/event/fill_event.h"

FillEvent::FillEvent(uint64_t clientId, uint64_t orderId, uint64_t matchId, int64_t price, double qty) : Event(clientId, orderId), _matchId(matchId), _price(price), _qty(qty) {
    if (price <= 0) {
        throw std::invalid_argument("Price must be greater than 0");
    }
    if (qty <= 0) {
        throw std::invalid_argument("Quantity must be greater than 0");
    }
}

const EventType& FillEvent::getType() const {
    static const EventType kType = EventType::Fill;
    return kType;
}

const uint64_t& FillEvent::getMatchId() const { return _matchId; }
const int64_t& FillEvent::getPrice() const { return _price; }
const double& FillEvent::getQty() const { return _qty; }
