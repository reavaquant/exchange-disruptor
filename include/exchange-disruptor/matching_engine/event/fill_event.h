#ifndef FILL_EVENT_H
#define FILL_EVENT_H

#include "matching_engine/event/event.h"

class FillEvent : public Event {
public:
    FillEvent(uint64_t clientId, uint64_t orderId, uint64_t matchId, int64_t price, int64_t qty);
    EventType getType() const override;
    uint64_t getMatchId() const;
    int64_t getPrice() const;
    int64_t getQty() const;
private:
    uint64_t _matchId;
    int64_t _price;
    int64_t _qty;
};

#endif // FILL_EVENT_H
