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

    const u_int64_t cmdClientId = cmd.getClientId();
    const uint64_t cmdOrderId = cmd.getOrderId();
    const auto& cmdSymbol = cmd.getSymbol();
    if (cmdSymbol != _symbol) {
        events.emplace_back(std::make_unique<RejectEvent>(cmdClientId, cmdOrderId, RejectReason::UnknownSymbol));
        return events;
    }
    
    switch (cmd.getType()) {
        case CommandType::Limit: {
            const LimitCommand& limitCmd = static_cast<const LimitCommand&>(cmd);
            const Side cmdSide = limitCmd.getSide();
            const int64_t cmdPrice = limitCmd.getPrice();
            int64_t cmdQtyRemaining = limitCmd.getQty();

            while (cmdQtyRemaining > 0) {
                Order* topOrder = cmdSide == Side::Buy ? _orderBook.peekAsk() : _orderBook.peekBid();
                if (topOrder == nullptr) {
                    break; // No more liquidity, exit loop
                }
                if ((cmdSide == Side::Buy && topOrder->getPrice() > cmdPrice) || (cmdSide == Side::Sell && topOrder->getPrice() < cmdPrice)) {
                    break; // end cross status
                }

                uint64_t topOrderClientId = topOrder->getClientId();
                uint64_t topOrderId = topOrder->getOrderId();
                int64_t execPrice = topOrder->getPrice();
                int64_t adverseQty = topOrder->getQtyRemaining();
                int64_t execQty = std::min(cmdQtyRemaining, adverseQty);

                if (execQty == adverseQty) {
                    // full fill top order
                    events.emplace_back(std::make_unique<FillEvent>(topOrderClientId, topOrderId, _matchId, execPrice, execQty));
                    events.emplace_back(std::make_unique<FillEvent>(cmdClientId, cmdOrderId, _matchId, execPrice, execQty));
                    cmdSide == Side::Buy ? _orderBook.consumeAsk() : _orderBook.consumeBid(); // Remove the top order
                    cmdQtyRemaining -= execQty;
                    _matchId++;
                    continue;
                } else {
                    // partial fill
                    topOrder->qtyDecrease(execQty);
                    cmdQtyRemaining -= execQty;
                    events.emplace_back(std::make_unique<FillEvent>(topOrderClientId, topOrderId, _matchId, execPrice, execQty));
                    events.emplace_back(std::make_unique<FillEvent>(cmdClientId, cmdOrderId, _matchId, execPrice, execQty));
                    _matchId++;
                    break;
                }
            }

            if (cmdQtyRemaining > 0) {
                Order limitOrder(cmdClientId, cmdOrderId, cmdSymbol, cmdSide, cmdPrice, cmdQtyRemaining); // at which price we set the remaining qty in the book ?
                auto rejectReason = _orderBook.addLimit(limitOrder);
                if (rejectReason) {
                    events.emplace_back(std::make_unique<RejectEvent>(cmdClientId, cmdOrderId, *rejectReason));
                    break;
                }
            }
            events.emplace_back(std::make_unique<AckEvent>(cmdClientId, cmdOrderId));
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
            const Side cmdSide = marketCmd.getSide();
            int64_t cmdQtyRemaining = marketCmd.getQty();

            events.emplace_back(std::make_unique<AckEvent>(cmdClientId, cmdOrderId));

            while (cmdQtyRemaining > 0) {
                Order* topOrder = cmdSide == Side::Buy ? _orderBook.peekAsk() : _orderBook.peekBid();
                if (topOrder == nullptr) {
                    break; // No more liquidity, exit loop
                }
                uint64_t topOrderClientId = topOrder->getClientId();
                uint64_t topOrderId = topOrder->getOrderId();
                int64_t execPrice = topOrder->getPrice();
                int64_t adverseQty = topOrder->getQtyRemaining();
                int64_t execQty = std::min(cmdQtyRemaining, adverseQty);

                if (execQty == adverseQty) {
                    // full fill top order
                    events.emplace_back(std::make_unique<FillEvent>(topOrderClientId, topOrderId, _matchId, execPrice, execQty));
                    events.emplace_back(std::make_unique<FillEvent>(cmdClientId, cmdOrderId, _matchId, execPrice, execQty));
                    cmdSide == Side::Buy ? _orderBook.consumeAsk() : _orderBook.consumeBid(); // Remove the top order
                    cmdQtyRemaining -= execQty;
                    _matchId++;
                    continue;
                } else {
                    // partial fill
                    topOrder->qtyDecrease(execQty);
                    cmdQtyRemaining -= execQty;
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
