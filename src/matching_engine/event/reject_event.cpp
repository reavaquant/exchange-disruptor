#include "matching_engine/event/reject_event.h"

RejectEvent::RejectEvent(uint64_t clientId, uint64_t orderId, RejectReason reason) : Event(clientId, orderId), _reason(reason) {}

const EventType& RejectEvent::getType() const {
    static const EventType kType = EventType::Reject;
    return kType;
}

const RejectReason& RejectEvent::getReason() const { return _reason; }
