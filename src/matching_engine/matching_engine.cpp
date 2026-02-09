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
            const auto& side = marketCmd.getSide();
            const auto& price = side == Side::Buy ? _orderBook.topAskPrice() : _orderBook.topBidPrice();

            if (!price) {
                events.emplace_back(RejectEvent(cmd.getClientId(), cmd.getOrderId(), RejectReason::InvalidPrice)); // No price available to execute against
                return events;
            }
            Order marketOrder(marketCmd.getClientId(), marketCmd.getOrderId(), marketCmd.getSymbol(), side, *price, marketCmd.getQty());
            double qtyRemaining = marketOrder.getQtyRemaining();

            while (qtyRemaining > 0) {
                Order* topOrder = marketCmd.getSide() == Side::Buy ? _orderBook.peekAsk() : _orderBook.peekBid();
                if (topOrder == nullptr) {
                    break; // No more orders to match against
                }
                int64_t execPrice = topOrder->getPrice();
                double adverseQty = topOrder->getQtyRemaining();
                double execQty = std::min(qtyRemaining, adverseQty);
                if (execQty == adverseQty) {
                    // full fill top order
                    side == Side::Buy ? _orderBook.consumeAsk() : _orderBook.consumeBid(); // Remove the top order
                    events.emplace_back(FillEvent(topOrder->getClientId(), topOrder->getOrderId(), /*matchId=*/0, execPrice, execQty));
                    events.emplace_back(FillEvent(marketOrder.getClientId(), marketOrder.getOrderId(), /*matchId=*/0, execPrice, execQty));
                    marketOrder.qtyDecrease(execQty);
                    continue;
                } else {
                    // partial fill
                    topOrder->qtyDecrease(execQty);
                    marketOrder.qtyDecrease(execQty);
                    events.emplace_back(FillEvent(topOrder->getClientId(), topOrder->getOrderId(), /*matchId=*/0, execPrice, execQty));
                    events.emplace_back(FillEvent(marketOrder.getClientId(), marketOrder.getOrderId(), /*matchId=*/0, execPrice, execQty));
                    continue;
                }
            }
            events.emplace_back(AckEvent(cmd.getClientId(), cmd.getOrderId()));
            break;
        }
    }
    return events;
}