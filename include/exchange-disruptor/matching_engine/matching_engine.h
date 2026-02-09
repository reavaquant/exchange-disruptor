#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include "matching_engine/orderbook/orderbook.h"
#include "matching_engine/command/command.h"
#include "matching_engine/event/event.h"
#include <memory>
#include <vector>

class MatchingEngine {
public:
    MatchingEngine(std::string symbol);
    std::vector<std::unique_ptr<Event>> process(const Command& cmd);
private:
    std::string _symbol;
    OrderBook _orderBook;
    static uint64_t _matchId;
};

#endif // MATCHING_ENGINE_H
