#ifndef EVENT_H
#define EVENT_H

#include "matching_engine/enum.h"
#include "matching_engine/command/command.h"

class Event {
public:
    Event(uint64_t clientId, uint64_t orderId);
    virtual ~Event() = default;
    virtual const EventType& getType() const = 0;
    const uint64_t& getClientId() const;
    const uint64_t& getOrderId() const;
private:
    uint64_t _clientId;
    uint64_t _orderId;
};

#endif // EVENT_H
