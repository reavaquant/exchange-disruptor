#ifndef FILL_EVENT_H
#define FILL_EVENT_H

#include "matching_engine/event/event.h"

class FillEvent : public Event {
public:
    FillEvent(uint64_t clientId, uint64_t orderId, uint64_t matchId, int64_t price, double qty);
    const EventType& getType() const override;
    const uint64_t& getMatchId() const;
    const int64_t& getPrice() const;
    const double& getQty() const;
private:
    uint64_t _matchId;
    int64_t _price;
    double _qty;
};

#endif // FILL_EVENT_H
