#ifndef ACK_EVENT_H
#define ACK_EVENT_H

#include "event.h"

class AckEvent : public Event {
public:
    AckEvent(uint64_t clientId, uint64_t orderId) : Event(clientId, orderId) {}
    EventType getType() const override;
};

#endif // ACK_EVENT_H