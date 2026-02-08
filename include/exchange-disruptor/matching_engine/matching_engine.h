#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include "matching_engine/orderbook/orderbook.h"
#include "matching_engine/command/command.h"
#include "matching_engine/event/event.h"

class MatchingEngine {
public:
    MatchingEngine();
    Event process(const Command& cmd);
private:
    OrderBook _orderBook;
    uint64_t _nextMatchId;
};

#endif // MATCHING_ENGINE_H