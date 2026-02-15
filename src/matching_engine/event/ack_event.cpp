#include "matching_engine/event/ack_event.h"

AckEvent::AckEvent(uint64_t clientId, uint64_t orderId) : Event(clientId, orderId) {}

EventType AckEvent::getType() const { return EventType::Ack; }
