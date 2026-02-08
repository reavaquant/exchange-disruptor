#ifndef REJECT_EVENT_H
#define REJECT_EVENT_H

#include "matching_engine/event/event.h"

class RejectEvent : public Event {
public:
    RejectEvent(uint64_t clientId, uint64_t orderId, RejectReason reason);
    EventType getType() const override;
    RejectReason getReason() const;
private:
    RejectReason _reason;
};

#endif // REJECT_EVENT_H