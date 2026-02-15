#include "matching_engine/event/reject_event.h"

RejectEvent::RejectEvent(uint64_t clientId, uint64_t orderId, RejectReason reason) : Event(clientId, orderId), _reason(reason) {}

EventType RejectEvent::getType() const { return EventType::Reject; }

RejectReason RejectEvent::getReason() const { return _reason; }
