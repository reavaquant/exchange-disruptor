#ifndef FILL_EVENT_H
#define FILL_EVENT_H

#include "event.h"

class FillEvent : public Event {
public:
    FillEvent(uint64_t clientId, uint64_t orderId, uint64_t matchId, double price, double qty) : Event(clientId, orderId), _matchId(matchId), _price(price), _qty(qty) {}
    EventType getType() const override;
    uint64_t getMatchId() const;
    double getPrice() const;
    double getQty() const;
private:
    uint64_t _matchId;
    double _price;
    double _qty;
};

#endif // FILL_EVENT_H