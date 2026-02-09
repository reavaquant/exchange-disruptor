#include "matching_engine/matching_engine.h"
#include "matching_engine/command/command.h"
#include "matching_engine/command/limit_command.h"
#include "matching_engine/command/cancel_command.h"
#include "matching_engine/command/market_command.h"
#include "matching_engine/event/event.h"
#include "matching_engine/event/reject_event.h"
#include "matching_engine/event/fill_event.h"
#include "matching_engine/event/ack_event.h"

#include <memory>
#include <vector>


MatchingEngine::MatchingEngine(std::string symbol) : _orderBook(symbol) {}

std::vector<std::unique_ptr<Event>> MatchingEngine::process(const Command& cmd) { // TODO: Implementer un type de retour plus perf plus tard
    std::vector<std::unique_ptr<Event>> events;
    if (cmd.getSymbol() != _symbol) {
        events.emplace_back(std::make_unique<RejectEvent>(cmd.getClientId(), cmd.getOrderId(), RejectReason::UnknownSymbol));
        return events;
    }
    switch (cmd.getType()) {
        case CommandType::Limit: {
            const LimitCommand& limitCmd = static_cast<const LimitCommand&>(cmd);
            Order order(limitCmd.getClientId(), limitCmd.getOrderId(), limitCmd.getSymbol(), limitCmd.getSide(), limitCmd.getPrice(), limitCmd.getQty());
            //look if crosses and generate FillEvents here before adding to book
            auto rejectReason = _orderBook.addLimit(order);
            if (rejectReason) {
                events.emplace_back(std::make_unique<RejectEvent>(cmd.getClientId(), cmd.getOrderId(), *rejectReason));
            } else {
                // If the order was accepted, we would normally check for matches and generate FillEvents here
            }
            break;
        }
        case CommandType::Cancel: {
            const CancelCommand& cancelCmd = static_cast<const CancelCommand&>(cmd);
            auto rejectReason = _orderBook.cancelOrder(cancelCmd.getOrderId(), cancelCmd.getClientId());
            if (rejectReason) {
                events.emplace_back(std::make_unique<RejectEvent>(cmd.getClientId(), cmd.getOrderId(), *rejectReason));
            } else {
                events.emplace_back(std::make_unique<AckEvent>(cmd.getClientId(), cmd.getOrderId()));
            }
            break;
        }
        case CommandType::Market: {
            const MarketCommand& marketCmd = static_cast<const MarketCommand&>(cmd);
            const auto& side = marketCmd.getSide();
            const auto& cmdClientId = marketCmd.getClientId();
            const auto& cmdOrderId = marketCmd.getOrderId();
            int64_t qtyRemaining = marketCmd.getQty();

            events.emplace_back(std::make_unique<AckEvent>(cmdClientId, cmdOrderId));

            while (qtyRemaining > 0) {
                Order* topOrder = marketCmd.getSide() == Side::Buy ? _orderBook.peekAsk() : _orderBook.peekBid();
                if (topOrder == nullptr) {
                    break; // No more liquidity, exit loop
                }
                uint64_t topOrderClientId = topOrder->getClientId();
                uint64_t topOrderId = topOrder->getOrderId();
                int64_t execPrice = topOrder->getPrice();
                int64_t adverseQty = topOrder->getQtyRemaining();
                int64_t execQty = std::min(qtyRemaining, adverseQty);

                if (execQty == adverseQty) {
                    // full fill top order
                    events.emplace_back(std::make_unique<FillEvent>(topOrderClientId, topOrderId, _matchId, execPrice, execQty));
                    events.emplace_back(std::make_unique<FillEvent>(cmdClientId, cmdOrderId, _matchId, execPrice, execQty));
                    side == Side::Buy ? _orderBook.consumeAsk() : _orderBook.consumeBid(); // Remove the top order
                    qtyRemaining -= execQty;
                    _matchId++;
                    continue;
                } else {
                    // partial fill
                    topOrder->qtyDecrease(execQty);
                    qtyRemaining -= execQty;
                    events.emplace_back(std::make_unique<FillEvent>(topOrderClientId, topOrderId, _matchId, execPrice, execQty));
                    events.emplace_back(std::make_unique<FillEvent>(cmdClientId, cmdOrderId, _matchId, execPrice, execQty));
                    _matchId++;
                    break;
                }
            }
            break;
        }
    }
    return events;
}
