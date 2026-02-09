#include "matching_engine/matching_engine.h"
#include "matching_engine/command/command.h"
#include "matching_engine/command/limit_command.h"
#include "matching_engine/command/cancel_command.h"
#include "matching_engine/command/market_command.h"
#include "matching_engine/event/event.h"
#include "matching_engine/event/reject_event.h"
#include "matching_engine/event/fill_event.h"
#include "matching_engine/event/ack_event.h"

#include <vector>


MatchingEngine::MatchingEngine(std::string symbol) : _orderBook(symbol) {}

std::vector<Event> MatchingEngine::process(const Command& cmd) { 
    std::vector<Event> events;
    if (cmd.getSymbol() != _symbol) {
        events.emplace_back(RejectEvent(cmd.getClientId(), cmd.getOrderId(), RejectReason::UnknownSymbol));
        return events;
    }
    switch (cmd.getType()) {
        case CommandType::Limit: {
            const LimitCommand& limitCmd = static_cast<const LimitCommand&>(cmd);
            Order order(limitCmd.getClientId(), limitCmd.getOrderId(), limitCmd.getSymbol(), limitCmd.getSide(), limitCmd.getPrice(), limitCmd.getQty());
            //look if crosses and generate FillEvents here before adding to book
            auto rejectReason = _orderBook.addLimit(order);
            if (rejectReason) {
                events.emplace_back(RejectEvent(cmd.getClientId(), cmd.getOrderId(), *rejectReason));
            } else {
                // If the order was accepted, we would normally check for matches and generate FillEvents here
            }
            break;
        }
        case CommandType::Cancel: {
            const CancelCommand& cancelCmd = static_cast<const CancelCommand&>(cmd);
            auto rejectReason = _orderBook.cancelOrder(cancelCmd.getOrderId(), cancelCmd.getClientId());
            if (rejectReason) {
                events.emplace_back(RejectEvent(cmd.getClientId(), cmd.getOrderId(), *rejectReason));
            } else {
                events.emplace_back(AckEvent(cmd.getClientId(), cmd.getOrderId()));
            }
            break;
        }
        case CommandType::Market: {
            const MarketCommand& marketCmd = static_cast<const MarketCommand&>(cmd);
            Order marketOrder(marketCmd.getClientId(), marketCmd.getOrderId(), marketCmd.getSymbol(), marketCmd.getSide(), 0, marketCmd.getQty());
            break;
        }
    }
    return events;
}