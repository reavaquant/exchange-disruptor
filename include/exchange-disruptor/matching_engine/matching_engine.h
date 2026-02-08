#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include "orderbook/orderbook.h"
#include "command/command.h"
#include "event/event.h"

class MatchingEngine {
public:
    MatchingEngine();
    Event process(const Command& cmd);
private:
    OrderBook _orderBook;
    uint64_t _nextMatchId;
};

#endif // MATCHING_ENGINE_H