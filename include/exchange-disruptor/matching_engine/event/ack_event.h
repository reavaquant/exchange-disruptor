#ifndef ACK_EVENT_H
#define ACK_EVENT_H

#include "matching_engine/event/event.h"

class AckEvent : public Event {
public:
    AckEvent(uint64_t clientId, uint64_t orderId);
    const EventType& getType() const override;
};

#endif // ACK_EVENT_H
