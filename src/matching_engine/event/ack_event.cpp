#include "matching_engine/event/ack_event.h"

AckEvent::AckEvent(uint64_t clientId, uint64_t orderId) : Event(clientId, orderId) {}

const EventType& AckEvent::getType() const {
    static const EventType kType = EventType::Ack;
    return kType;
}
